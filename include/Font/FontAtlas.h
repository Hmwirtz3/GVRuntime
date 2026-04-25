#pragma once
#include <stdint.h>

// ------------------------------------------------------------
// Atlas size
// ------------------------------------------------------------
static const int FONT_ATLAS_WIDTH  = 128;
static const int FONT_ATLAS_HEIGHT = 128;

// ------------------------------------------------------------
// Raw atlas texture (from your .inl)
// ------------------------------------------------------------
extern unsigned char g_FontAtlasPixels[];

// ------------------------------------------------------------
// Glyph rectangle
// ------------------------------------------------------------
struct GlyphRect
{
    int x, y, w, h;
};

// ASCII 0–127
extern GlyphRect g_FontGlyphs[128];

void InitFontGlyphs();
