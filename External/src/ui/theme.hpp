#pragma once

#include <imgui.h>

// ============================================================
//  Bankroll Mafia theme
//  Deep charcoal + near-black backgrounds, ultra-dark purple
//  accent (#1a0a1f tone), bright white/light-grey text.
//  Accent colour: purple  IM_COL32(168, 130, 210, 255)
// ============================================================
namespace ui::theme
{
    inline constexpr float frame_width  = 1149.f;
    inline constexpr float frame_height = 748.f;

    // ── backgrounds ───────────────────────────────────────────
    inline constexpr ImU32 background  = IM_COL32(  9,   9,   9, 255 );  // near-black
    inline constexpr ImU32 surface     = IM_COL32( 18,  16,  20, 255 );  // bankroll card bg
    inline constexpr ImU32 surface_2   = IM_COL32( 22,  20,  27, 255 );
    inline constexpr ImU32 surface_3   = IM_COL32( 13,  13,  13, 255 );  // tab bg
    inline constexpr ImU32 surface_4   = IM_COL32( 16,  14,  18, 255 );  // dropdown bg
    inline constexpr ImU32 surface_5   = IM_COL32( 26,  24,  30, 255 );  // slider track
    inline constexpr ImU32 surface_6   = IM_COL32(  6,   5,   7, 255 );  // config list bg
    inline constexpr ImU32 surface_7   = IM_COL32( 35,  32,  44, 255 );  // selected item
    inline constexpr ImU32 surface_8   = IM_COL32( 18,  18,  22, 255 );  // widget top
    inline constexpr ImU32 surface_9   = IM_COL32( 10,  10,  14, 255 );  // widget bottom

    // ── borders ───────────────────────────────────────────────
    inline constexpr ImU32 border      = IM_COL32( 22,  22,  22, 255 );  // section border
    inline constexpr ImU32 border_2    = IM_COL32( 50,  47,  58, 255 );  // dropdown border

    // ── text ──────────────────────────────────────────────────
    inline constexpr ImU32 text        = IM_COL32( 205, 202, 215, 255 );  // bright
    inline constexpr ImU32 text_muted  = IM_COL32( 168, 165, 178, 255 );  // normal
    inline constexpr ImU32 text_subtle = IM_COL32( 140, 137, 150, 255 );
    inline constexpr ImU32 text_faint  = IM_COL32(  82,  79,  95, 255 );  // dim
    inline constexpr ImU32 text_5      = IM_COL32(  64,  61,  75, 255 );  // bind text
    inline constexpr ImU32 icon        = IM_COL32( 255, 255, 255, 255 );

    // ── accent – bankroll purple ──────────────────────────────
    inline constexpr ImU32 accent      = IM_COL32( 168, 130, 210, 255 );  // purple
    inline constexpr ImU32 accent_hover= IM_COL32( 188, 148, 228, 255 );
    inline constexpr ImU32 accent_dark = IM_COL32(  60,  38,  60, 255 );

    // ── fill colours (replaces red fills with purple-dark) ───
    inline constexpr ImU32 fill_blue   = IM_COL32( 18,  16,  24, 255 );
    inline constexpr ImU32 fill_red    = IM_COL32( 50,  32,  64, 255 );  // selection fill
    inline constexpr ImU32 fill_red_2  = IM_COL32( 65,  45,  80, 255 );  // hover fill
    inline constexpr ImU32 fill_red_3  = IM_COL32( 90,  55, 110, 255 );  // active fill

    // ── gradient / divider helpers ────────────────────────────
    inline constexpr ImU32 divider     = IM_COL32( 46,  44,  55, 255 );
    inline constexpr ImU32 col_hdr     = IM_COL32( 85,  82,  98, 255 );  // section label
}
