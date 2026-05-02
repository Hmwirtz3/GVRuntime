#pragma once

#include <stdint.h>

// ------------------------------------------------------------
// Glyph (same as before)
// ------------------------------------------------------------
struct Glyph
{
    float u0, v0;
    float u1, v1;

    int width;
    int height;

    int bearingX;
    int bearingY;

    int advance;
};




void Font_Init();
void Font_Shutdown();


void Font_Begin();
void Font_End();
void BeginUI();

// rendering
void DrawText(const char* text, float x, float y, float scale, uint32_t color);

// debug logging
void Log(const char* fmt, ...);
void DrawDebug();