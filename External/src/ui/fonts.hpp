#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>

namespace ui
{
    struct text_glyph
    {
        unsigned int codepoint;
        float x;
        float baseline;
    };

    bool load_fonts( );
    void reset_fonts( );
    bool fonts_ready( );

    void draw_glyph_run( ImDrawList* draw, int slot, ImVec2 origin, ImU32 color, const text_glyph* glyphs, int count );

    template < int count >
    void draw_glyph_run( ImDrawList* draw, int slot, ImVec2 origin, ImU32 color, const text_glyph ( &glyphs )[count] )
    {
        draw_glyph_run( draw, slot, origin, color, glyphs, count );
    }

    struct glyph_text
    {
        const text_glyph* glyphs;
        int count;

        template < int length >
        constexpr glyph_text( const text_glyph ( &table )[length] ) : glyphs( table ), count( length ) { }
    };

    inline void draw_glyph_run( ImDrawList* draw, int slot, ImVec2 origin, ImU32 color, glyph_text text )
    {
        draw_glyph_run( draw, slot, origin, color, text.glyphs, text.count );
    }

    void draw_text_box( ImDrawList* draw, int slot, ImVec2 min, ImVec2 max, int horizontal, int vertical, ImU32 color, const char* text );
    void draw_text_line( ImDrawList* draw, int slot, ImVec2 anchor, int align, ImU32 color, const char* text );
    void draw_icon_glyph( ImDrawList* draw, ImVec2 position, ImU32 color, const char* glyph );

    ImFont* slot_font( int slot );
    float slot_font_size( int slot );
    float slot_font_ascent( int slot );

    namespace text_fonts
    {
        inline constexpr int roboto_700_16 = 0;
        inline constexpr int roboto_700_18 = 1;
        inline constexpr int roboto_600_15_5 = 2;
        inline constexpr int roboto_600_16 = 3;
        inline constexpr int roboto_600_16_5 = 4;
        inline constexpr int roboto_600_17 = 5;
        inline constexpr int roboto_600_17_5 = 6;
        inline constexpr int roboto_600_18 = 7;
        inline constexpr int outfit_thin_600_16_5 = 8;
        inline constexpr int outfit_thin_600_17 = 9;
        inline constexpr int inter_600_15_5 = 10;
        inline constexpr int inter_800_18 = 11;
    }

    namespace icons
    {
        extern const char cursor_fill_glyph[];
    }
}
