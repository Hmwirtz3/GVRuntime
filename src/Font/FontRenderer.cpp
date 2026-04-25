#include "Font/FontRenderer.h"
#include "Font/FontAtlas.h"

#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include <malloc.h>
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <string>

struct FontVertex
{
    float    u, v;
    uint32_t color;
    float    x, y, z;
};

static const int MAX_FONT_VERTICES = 4096;
static const int LINE_HEIGHT       = 17;
static const int GLYPH_SPACING     = 1;
static const int MAX_LINES         = 32;

static uint32_t* m_textureRGBA = nullptr;
static void*     m_vertexBuffer = nullptr;
static int       m_vertexCount = 0;

static Glyph     m_glyphs[128];
static std::vector<std::string> m_lines;

void BeginUI()
{
    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumOrtho(0, 480, 272, 0, -1, 1);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    sceGumUpdateMatrix();

    sceGuDisable(GU_DEPTH_TEST);
    sceGuDisable(GU_CULL_FACE);
}

void Font_Init()
{
    const int atlasPixels = FONT_ATLAS_WIDTH * FONT_ATLAS_HEIGHT;

    m_textureRGBA = (uint32_t*)memalign(16, atlasPixels * sizeof(uint32_t));

    for (int i = 0; i < atlasPixels; i++)
    {
        uint8_t a = g_FontAtlasPixels[i];
        m_textureRGBA[i] = (a << 24) | 0x00FFFFFF;
    }

    sceKernelDcacheWritebackRange(
        m_textureRGBA,
        atlasPixels * sizeof(uint32_t)
    );

    m_vertexBuffer = memalign(16, MAX_FONT_VERTICES * sizeof(FontVertex));
    m_vertexCount  = 0;

    const float fw = (float)FONT_ATLAS_WIDTH;
    const float fh = (float)FONT_ATLAS_HEIGHT;
    const float inset = 0.25f;

    for (int i = 0; i < 128; i++)
    {
        Glyph& g   = m_glyphs[i];
        g.u0       = 0.0f;
        g.v0       = 0.0f;
        g.u1       = 0.0f;
        g.v1       = 0.0f;
        g.width    = 0;
        g.height   = 0;
        g.bearingX = 0;
        g.bearingY = 0;
        g.advance  = 0;
    }

    for (int c = 32; c < 128; c++)
    {
        const GlyphRect& r = g_FontGlyphs[c];

        if (r.w == 0 || r.h == 0)
            continue;

        Glyph& g = m_glyphs[c];

        g.u0 = (r.x + inset) / fw;
        g.u1 = (r.x + r.w - inset) / fw;
        g.v0 = (r.y + inset) / fh;
        g.v1 = (r.y + r.h - inset) / fh;

        g.width    = r.w;
        g.height   = r.h;
        g.bearingX = 0;
        g.bearingY = 0;
        g.advance  = r.w + GLYPH_SPACING;
    }

    if (m_glyphs[32].advance == 0)
        m_glyphs[32].advance = 4;
}

void Font_Shutdown()
{
    if (m_textureRGBA)
    {
        free(m_textureRGBA);
        m_textureRGBA = nullptr;
    }

    if (m_vertexBuffer)
    {
        free(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }

    m_vertexCount = 0;
}

void Font_Begin()
{
    m_vertexCount = 0;

    sceGuDisable(GU_DEPTH_TEST);
    sceGuDisable(GU_CULL_FACE);

    sceGuEnable(GU_TEXTURE_2D);
    sceGuEnable(GU_BLEND);

    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);

    sceGuTexImage(
        0,
        FONT_ATLAS_WIDTH,
        FONT_ATLAS_HEIGHT,
        FONT_ATLAS_WIDTH,
        m_textureRGBA
    );
}

void Font_End()
{
    if (m_vertexCount > 0 && m_vertexBuffer)
    {
        sceKernelDcacheWritebackRange(
            m_vertexBuffer,
            m_vertexCount * (int)sizeof(FontVertex)
        );

       sceGuDrawArray(
    GU_TRIANGLES,
    GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
    m_vertexCount,
    0,
    m_vertexBuffer
);
    }

    sceGuDisable(GU_TEXTURE_2D);
    sceGuDisable(GU_BLEND);
}

static void DrawChar(const Glyph& g, float x, float y, uint32_t color)
{
    if (!m_vertexBuffer) return;
    if (m_vertexCount + 6 > MAX_FONT_VERTICES) return;
    if (g.width <= 0 || g.height <= 0) return;

    const float x0 = x;
    const float y0 = y;
    const float x1 = x + (float)g.width;
    const float y1 = y + (float)g.height;

    FontVertex* buf = (FontVertex*)m_vertexBuffer;
    FontVertex* v   = &buf[m_vertexCount];

    v[0] = { g.u0, g.v0, color, x0, y0, 0.0f };
    v[1] = { g.u1, g.v0, color, x1, y0, 0.0f };
    v[2] = { g.u0, g.v1, color, x0, y1, 0.0f };

    v[3] = { g.u0, g.v1, color, x0, y1, 0.0f };
    v[4] = { g.u1, g.v0, color, x1, y0, 0.0f };
    v[5] = { g.u1, g.v1, color, x1, y1, 0.0f };

    m_vertexCount += 6;
}

void DrawText(const char* text, float x, float y, uint32_t color)
{
    const float startX = x;
    const float maxX   = 480.0f - 10.0f;

    float penX = x;
    float penY = y;

    while (*text)
    {
        unsigned char c = (unsigned char)(*text++);

        if (c == '\n')
        {
            penX = startX;
            penY += (float)LINE_HEIGHT;
            continue;
        }

        if (c >= 128)
            continue;

        const Glyph& g = m_glyphs[c];

        if (penX + g.advance > maxX)
        {
            penX = startX;
            penY += (float)LINE_HEIGHT;
        }

        DrawChar(g, penX, penY, color);

        penX += (float)g.advance;
    }
}

void Log(const char* fmt, ...)
{
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    const int maxVisibleLines = 272 / LINE_HEIGHT;

    if ((int)m_lines.size() >= maxVisibleLines)
    {
        m_lines.clear();
    }

    m_lines.push_back(buf);
}

void DrawDebug()
{
    float y = 10.0f;

    for (const auto& line : m_lines)
    {
        DrawText(line.c_str(), 10.0f, y, 0xFFFFFFFF);
        y += (float)LINE_HEIGHT;
    }
}