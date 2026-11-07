// Raycast Silent Aim (ported from the Maku side project,
// features/raycast_silentaim.cpp).
//
// =====================================================================
// Architecture
// =====================================================================
// The engine hook works by:
//   1. Locating the "Raycast" class descriptor through the Roblox
//      reflection name registry (falling back to a string/descriptor
//      scan and finally the static offset) to get the function pointer
//      slot.
//   2. Building a machine-code stub at runtime (make_hook_thunk) that:
//        a. reads a RaycastState struct from a fixed remote address,
//        b. checks the active flag + reserved (wallbang) flag,
//        c. replaces the in-flight ray's origin / direction with the
//           camera -> target vector when active,
//        d. forwards the call to the original function.
//   3. Patching the function pointer slot so the engine calls the stub.
//      SetProcessValidCallTargets is used to keep CFG happy.
//
// This is an EXTERNAL overlay: everything goes through the process
// handle from memory->get_process_handle(), with a file-local mutex
// protecting the raw Read/WriteProcessMemory size checks.
//
// Adaptations for this project:
//   - Target selection is NOT duplicated here: the module consumes the
//     target already resolved by features/silentaim (silentaim::state),
//     so team / knock / gun / fov / prediction filtering is identical
//     for the viewport and the raycast method.
//   - Wallbang is ported in full: the engine-hook method writes the remote
//     "reserved" flag, and the position-spoof method runs a dedicated
//     high-priority spoof thread (see position_spoof_worker).
//   - The snap line / target dot are drawn by render.cpp from
//     silentaim::state, so the Maku draw_snapline port was dropped.

#include "raycast_silentaim.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <imgui.h>

#include <cache/cache.h>
#include <game/game.h>
#include <settings.h>
#include <features/keybind/keybind.h>
#include <features/silentaim/silentaim.h>
#include <memory/memory.h>
#include <sdk/sdk.h>

namespace raycast_silentaim
{
	// --- Diagnostics --------------------------------------------------
	static void raycast_log(const char* message)
	{
		char path[MAX_PATH]{};
		if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
			return;

		char* slash = strrchr(path, '\\');
		if (!slash)
			return;

		strcpy_s(slash + 1, MAX_PATH - static_cast<std::size_t>((slash + 1) - path), "raycast.log");

		FILE* file = nullptr;
		if (fopen_s(&file, path, "a") != 0 || file == nullptr)
			return;

		fprintf(file, "%s\n", message);
		fclose(file);
	}

	// --- Public globals (declared in the header) -----------------------
	bool           target_acquired   = false;
	math::vector2  target_screen_pos{};
	std::uint64_t  target_address    = 0;

	// --- File-local IO lock (main memory_t has no handle mutex) --------
	static std::mutex s_io_mtx;

	void set_active(bool on, math::vector3 target_pos = {}, std::uintptr_t target_part = 0, math::vector3 cam_pos = {});
	static void restore_position_spoof();
	static void position_spoof_worker();

	const std::uintptr_t desc_rva_z       = Offsets::WorldRoot::RaycastBoundDesc;
	const std::uintptr_t bound_fn_offset  = Offsets::WorldRoot::RaycastBoundFn;

#pragma pack(push, 4)
	struct RaycastState
	{
		std::uint32_t active   = 0;
		std::uint32_t reserved = 0;
		float         target_x = 0.f;
		float         target_y = 0.f;
		float         target_z = 0.f;
		float         scale    = 1.15f;
		std::uint64_t calls    = 0;
		float         cam_x    = 0.f;
		float         cam_y    = 0.f;
		float         cam_z    = 0.f;
		float         limit_sq = 900.f;
	};
#pragma pack(pop)
	static_assert(offsetof(RaycastState, active)   == 0x00, "active");
	static_assert(offsetof(RaycastState, target_x) == 0x08, "target");
	static_assert(offsetof(RaycastState, calls)    == 0x18, "calls");

	struct Hook
	{
		std::uintptr_t slot             = 0;
		std::uintptr_t thunk            = 0;
		std::uintptr_t state            = 0;
		std::uintptr_t originalFunction = 0;
		std::uintptr_t module_base      = 0;
		bool           thunk_owned      = false;
		bool           installed        = false;
		bool           active           = false;
	};

	Hook g_hook{};
	bool g_wallbang = false;
	auto g_lastFail = std::chrono::steady_clock::time_point{};

	// Wallbang position-spoof method: shared state between the aim thread and
	// the high-priority spoof thread.
	static std::atomic<std::uintptr_t> s_spoof_hrp_prim{ 0 };
	static std::atomic<bool>           s_spoof_active{ false };
	static math::vector3               s_spoof_target_pos{};
	static std::mutex                  s_spoof_mtx;

	bool addr_ok(std::uintptr_t a)
	{
		return a >= 0x10000ull && a < 0x00007FFFFFFFFFFFull;
	}

	bool valid_float(float v) { return std::isfinite(v) && v > -1e8f && v < 1e8f; }

	bool w_mem(std::uintptr_t a, const void* d, std::size_t s)
	{
		if (!addr_ok(a) || !d || !s || !memory->get_process_handle())
		{
			return false;
		}
		std::lock_guard<std::mutex> lk(s_io_mtx);
		SIZE_T w = 0;
		if (!WriteProcessMemory(memory->get_process_handle(), (void*)a, d, s, &w) || w != s)
		{
			return false;
		}
		return true;
	}

	bool r_mem(std::uintptr_t a, void* d, std::size_t s)
	{
		if (!addr_ok(a) || !d || !s || !memory->get_process_handle())
		{
			return false;
		}
		std::lock_guard<std::mutex> lk(s_io_mtx);
		SIZE_T r = 0;
		if (!ReadProcessMemory(memory->get_process_handle(), (void*)a, d, s, &r) || r != s)
		{
			return false;
		}
		return true;
	}

	bool read_val(std::uintptr_t a, void* d, std::size_t s)
	{
		SIZE_T r = 0;
		if (!ReadProcessMemory(memory->get_process_handle(), (void*)a, d, s, &r) || r != s)
		{
			return false;
		}
		return true;
	}

	std::size_t page_sz()
	{
		static std::size_t p = [] {
			SYSTEM_INFO i{};
			GetSystemInfo(&i);
			return i.dwPageSize ? (std::size_t)i.dwPageSize : 0x1000u;
		}();
		return p;
	}

	DWORD query_protect(std::uintptr_t a)
	{
		MEMORY_BASIC_INFORMATION mbi{};
		if (!VirtualQueryEx(memory->get_process_handle(), (void*)a, &mbi, sizeof(mbi))) return 0;
		return mbi.Protect;
	}

	bool is_executable_protect(DWORD p)
	{
		DWORD x = p & 0xFF;
		return x == PAGE_EXECUTE || x == PAGE_EXECUTE_READ ||
			x == PAGE_EXECUTE_READWRITE || x == PAGE_EXECUTE_WRITECOPY;
	}

	// --- Stage 1: Reflection-system name registry lookup -----------------
	// (ported from the side project's features/instance.new/reflection/Reflect.cpp)
	std::uint64_t reflect_name(std::uintptr_t base, const char* text)
	{
		if (!text || !text[0]) return 0;

		const std::uint64_t reg = memory->read<std::uint64_t>(base + Offsets::Reflection::NameRegistry);
		{
			char line[256];
			snprintf(line, sizeof(line), "REFLECT name='%s' base=0x%llX NameRegistry=0x%llX",
				text, static_cast<unsigned long long>(base),
				static_cast<unsigned long long>(reg));
			raycast_log(line);
		}
		if (!reg)
		{
			raycast_log("REFLECT registry pointer is 0 -> lookup aborted");
			return 0;
		}

		const std::uint64_t t_start = memory->read<std::uint64_t>(reg + Offsets::Reflection::NameTable + Offsets::Reflection::TableStart);
		const std::uint64_t t_end   = memory->read<std::uint64_t>(reg + Offsets::Reflection::NameTable + Offsets::Reflection::TableEnd);
		const std::uint64_t t_empty = memory->read<std::uint64_t>(reg + Offsets::Reflection::NameTable + Offsets::Reflection::TableEmpty);
		{
			char line[256];
			snprintf(line, sizeof(line), "REFLECT reg=0x%llX t_start=0x%llX t_end=0x%llX t_empty=0x%llX entries=%llu",
				static_cast<unsigned long long>(reg),
				static_cast<unsigned long long>(t_start),
				static_cast<unsigned long long>(t_end),
				static_cast<unsigned long long>(t_empty),
				(t_end > t_start && (t_end - t_start) <= 0x400000)
					? static_cast<unsigned long long>((t_end - t_start) / Offsets::Reflection::TableStride)
					: 0ull);
			raycast_log(line);
		}
		if (!t_start || t_end <= t_start || (t_end - t_start) > 0x400000)
		{
			raycast_log("REFLECT name table bounds invalid -> lookup aborted");
			return 0;
		}

		const std::size_t len = std::strlen(text);
		char cmp[128];
		if (len + 1 > sizeof(cmp)) return 0;

		for (std::uint64_t e = t_start; e + Offsets::Reflection::TableStride <= t_end; e += Offsets::Reflection::TableStride)
		{
			const std::uint64_t key = memory->read<std::uint64_t>(e);
			if (!key || key == t_empty) continue;
			if (!r_mem(key, cmp, len + 1)) continue;
			if (cmp[len] != '\0' || std::memcmp(cmp, text, len) != 0) continue;

			const std::uint64_t name_val = memory->read<std::uint64_t>(e + Offsets::Reflection::EntryValue);
			{
				char line[256];
				snprintf(line, sizeof(line), "REFLECT MATCH entry=0x%llX name_val=0x%llX", static_cast<unsigned long long>(e), static_cast<unsigned long long>(name_val));
				raycast_log(line);
			}
			return name_val;
		}

		raycast_log("REFLECT walked whole table, no entry matched this name");
		return 0;
	}

	// --- Stages 1-3 of slot resolution ------------------------------------
	std::uintptr_t get_raycast_slot(std::uintptr_t base)
	{
		if (!base || !memory->get_process_handle())
		{
			return 0;
		}

		static std::uintptr_t s_cached_slot = 0;
		static std::uintptr_t s_cached_base = 0;
		if (s_cached_slot && s_cached_base == base)
		{
			std::uintptr_t cur_fn = memory->read<std::uint64_t>(s_cached_slot);
			if (addr_ok(cur_fn))
			{
				return s_cached_slot;
			}
			s_cached_slot = 0;
		}

		// ---- PE header validation (catches a stale / wrong base) ----
		std::uint32_t img_sz = 0;
		IMAGE_DOS_HEADER dos{};
		if (r_mem(base, &dos, sizeof(dos)) && dos.e_magic == IMAGE_DOS_SIGNATURE)
		{
			const std::int32_t lfanew = (std::int32_t)dos.e_lfanew;
			IMAGE_NT_HEADERS64 nt{};
			if (r_mem(base + lfanew, &nt, sizeof(nt)) && nt.Signature == IMAGE_NT_SIGNATURE)
			{
				img_sz = nt.OptionalHeader.SizeOfImage;
			}
		}
		if (img_sz < 0x1000 || img_sz > 0x20000000u)
		{
			img_sz = 0x8000000u;
		}

		{
			char line[256];
			snprintf(line, sizeof(line), "SLOT base=0x%llX img_sz=0x%X desc_rva=0x%llX fn_off=0x%llX static_slot=0x%llX",
				static_cast<unsigned long long>(base),
				img_sz,
				static_cast<unsigned long long>(desc_rva_z),
				static_cast<unsigned long long>(bound_fn_offset),
				static_cast<unsigned long long>(base + desc_rva_z + bound_fn_offset));
			raycast_log(line);
		}

		// ---- Stage 0: static descriptor slot ----------------------------
		// The descriptor RVA and bound-function offset are known for this
		// build (validated against the live process), so resolve directly.
		// This is tried first so startup never pays for the heavier scans.
		if (desc_rva_z && bound_fn_offset)
		{
			std::uintptr_t static_slot = base + desc_rva_z + bound_fn_offset;
			std::uintptr_t static_fn = 0;
			if (r_mem(static_slot, &static_fn, sizeof(static_fn)) &&
				addr_ok(static_fn) && is_executable_protect(query_protect(static_fn)))
			{
				char line[192];
				snprintf(line, sizeof(line), "STAGE0 static slot 0x%llX -> fn 0x%llX (accepted)",
					static_cast<unsigned long long>(static_slot),
					static_cast<unsigned long long>(static_fn));
				raycast_log(line);
				s_cached_slot = static_slot;
				s_cached_base = base;
				return static_slot;
			}
		}

		// ---- Stage 1: Reflection System ---------------------------------
		// (the resolution path the side project actually relies on; dropped
		//  in the initial port because it was wrongly believed Reflection was
		//  unavailable - Offsets::Reflection exists and matches the side)
		if (desc_rva_z && bound_fn_offset)
		{
			const std::uint64_t name_val = reflect_name(base, "Raycast");
			if (name_val)
			{
				std::uintptr_t addr = base;
				MEMORY_BASIC_INFORMATION mbi{};
				while (addr < base + img_sz && VirtualQueryEx(memory->get_process_handle(), (void*)addr, &mbi, sizeof(mbi)))
				{
					std::uintptr_t rb = (std::uintptr_t)mbi.BaseAddress;
					std::size_t rs = (std::size_t)mbi.RegionSize;
					std::uintptr_t next = rb + rs;
					if (next <= addr) break;
					if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && !is_executable_protect(mbi.Protect))
					{
						std::uintptr_t s = (rb > base) ? rb : base;
						std::uintptr_t e = (next < base + img_sz) ? next : base + img_sz;
						if (e > s)
						{
							std::vector<std::uint8_t> buf(e - s);
							if (r_mem(s, buf.data(), buf.size()))
							{
								for (std::size_t i = 8; i + 8 <= buf.size(); i += 8)
								{
									std::uint64_t v = 0;
									std::memcpy(&v, buf.data() + i, 8);
									if (v == name_val)
									{
										std::uintptr_t desc = s + i - 8;
										std::uintptr_t vt = memory->read<std::uint64_t>(desc);
										if (vt >= base && vt < base + img_sz)
										{
											std::uintptr_t slot_addr = desc + bound_fn_offset;
											std::uintptr_t fn = memory->read<std::uint64_t>(slot_addr);
											if (addr_ok(fn) && is_executable_protect(query_protect(fn)))
											{
												s_cached_slot = slot_addr;
												s_cached_base = base;
												return slot_addr;
											}
										}
									}
								}
							}
						}
					}
					addr = next;
				}
			}
		}


		// --- Stage 2: Direct String & 2-Hop Reflection Pointer Scan ---
		std::vector<std::uintptr_t> string_addrs;
		{
			const char target_str[] = "Raycast";
			const std::size_t str_len = sizeof(target_str);
			std::uintptr_t addr = base;
			MEMORY_BASIC_INFORMATION mbi{};
			while (addr < base + img_sz && VirtualQueryEx(memory->get_process_handle(), (void*)addr, &mbi, sizeof(mbi)))
			{
				std::uintptr_t rb = (std::uintptr_t)mbi.BaseAddress;
				std::size_t rs = (std::size_t)mbi.RegionSize;
				std::uintptr_t next = rb + rs;
				if (next <= addr) break;
				if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && !is_executable_protect(mbi.Protect))
				{
					std::uintptr_t s = (rb > base) ? rb : base;
					std::uintptr_t e = (next < base + img_sz) ? next : base + img_sz;
					if (e > s)
					{
						std::vector<std::uint8_t> buf(e - s);
						if (r_mem(s, buf.data(), buf.size()))
						{
							for (std::size_t i = 0; i + str_len <= buf.size(); ++i)
							{
								if (std::memcmp(buf.data() + i, target_str, str_len) == 0)
								{
									string_addrs.push_back(s + i);
								}
							}
						}
					}
				}
				addr = next;
			}
		}

		{
			char line[192];
			snprintf(line, sizeof(line), "STAGE2 string 'Raycast' occurrences found: %zu", string_addrs.size());
			raycast_log(line);
		}

		// Step 2a: Find all name object candidates that point to the string
		std::unordered_set<std::uintptr_t> name_candidates;
		for (std::uintptr_t str_addr : string_addrs)
		{
			name_candidates.insert(str_addr); // Also try direct string pointer

			std::uintptr_t addr = base;
			MEMORY_BASIC_INFORMATION mbi{};
			while (addr < base + img_sz && VirtualQueryEx(memory->get_process_handle(), (void*)addr, &mbi, sizeof(mbi)))
			{
				std::uintptr_t rb = (std::uintptr_t)mbi.BaseAddress;
				std::size_t rs = (std::size_t)mbi.RegionSize;
				std::uintptr_t next = rb + rs;
				if (next <= addr) break;
				if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && !is_executable_protect(mbi.Protect))
				{
					std::uintptr_t s = (rb > base) ? rb : base;
					std::uintptr_t e = (next < base + img_sz) ? next : base + img_sz;
					if (e > s)
					{
						std::vector<std::uint8_t> buf(e - s);
						if (r_mem(s, buf.data(), buf.size()))
						{
							for (std::size_t i = 0; i + 8 <= buf.size(); i += 8)
							{
								std::uint64_t ptr_val = 0;
								std::memcpy(&ptr_val, buf.data() + i, 8);
								if (ptr_val == str_addr)
								{
									std::uintptr_t ptr_loc = s + i;
									name_candidates.insert(ptr_loc);
									if (ptr_loc >= base + 8) name_candidates.insert(ptr_loc - 8);
									if (ptr_loc >= base + 16) name_candidates.insert(ptr_loc - 16);
									if (i + 16 <= buf.size())
									{
										std::uint64_t next_val = 0;
										std::memcpy(&next_val, buf.data() + i + 8, 8);
										if (addr_ok(next_val)) name_candidates.insert(next_val);
									}
								}
							}
						}
					}
				}
				addr = next;
			}
		}

		// Step 2b: Find method descriptors referencing any name candidate
		{
			char line[192];
			snprintf(line, sizeof(line), "STAGE2b name_candidates: %zu", name_candidates.size());
			raycast_log(line);
		}
		for (std::uintptr_t name_cand : name_candidates)
		{
			std::uintptr_t addr = base;
			MEMORY_BASIC_INFORMATION mbi{};
			while (addr < base + img_sz && VirtualQueryEx(memory->get_process_handle(), (void*)addr, &mbi, sizeof(mbi)))
			{
				std::uintptr_t rb = (std::uintptr_t)mbi.BaseAddress;
				std::size_t rs = (std::size_t)mbi.RegionSize;
				std::uintptr_t next = rb + rs;
				if (next <= addr) break;
				if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && !is_executable_protect(mbi.Protect))
				{
					std::uintptr_t s = (rb > base) ? rb : base;
					std::uintptr_t e = (next < base + img_sz) ? next : base + img_sz;
					if (e > s)
					{
						std::vector<std::uint8_t> buf(e - s);
						if (r_mem(s, buf.data(), buf.size()))
						{
							for (std::size_t i = 0; i + 8 <= buf.size(); i += 8)
							{
								std::uint64_t ptr_val = 0;
								std::memcpy(&ptr_val, buf.data() + i, 8);
								if (ptr_val == name_cand)
								{
									std::uintptr_t ptr_loc = s + i;
									for (std::uintptr_t offset_back : { 0x08ull, 0x10ull, 0x18ull, 0x00ull })
									{
										if (ptr_loc < base + offset_back) continue;
										std::uintptr_t desc_cand = ptr_loc - offset_back;
										std::uintptr_t vt = memory->read<std::uint64_t>(desc_cand);
										if (vt >= base && vt < base + img_sz)
										{
											for (std::uintptr_t fn_off : { bound_fn_offset, 0x78ull, 0x88ull, 0x70ull, 0x90ull, 0x80ull, 0x68ull, 0x98ull })
											{
												std::uintptr_t slot_cand = desc_cand + fn_off;
												std::uintptr_t fn_cand = memory->read<std::uint64_t>(slot_cand);
												if (addr_ok(fn_cand) && fn_cand >= base && fn_cand < base + img_sz && is_executable_protect(query_protect(fn_cand)))
												{
													s_cached_slot = slot_cand;
													s_cached_base = base;
													{
														char sl[192];
														snprintf(sl, sizeof(sl), "STAGE2b ACCEPT slot=0x%llX fn=0x%llX desc=0x%llX", static_cast<unsigned long long>(slot_cand), static_cast<unsigned long long>(fn_cand), static_cast<unsigned long long>(desc_cand));
														raycast_log(sl);
													}
													return slot_cand;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				addr = next;
			}
		}

		// --- Stage 3: Static Slot Fallback ---
		std::uintptr_t static_slot = base + desc_rva_z + bound_fn_offset;
		std::uintptr_t static_fn = 0;
		if (r_mem(static_slot, &static_fn, sizeof(static_fn)) && addr_ok(static_fn))
		{
			{
				char line[192];
				snprintf(line, sizeof(line), "STAGE3 static slot 0x%llX -> fn 0x%llX (accepted)", static_cast<unsigned long long>(static_slot), static_cast<unsigned long long>(static_fn));
				raycast_log(line);
			}
			s_cached_slot = static_slot;
			s_cached_base = base;
			return static_slot;
		}

		{
			char line[256];
			snprintf(line, sizeof(line), "STAGE3 static slot 0x%llX read_ok=%d fn=0x%llX (rejected)",
				static_cast<unsigned long long>(static_slot), 0,
				static_cast<unsigned long long>(static_fn));
			raycast_log(line);
		}


		raycast_log("get_raycast_slot: every stage failed");
		return 0;
	}

	std::uintptr_t get_module_base(const wchar_t* name)
	{
		if (!memory->get_process_handle()) return 0;
		HMODULE mods[1024];
		DWORD needed = 0;
		if (!EnumProcessModules(memory->get_process_handle(), mods, sizeof(mods), &needed)) return 0;
		DWORD cnt = needed / sizeof(HMODULE);
		for (DWORD i = 0; i < cnt; ++i)
		{
			wchar_t buf[MAX_PATH]{};
			if (GetModuleBaseNameW(memory->get_process_handle(), mods[i], buf, MAX_PATH) &&
				_wcsicmp(buf, name) == 0)
				return (std::uintptr_t)mods[i];
		}
		return 0;
	}

	std::string module_name_for_addr(std::uintptr_t addr)
	{
		if (!memory->get_process_handle()) return "?";
		HMODULE mods[1024];
		DWORD needed = 0;
		if (!EnumProcessModules(memory->get_process_handle(), mods, sizeof(mods), &needed)) return "?";
		DWORD cnt = needed / sizeof(HMODULE);
		for (DWORD i = 0; i < cnt; ++i)
		{
			std::uintptr_t mb = (std::uintptr_t)mods[i];
			MODULEINFO mi{};
			if (mb && addr >= mb && GetModuleInformation(memory->get_process_handle(), mods[i], &mi, sizeof(mi)) &&
				addr < mb + mi.SizeOfImage)
			{
				wchar_t buf[MAX_PATH]{};
				if (!GetModuleBaseNameW(memory->get_process_handle(), mods[i], buf, MAX_PATH)) return "?";
				char abuf[256];
				sprintf_s(abuf, "%S base=0x%llX size=0x%llX", buf, (unsigned long long)mb, (unsigned long long)mi.SizeOfImage);
				return std::string(abuf);
			}
		}
		return "no-module";
	}

	bool protect_remote(std::uintptr_t address, std::size_t size, DWORD protection, DWORD* old_protect = nullptr)
	{
		if (!addr_ok(address) || !size || !memory->get_process_handle()) return false;

		std::uintptr_t page_mask = ~((std::uintptr_t)page_sz() - 1);
		std::uintptr_t base = address & page_mask;
		std::uintptr_t end  = (address + size + page_sz() - 1) & page_mask;
		std::size_t span = (std::size_t)(end - base);

		using NtProtectFn = LONG(WINAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
		static NtProtectFn nt_protect = nullptr;
		if (!nt_protect)
		{
			HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
			if (ntdll) nt_protect = (NtProtectFn)GetProcAddress(ntdll, "NtProtectVirtualMemory");
		}

		auto try_one = [&](DWORD req) -> bool {
			DWORD old = 0;
			if (VirtualProtectEx(memory->get_process_handle(), (void*)base, span, req, &old))
			{
				if (old_protect) *old_protect = old;
				return true;
			}
			if (!nt_protect) return false;
			PVOID  nb = (void*)base;
			SIZE_T ns = span;
			ULONG  no = 0;
			LONG st = nt_protect(memory->get_process_handle(), &nb, &ns, req, &no);
			if (st >= 0) { if (old_protect) *old_protect = (DWORD)no; return true; }
			return false;
		};

		if (try_one(protection)) return true;
		if (protection == PAGE_EXECUTE_READWRITE && try_one(PAGE_EXECUTE_WRITECOPY)) return true;
		if (protection == PAGE_READWRITE && try_one(PAGE_WRITECOPY)) return true;
		return false;
	}

	bool write_protected(std::uintptr_t address, const void* data, std::size_t size)
	{
		if (!addr_ok(address) || !data || !size) return false;
		DWORD old = 0;
		bool changed = protect_remote(address, size, PAGE_EXECUTE_READWRITE, &old);
		bool wrote   = w_mem(address, data, size);
		if (changed) protect_remote(address, size, old, nullptr);
		return wrote;
	}

	bool mark_cfg(std::uintptr_t t)
	{
		auto resolve = []() -> FARPROC {
			const char* mods[] = {
				"kernelbase.dll", "kernel32.dll",
				"api-ms-win-core-memory-l1-1-3.dll"
			};
			for (auto* m : mods)
			{
				HMODULE h = GetModuleHandleA(m);
				if (!h) h = LoadLibraryA(m);
				if (!h) continue;
				FARPROC p = GetProcAddress(h, "SetProcessValidCallTargets");
				if (p) return p;
			}
			return nullptr;
		};

		FARPROC proc = resolve();
		if (!proc)
		{
			return false;
		}

		struct Info { ULONG_PTR Offset; ULONG Flags; } info{};
		info.Offset = t & (page_sz() - 1);
		info.Flags  = CFG_CALL_TARGET_VALID;

		using Fn = BOOL(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, void*);
		SetLastError(0);
		BOOL ok = ((Fn)proc)(
			memory->get_process_handle(),
			(void*)(t & ~((std::uintptr_t)page_sz() - 1)),
			page_sz(), 1, &info);

		if (!ok)
		{
			return false;
		}
		return true;
	}

	void append_u64(std::vector<std::uint8_t>& c, std::uint64_t v)
	{
		const auto* b = (const std::uint8_t*)&v;
		c.insert(c.end(), b, b + 8);
	}

	void patch_rel32(std::vector<std::uint8_t>& c, std::size_t o, std::size_t t)
	{
		std::int32_t v = (std::int32_t)((std::ptrdiff_t)t - (std::ptrdiff_t)(o + 4));
		std::memcpy(c.data() + o, &v, 4);
	}

	// Machine-code stub that redirects the engine raycast. Copied
	// byte-for-byte from the side project (make_hook_thunk).
	std::vector<std::uint8_t> make_hook_thunk(std::uintptr_t state, std::uintptr_t orig)
	{
		std::vector<std::uint8_t> c;
		c.reserve(384);
		std::vector<std::size_t> inactive;

		auto je_inactive = [&] {
			c.insert(c.end(), { 0x0F, 0x84 });
			inactive.push_back(c.size());
			c.insert(c.end(), { 0, 0, 0, 0 });
		};
		auto jbe_inactive = [&] {
			c.insert(c.end(), { 0x0F, 0x86 });
			inactive.push_back(c.size());
			c.insert(c.end(), { 0, 0, 0, 0 });
		};
		auto ja_inactive = [&] {
			c.insert(c.end(), { 0x0F, 0x87 });
			inactive.push_back(c.size());
			c.insert(c.end(), { 0, 0, 0, 0 });
		};

		c.insert(c.end(), { 0x48, 0x83, 0xEC, 0x68 });
		c.insert(c.end(), { 0x49, 0xBA });
		append_u64(c, state);
		c.insert(c.end(), { 0x41, 0x83, 0x3A, 0x00 });
		je_inactive();
		c.insert(c.end(), { 0x4D, 0x85, 0xC0 }); je_inactive();
		c.insert(c.end(), { 0x4D, 0x85, 0xC9 }); je_inactive();

		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x42, 0x08 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x00 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x44, 0x24, 0x40 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x4A, 0x0C });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x48, 0x04 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x4C, 0x24, 0x44 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x52, 0x10 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x50, 0x08 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x54, 0x24, 0x48 });

		c.insert(c.end(), { 0x0F, 0x28, 0xD8 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xDB });
		c.insert(c.end(), { 0x0F, 0x28, 0xE1 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xDC });
		c.insert(c.end(), { 0x0F, 0x28, 0xE2 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xDC });
		c.insert(c.end(), { 0xF3, 0x0F, 0x51, 0xDB });
		c.insert(c.end(), { 0x0F, 0x57, 0xED });
		c.insert(c.end(), { 0x0F, 0x2E, 0xDD });
		jbe_inactive();

		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x21 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x69, 0x04 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xED });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE5 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x69, 0x08 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xED });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE5 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x51, 0xE4 });
		c.insert(c.end(), { 0x0F, 0x57, 0xED });
		c.insert(c.end(), { 0x0F, 0x2E, 0xE5 });
		jbe_inactive();

		c.insert(c.end(), { 0x41, 0x8B, 0x42, 0x04 });
		c.insert(c.end(), { 0xA8, 0x01 });
		c.insert(c.end(), { 0x0F, 0x85 });
		const std::size_t wallbang_jmp = c.size();
		c.insert(c.end(), { 0, 0, 0, 0 });

		c.insert(c.end(), { 0x0F, 0x28, 0xEC });
		c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xEB });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xC5 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xCD });
		c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xD5 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x11, 0x01 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x11, 0x49, 0x04 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x11, 0x51, 0x08 });
		c.insert(c.end(), { 0x49, 0xFF, 0x42, 0x18 });
		c.push_back(0xE9);
		const std::size_t to_call = c.size();
		c.insert(c.end(), { 0, 0, 0, 0 });

		const std::size_t wallbang_off = c.size();
		patch_rel32(c, wallbang_jmp, wallbang_off);

		c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xC3 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xCB });
		c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xD3 });

		c.insert(c.end(), { 0x0F, 0x28, 0xE0 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x59, 0x62, 0x14 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x6A, 0x08 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x5C, 0xEC });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x6C, 0x24, 0x50 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE4 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x64, 0x24, 0x40 });
		c.insert(c.end(), { 0x0F, 0x28, 0xE1 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x59, 0x62, 0x14 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x6A, 0x0C });
		c.insert(c.end(), { 0xF3, 0x0F, 0x5C, 0xEC });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x6C, 0x24, 0x54 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE4 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x64, 0x24, 0x44 });
		c.insert(c.end(), { 0x0F, 0x28, 0xE2 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x59, 0x62, 0x14 });
		c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x6A, 0x10 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x5C, 0xEC });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x6C, 0x24, 0x58 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE4 });
		c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x64, 0x24, 0x48 });

		c.insert(c.end(), { 0x4C, 0x8D, 0x44, 0x24, 0x50 });
		c.insert(c.end(), { 0x4C, 0x8D, 0x4C, 0x24, 0x40 });
		c.insert(c.end(), { 0x49, 0xFF, 0x42, 0x18 });

		const std::size_t call_off = c.size();
		patch_rel32(c, to_call, call_off);
		const std::size_t inactive_off = c.size();
		for (auto o : inactive) patch_rel32(c, o, inactive_off);

		c.insert(c.end(), { 0x48, 0x8B, 0x84, 0x24, 0x90, 0x00, 0x00, 0x00 });
		c.insert(c.end(), { 0x48, 0x89, 0x44, 0x24, 0x20 });
		c.insert(c.end(), { 0x48, 0xB8 });
		append_u64(c, orig);
		c.insert(c.end(), { 0xFF, 0xD0 });
		c.insert(c.end(), { 0x48, 0x83, 0xC4, 0x68 });
		c.push_back(0xC3);
		return c;
	}

	bool region_is_padding(std::uintptr_t a, std::size_t n)
	{
		std::vector<std::uint8_t> buf(n);
		if (!read_val(a, buf.data(), n)) return false;
		for (auto b : buf) if (b != 0xCC && b != 0x00 && b != 0x90) return false;
		return true;
	}

	std::uintptr_t find_cave_in_module(std::uintptr_t module_base, std::size_t need,
		std::uintptr_t min_offset, std::uintptr_t ignore)
	{
		if (!addr_ok(module_base) || !need) return 0;

		IMAGE_DOS_HEADER dos{};
		if (!read_val(module_base, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE) return 0;

		IMAGE_NT_HEADERS64 nt{};
		std::uintptr_t nt_addr = module_base + (std::uintptr_t)dos.e_lfanew;
		if (!read_val(nt_addr, &nt, sizeof(nt)) || nt.Signature != IMAGE_NT_SIGNATURE) return 0;

		std::uintptr_t section_base = nt_addr + offsetof(IMAGE_NT_HEADERS64, OptionalHeader)
			+ nt.FileHeader.SizeOfOptionalHeader;

		for (WORD i = 0; i < nt.FileHeader.NumberOfSections; ++i)
		{
			IMAGE_SECTION_HEADER sec{};
			if (!read_val(section_base + (std::uintptr_t)i * sizeof(sec), &sec, sizeof(sec))) break;
			if (!(sec.Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;

			std::uintptr_t sec_off  = sec.VirtualAddress;
			std::size_t sec_sz = sec.Misc.VirtualSize ? sec.Misc.VirtualSize : sec.SizeOfRawData;
			if (!sec_off || sec_sz < need) continue;

			std::uintptr_t scan_off = sec_off;
			if (scan_off < min_offset) scan_off = min_offset;
			if (scan_off >= sec_off + sec_sz) continue;

			std::uintptr_t scan_start = module_base + scan_off;
			std::size_t scan_sz  = (std::size_t)(sec_off + sec_sz - scan_off);

			std::vector<std::uint8_t> buf(scan_sz);
			if (!read_val(scan_start, buf.data(), buf.size())) continue;

			std::size_t run_start = 0, run_len = 0;
			for (std::size_t j = 0; j < buf.size(); ++j)
			{
				std::uint8_t b = buf[j];
				if (b != 0x00 && b != 0xCC && b != 0x90) { run_len = 0; run_start = j + 1; continue; }
				++run_len;
				if (run_len < need) continue;

				std::uintptr_t cand    = scan_start + run_start;
				std::uintptr_t aligned = (cand + 0x0F) & ~(std::uintptr_t)0x0F;
				std::size_t loss  = (std::size_t)(aligned - cand);
				if (run_len < need + loss) continue;
				if (ignore && aligned == ignore) continue;
				if (g_hook.thunk && aligned == g_hook.thunk) continue;
				return aligned;
			}
		}
		return 0;
	}

	std::uintptr_t find_exec_cave(std::size_t need, std::uintptr_t = 0, std::uintptr_t ignore = 0)
	{
		static const wchar_t* pref[] = {
			L"winsta.dll", L"win32u.dll", L"uxtheme.dll", L"dwmapi.dll",
			L"msctf.dll",  L"TextInputFramework.dll", L"CoreMessaging.dll", L"user32.dll",
		};

		for (std::size_t mi = 0; mi < sizeof(pref) / sizeof(pref[0]); ++mi)
		{
			std::uintptr_t mod = get_module_base(pref[mi]);
			if (!mod)
			{
				continue;
			}
			std::uintptr_t min_off = (mi == 0) ? 0x2000u : 0x1000u;
			std::uintptr_t cave    = find_cave_in_module(mod, need, min_off, ignore);
			if (cave)
			{
				return cave;
			}
		}

		MEMORY_BASIC_INFORMATION mbi{};
		std::uintptr_t addr = 0, fallback_rwx = 0;
		while (VirtualQueryEx(memory->get_process_handle(), (void*)addr, &mbi, sizeof(mbi)))
		{
			std::uintptr_t base = (std::uintptr_t)mbi.BaseAddress;
			std::size_t size = (std::size_t)mbi.RegionSize;
			addr = base + size;
			if (addr < base) break;
			if (mbi.State != MEM_COMMIT) continue;
			if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) continue;
			if (!is_executable_protect(mbi.Protect)) continue;
			if (size < need) continue;
			if (g_hook.thunk && base <= g_hook.thunk && g_hook.thunk < addr) continue;

			for (std::size_t off = 0; off + need <= size; off += 0x10)
			{
				std::uintptr_t cand = base + off;
				if (ignore && cand == ignore) continue;
				if (region_is_padding(cand, need))
				{
					return cand;
				}
			}

			if (!fallback_rwx && mbi.Type == MEM_PRIVATE &&
				(mbi.Protect & 0xFF) == PAGE_EXECUTE_READWRITE && size >= need + 0x40)
			{
				std::uintptr_t cand = base + size - need;
				if (!ignore || cand != ignore) fallback_rwx = cand;
			}
		}
		return fallback_rwx;
	}

	std::uintptr_t alloc_exec_page()
	{
		std::uintptr_t p = (std::uintptr_t)VirtualAllocEx(memory->get_process_handle(), nullptr,
			page_sz(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!p) return 0;
		if (!is_executable_protect(query_protect(p)))
		{
			VirtualFreeEx(memory->get_process_handle(), (void*)p, 0, MEM_RELEASE);
			return 0;
		}
		return p;
	}

	bool install()
	{
		if (g_hook.installed) return true;

		std::uintptr_t base = memory->get_module_address();
		if (!base) return false;

		auto now = std::chrono::steady_clock::now();
		if (g_lastFail.time_since_epoch().count() != 0 &&
			now - g_lastFail < std::chrono::milliseconds(1500)) return false;

		std::uintptr_t slot = get_raycast_slot(base);
		if (!slot)
		{
			raycast_log("install: raycast slot not found");
			g_lastFail = now;
			return false;
		}
		std::uintptr_t fn = 0;
		if (!r_mem(slot, &fn, sizeof(fn)) || !addr_ok(fn))
		{
			g_lastFail = now;
			return false;
		}

		if (!g_hook.state)
		{
			g_hook.state = (std::uintptr_t)VirtualAllocEx(memory->get_process_handle(), nullptr,
				page_sz(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		}
		if (!g_hook.state)
		{
			g_lastFail = now;
			return false;
		}

		auto thunk = make_hook_thunk(g_hook.state, fn);
		if (thunk.size() > 0x200)
		{
			g_lastFail = now;
			return false;
		}

		bool owned = false;
		std::uintptr_t stub = 0, ignore_cave = 0;

		for (int attempt = 0; attempt < 8 && !stub; ++attempt)
		{
			std::uintptr_t cand = find_exec_cave(0x200, base, ignore_cave);
			if (!cand)
			{
				break;
			}
			SetLastError(0);
			DWORD old_prot = 0;
			bool prot_ok = protect_remote(cand, thunk.size(), PAGE_EXECUTE_READWRITE, &old_prot);
			SetLastError(0);
			if (!write_protected(cand, thunk.data(), thunk.size()))
			{
				if (prot_ok) protect_remote(cand, thunk.size(), old_prot, nullptr);
				ignore_cave = cand;
				continue;
			}
			stub = cand; owned = false;
		}

		if (!stub)
		{
			stub = alloc_exec_page();
			owned = (stub != 0);
			if (stub)
			{
				SetLastError(0);
				if (!write_protected(stub, thunk.data(), thunk.size()))
				{
					VirtualFreeEx(memory->get_process_handle(), (void*)stub, 0, MEM_RELEASE);
					stub = 0; owned = false;
				}
			}
		}

		if (!stub)
		{
			g_lastFail = now;
			return false;
		}

		RaycastState empty{};
		SetLastError(0);
		if (!w_mem(g_hook.state, &empty, sizeof(empty)))
		{
			g_lastFail = now;
			if (owned) VirtualFreeEx(memory->get_process_handle(), (void*)stub, 0, MEM_RELEASE);
			return false;
		}

		FlushInstructionCache(memory->get_process_handle(), (void*)stub, thunk.size());
		mark_cfg(stub);

		if (!is_executable_protect(query_protect(stub)))
		{
			g_lastFail = now;
			if (owned) VirtualFreeEx(memory->get_process_handle(), (void*)stub, 0, MEM_RELEASE);
			return false;
		}

		protect_remote(slot, 8, PAGE_READWRITE, nullptr);
		std::uintptr_t verify = 0;
		if (!write_protected(slot, &stub, sizeof(stub)) ||
			(!r_mem(slot, &verify, sizeof(verify)) || verify != stub))
		{
			g_lastFail = now;
			if (owned) VirtualFreeEx(memory->get_process_handle(), (void*)stub, 0, MEM_RELEASE);
			return false;
		}

		g_hook.slot             = slot;
		g_hook.module_base      = base;
		g_hook.originalFunction = fn;
		g_hook.thunk            = stub;
		g_hook.thunk_owned      = owned;
		g_hook.installed        = true;
		g_hook.active           = false;

		{
			char line[256];
			snprintf(line, sizeof(line), "HOOK installed slot=0x%llX orig=0x%llX thunk=0x%llX owned=%d state=0x%llX",
				static_cast<unsigned long long>(slot),
				static_cast<unsigned long long>(fn),
				static_cast<unsigned long long>(stub),
				owned ? 1 : 0,
				static_cast<unsigned long long>(g_hook.state));
			raycast_log(line);
		}
		return true;
	}

	void remove_hook()
	{
		if (g_hook.installed && addr_ok(g_hook.originalFunction) && g_hook.slot)
		{
			write_protected(g_hook.slot, &g_hook.originalFunction, sizeof(g_hook.originalFunction));
		}
		Sleep(50);
		if (g_hook.thunk && !g_hook.thunk_owned)
		{
			std::vector<std::uint8_t> pad(0x200, 0xCC);
			write_protected(g_hook.thunk, pad.data(), pad.size());
		}
		if (g_hook.thunk && g_hook.thunk_owned)
			VirtualFreeEx(memory->get_process_handle(), (void*)g_hook.thunk, 0, MEM_RELEASE);
		if (g_hook.state)
			VirtualFreeEx(memory->get_process_handle(), (void*)g_hook.state, 0, MEM_RELEASE);
		g_hook     = {};
		g_wallbang = false;
	}

	void ensure(bool want)
	{
		std::uintptr_t base = memory->get_module_address();
		static std::uintptr_t s_last_base = 0;

		if (g_hook.installed && base && s_last_base && base != s_last_base)
		{
			remove_hook();
		}
		if (base) s_last_base = base;

		if (want)
		{
			if (!base)
			{
				return;
			}

			static auto last_ensure_check = std::chrono::steady_clock::now();
			auto now = std::chrono::steady_clock::now();
			bool run_check = !g_hook.installed || (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_ensure_check).count() >= 250);

			if (run_check)
			{
				last_ensure_check = now;
				if (g_hook.installed && g_hook.thunk && g_hook.slot)
				{
					std::uintptr_t cur = 0;
					if (r_mem(g_hook.slot, &cur, sizeof(cur)) && addr_ok(cur))
					{
						if (cur != g_hook.thunk)
						{
							g_hook.installed = false;
						}
					}
					else
					{
						g_hook.installed = false;
					}
				}
				if (!g_hook.installed)
				{
					g_lastFail = {};
					install();
				}
			}
		}
		else if (g_hook.installed)
		{
			remove_hook();
		}
	}

	// Key-active evaluation mirror of main silentaim.cpp's key_active,
	// using keybind::imgui_key_to_vk (keybinds are ImGuiKey values).
	static bool key_active(int key, int mode, bool& latch_prev, bool& latch_state)
	{
		if (key == 0) return false;
		if (mode == 2) return true;
		const int vk = keybind::imgui_key_to_vk(static_cast<ImGuiKey>(key));
		if (vk == 0) return false;
		const bool pressed = (GetAsyncKeyState(vk) & 0x8000) != 0;
		if (mode == 0) return pressed;
		if (pressed && !latch_prev) latch_state = !latch_state;
		latch_prev = pressed;
		return latch_state;
	}

	void set_active(bool on, math::vector3 target_pos, std::uintptr_t target_part, math::vector3 cam_pos)
	{
		ensure(on);

		struct active_state_t
		{
			std::uint32_t active   = 0;
			std::uint32_t reserved = 0;
			float         target_x = 0.f;
			float         target_y = 0.f;
			float         target_z = 0.f;
			float         scale    = 1.15f;
		};
		static active_state_t last_written{};
		static bool s_last_on = false;

		if (!on)
		{
			if (s_last_on)
			{
				s_last_on = false;
			}
			if (g_hook.active && g_hook.state)
			{
				std::uint32_t v = 0;
				w_mem(g_hook.state, &v, sizeof(v));
				g_hook.active = false;
				last_written = {};
			}
			g_wallbang = false;
			restore_position_spoof();
			return;
		}
		if (!g_hook.installed)
		{
			return; // ensure() above already attempted install
		}
		if (!s_last_on)
		{
			s_last_on = true;
		}

		// Wallbang ("Magic Bullet"): sets the remote reserved flag so the
		// in-engine thunk rewrites the ray origin/direction to the target.
		// method 1 additionally runs the position-spoof thread; method 2
		// replaces the target point with a real raycast hit on the hitbox.
		const bool wallbang_on = settings::silentaim::wallbang;

		// Method 2 (Raycast): intersect the camera -> target ray with the target's own hitboxes
		// and hand the hook the exact entry point instead of the part centre. Methods 0 and 1
		// keep their old behaviour.
		auto raycast_surface_point = [](const math::vector3& target, math::vector3& out) -> bool
		{
			cache::entity_t ent{};
			{
				std::lock_guard<std::mutex> lk(cache::mtx);
				ent = silentaim::state.target;
			}

			if (!ent.instance.address || !game::camera)
				return false;

			math::vector3 origin{};
			try { origin = memory->read<math::vector3>(game::camera + Offsets::Camera::Position); }
			catch (...) { return false; }

			const math::vector3 delta = target - origin;
			const float distance = delta.length();
			if (!(distance > 0.5f))
				return false;

			const math::vector3 dir = delta.normalized();
			bool hit = false;
			float best_t = distance + 2.0f;

			for (const auto& part_pair : ent.parts)
			{
				rbx::part_t part = part_pair.second;
				if (!part.address)
					continue;

				rbx::primitive_t prim = part.get_primitive();
				if (!prim.address)
					continue;

				math::vector3 centre{}, size{};
				math::matrix3 rot{};
				try
				{
					centre = prim.get_position();
					size = prim.get_size();
					rot = prim.get_rotation();
				}
				catch (...)
				{
					continue;
				}

				if (size.x <= 0.f || size.y <= 0.f || size.z <= 0.f)
					continue;

				const math::vector3 half{ size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
				const math::vector3 axes[3] = {
					{ rot.m[0][0], rot.m[0][1], rot.m[0][2] },
					{ rot.m[1][0], rot.m[1][1], rot.m[1][2] },
					{ rot.m[2][0], rot.m[2][1], rot.m[2][2] }
				};

				const math::vector3 offset = centre - origin;
				float tmin = 0.f;
				float tmax = distance + 2.0f;
				bool miss = false;

				for (int axis = 0; axis < 3; ++axis)
				{
					const float e = axes[axis].dot(offset);
					const float f = axes[axis].dot(dir);
					const float abs_f = f < 0.f ? -f : f;

					if (abs_f > 0.000001f)
					{
						float t1 = (e + half[axis]) / f;
						float t2 = (e - half[axis]) / f;
						if (t1 > t2) { const float tmp = t1; t1 = t2; t2 = tmp; }
						if (t1 > tmin) tmin = t1;
						if (t2 < tmax) tmax = t2;
						if (tmax < tmin) { miss = true; break; }
					}
					else if (-e - half[axis] > 0.f || -e + half[axis] < 0.f)
					{
						miss = true;
						break;
					}
				}

				if (miss || tmin <= 0.f)
					continue;

				if (tmin < best_t)
				{
					best_t = tmin;
					hit = true;
				}
			}

			if (!hit)
				return false;

			out = origin + dir * best_t;
			return true;
		};

		math::vector3 aim_point = target_pos;
		if (wallbang_on && settings::silentaim::wallbang_method == 2)
		{
			math::vector3 surface{};
			if (raycast_surface_point(target_pos, surface))
				aim_point = surface;
		}

		active_state_t state{};
		state.active = 1;
		state.reserved = wallbang_on ? 1u : 0u;
		state.target_x = aim_point.x;
		state.target_y = aim_point.y;
		state.target_z = aim_point.z;
		// Fixed at the value the hook originally shipped with. The
		// game may apply this as a damage / falloff multiplier, but it
		// is not user-tunable - there is no damage setting in the menu.
		state.scale = 1.15f;

		if (state.active != last_written.active ||
			state.reserved != last_written.reserved ||
			state.target_x != last_written.target_x ||
			state.target_y != last_written.target_y ||
			state.target_z != last_written.target_z ||
			state.scale != last_written.scale)
		{
			if (w_mem(g_hook.state, &state, sizeof(state)))
			{
				last_written = state;
			}
		}

		g_hook.active = true;
		g_wallbang    = wallbang_on;

		// Position Spoof method — signal the target to the spoof thread.
		if (wallbang_on && settings::silentaim::wallbang_method == 1)
		{
			std::uintptr_t hrp_prim = 0;
			{
				std::lock_guard<std::mutex> lk(cache::mtx);
				auto& lp = cache::cached_local_player;
				auto it = lp.parts.find("HumanoidRootPart");
				if (it != lp.parts.end() && it->second.address)
					hrp_prim = it->second.get_primitive().address;
			}
			{
				std::lock_guard<std::mutex> lk(s_spoof_mtx);
				s_spoof_hrp_prim.store(hrp_prim);
				s_spoof_target_pos = { target_pos.x, target_pos.y + 1.0f, target_pos.z };
				s_spoof_active.store(true);
			}
		}
		else
		{
			s_spoof_active.store(false);
			s_spoof_hrp_prim.store(0);
		}
	}

	static void restore_position_spoof()
	{
		s_spoof_active.store(false);
		s_spoof_hrp_prim.store(0);
	}

	// Dedicated high-priority thread for invisible position spoofing. Writes
	// the spoof position, busy-waits ~0.3ms, then restores the real position.
	static void position_spoof_worker()
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		LARGE_INTEGER freq{};
		QueryPerformanceFrequency(&freq);
		const long long pulse_ticks = freq.QuadPart * 3 / 10000; // 0.3ms in QPC ticks

		while (true)
		{
			if (!s_spoof_active.load() || s_spoof_hrp_prim.load() == 0 ||
				settings::silentaim::wallbang_method != 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}

			std::uintptr_t prim;
			math::vector3 spoof_pos;
			{
				std::lock_guard<std::mutex> lk(s_spoof_mtx);
				prim      = s_spoof_hrp_prim.load();
				spoof_pos = s_spoof_target_pos;
			}

			if (!prim) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); continue; }

			math::vector3 real_pos = memory->read<math::vector3>(prim + Offsets::Primitive::Position);
			memory->write<math::vector3>(prim + Offsets::Primitive::Position, spoof_pos);

			LARGE_INTEGER start{}, now{};
			QueryPerformanceCounter(&start);
			do { QueryPerformanceCounter(&now); } while ((now.QuadPart - start.QuadPart) < pulse_ticks);

			memory->write<math::vector3>(prim + Offsets::Primitive::Position, real_pos);

			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
	}

	static rbx::part_t get_target_part_for_entity(const cache::entity_t& entity)
	{
		// Silent aim always targets the Head; if the entity has no Head
		// part (e.g. partial character) fall back to HumanoidRootPart.
		auto it = entity.parts.find("Head");
		if (it != entity.parts.end() && it->second.address)
			return it->second;
		auto hrp = entity.parts.find("HumanoidRootPart");
		if (hrp != entity.parts.end() && hrp->second.address)
			return hrp->second;
		return rbx::part_t(0);
	}

	void raycast_worker()
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

		auto last_status_log = std::chrono::steady_clock::now() - std::chrono::seconds(5);

		while (true)
		{
			// The legacy slot hook now only serves Viewport-mode wallbang; Raycast mode is
			// handled by raycast_silent, which patches the same function-pointer slot.
			const bool want = settings::silentaim::enabled && !settings::silentaim::raycast &&
				settings::silentaim::wallbang && settings::silentaim::wallbang_method != 2;

			{
				const auto now = std::chrono::steady_clock::now();
				if (now - last_status_log >= std::chrono::seconds(1))
				{
					last_status_log = now;
					char line[256];
					snprintf(line, sizeof(line), "STATUS want=%d installed=%d active=%d target=%llu tx=%.1f ty=%.1f calls=%llu",
						want ? 1 : 0,
						g_hook.installed ? 1 : 0,
						target_acquired ? 1 : 0,
						static_cast<unsigned long long>(target_address),
						target_screen_pos.x, target_screen_pos.y,
						static_cast<unsigned long long>(get_calls()));
					raycast_log(line);
				}
			}

			if (!want)
			{
				set_active(false);
				target_acquired = false;
				target_screen_pos = math::vector2{ -1.f, -1.f };
				target_address = 0;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			if (!memory->get_process_handle() || !game::datamodel.address)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			// The target is resolved by the main silent aim feature, so the
			// viewport method and the raycast method share the exact same
			// filtering (team / knock / gun / fov / prediction).
			const bool has_target = silentaim::is_active() &&
				silentaim::state.data_ready &&
				silentaim::state.target.instance.address != 0;

			if (has_target)
			{
				const math::vector3 target_pos = silentaim::state.target_world_pos;
				if (target_pos.x != 0.f || target_pos.y != 0.f || target_pos.z != 0.f)
				{
					math::vector3 cam_pos{};
					std::uint64_t camera = game::camera;
					if (game::workspace.address)
					{
						try
						{
							rbx::instance_t camera_instance = game::workspace.find_first_child_by_class("Camera");
							if (camera_instance.address)
								camera = camera_instance.address;
						}
						catch (...) {}
					}
					if (camera)
						cam_pos = memory->read<math::vector3>(camera + Offsets::Camera::Position);

					set_active(true, target_pos, silentaim::state.target.instance.address, cam_pos);
					target_acquired = true;
					target_screen_pos = silentaim::state.target_screen_pos;
					target_address = silentaim::state.target.instance.address;
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
					continue;
				}
			}

			set_active(false);
			target_acquired = false;
			target_screen_pos = math::vector2{ -1.f, -1.f };
			target_address = 0;
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void init()
	{
		static std::once_flag once;
		std::call_once(once, [] {
			std::thread(raycast_worker).detach();
			std::thread(position_spoof_worker).detach();
		});
	}

	void run() { init(); }

	bool is_installed()
	{
		return g_hook.installed;
	}

	std::uint64_t get_calls()
	{
		if (!g_hook.installed || !g_hook.state) return 0;
		std::uint64_t val = 0;
		r_mem(g_hook.state + offsetof(RaycastState, calls), &val, sizeof(val));
		return val;
	}
} // namespace raycast_silentaim
