#include "Modules/TextureDictionary/TextureDictionary.h"

#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"

#include <cstdint>
#include <cstring>
#include <pspgu.h>
#include <vector>

namespace GV
{
    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    static uint16_t g_fallbackTexture[64 * 64];
    static bool g_fallbackTextureBuilt = false;

    static uint16_t Make4444(
        uint8_t r,
        uint8_t g,
        uint8_t b,
        uint8_t a)
    {
        return
            ((a >> 4) << 12) |
            ((b >> 4) << 8)  |
            ((g >> 4) << 4)  |
            ((r >> 4) << 0);
    }

    static void BuildFallbackTexture()
    {
        if (g_fallbackTextureBuilt)
            return;

        g_fallbackTextureBuilt = true;

        for (uint32_t y = 0; y < 64; y++)
        {
            for (uint32_t x = 0; x < 64; x++)
            {
                bool checker =
                    (((x / 8) + (y / 8)) & 1) != 0;

                uint16_t color = 0;

                if (x < 32 && y < 32)
                {
                    color = checker
                        ? Make4444(255, 0, 255, 255)
                        : Make4444(0, 0, 0, 255);
                }
                else if (x >= 32 && y < 32)
                {
                    color = checker
                        ? Make4444(255, 255, 0, 255)
                        : Make4444(0, 0, 0, 255);
                }
                else if (x < 32 && y >= 32)
                {
                    color = checker
                        ? Make4444(0, 255, 0, 255)
                        : Make4444(0, 0, 0, 255);
                }
                else
                {
                    color = checker
                        ? Make4444(255, 0, 0, 255)
                        : Make4444(0, 0, 0, 255);
                }

                g_fallbackTexture[(y * 64) + x] = color;
            }
        }
    }

    static void BindFallbackTexture()
    {
        BuildFallbackTexture();

        sceGuEnable(GU_TEXTURE_2D);

        sceGuTexMode(
            GU_PSM_4444,
            0,
            0,
            GU_FALSE);

        sceGuTexImage(
            0,
            64,
            64,
            64,
            g_fallbackTexture);

        sceGuTexFunc(
            GU_TFX_REPLACE,
            GU_TCC_RGBA);

        sceGuTexFilter(
            GU_NEAREST,
            GU_NEAREST);

        sceGuTexWrap(
            GU_REPEAT,
            GU_REPEAT);

        sceGuTexFlush();
    }

    struct TextureRuntime
    {
        uint32_t id = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t bufferWidth = 0;
        uint32_t format = 0;
        std::vector<uint8_t> data;
        uint32_t dataSize = 0;
    };

    static std::vector<TextureRuntime> g_textures;

    static TextureRuntime* FindTexture(uint32_t id)
    {
        for (size_t i = 0; i < g_textures.size(); i++)
        {
            if (g_textures[i].id == id)
                return &g_textures[i];
        }

        return nullptr;
    }

    void TextureDictionary::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base = bytes.data();
        const uint8_t* ptr = base + start;
        const uint8_t* endPtr = base + end;

        if (ptr >= endPtr)
            return;

        const uint8_t* dictStart = ptr;
        const GV_ChunkHeader* dictHeader = (const GV_ChunkHeader*)ptr;

        ptr += sizeof(GV_ChunkHeader);
        ptr += 4;

        const uint8_t* dictEnd = dictStart + dictHeader->size;

        if (dictEnd > endPtr)
            dictEnd = endPtr;

        while (ptr < dictEnd)
        {
            const uint8_t* chunkStart = ptr;
            const GV_ChunkHeader* header = (const GV_ChunkHeader*)ptr;

            ptr += sizeof(GV_ChunkHeader);
            ptr += 4;

            const uint8_t* chunkEnd = chunkStart + header->size;

            if (chunkEnd > dictEnd)
                break;

            if (header->type == GV_CHUNK_TEXTURE_NATIVE)
            {
                uint32_t textureID = ReadUInt32(ptr);
                uint32_t width     = ReadUInt32(ptr);
                uint32_t height    = ReadUInt32(ptr);
                uint32_t format    = ReadUInt32(ptr);
                uint32_t dataSize  = ReadUInt32(ptr);

                Align16(ptr);

                const uint8_t* pixelData = ptr;

                TextureRuntime* existing =
                    FindTexture(textureID);

                if (!existing)
                {
                    TextureRuntime tex{};

                    tex.id = textureID;
                    tex.width = width;
                    tex.height = height;
                    tex.format = format;

                    uint32_t bytesPerPixel = 2;

                    // PSP textures need aligned stride
                    tex.bufferWidth = (width + 7) & ~7;

                    uint32_t srcRowBytes =
                        width * bytesPerPixel;

                    uint32_t dstRowBytes =
                        tex.bufferWidth * bytesPerPixel;

                    tex.dataSize =
                        dstRowBytes * height;

                    tex.data.resize(tex.dataSize);

                    std::memset(
                        tex.data.data(),
                        0,
                        tex.dataSize);

                    for (uint32_t y = 0; y < height; y++)
                    {
                        const uint8_t* src =
                            pixelData +
                            (y * srcRowBytes);

                        uint8_t* dst =
                            tex.data.data() +
                            (y * dstRowBytes);

                        std::memcpy(
                            dst,
                            src,
                            srcRowBytes);
                    }

                    g_textures.push_back(tex);
                }

                ptr += dataSize;
            }
            else
            {
                uint32_t skip =
                    header->size -
                    sizeof(GV_ChunkHeader) -
                    4;

                ptr += skip;
            }

            ptr = chunkEnd;
            Align16(ptr);
        }
    }

    bool TextureDictionary::Bind(uint32_t textureID)
    {
        TextureRuntime* tex = FindTexture(textureID);

        if (!tex || tex->data.empty())
        {
            BindFallbackTexture();
            return true;
        }

        sceGuEnable(GU_TEXTURE_2D);

        switch (tex->format)
        {
            case 0:
                sceGuTexMode(
                    GU_PSM_5650,
                    0,
                    0,
                    GU_FALSE);
                break;

            case 1:
                sceGuTexMode(
                    GU_PSM_4444,
                    0,
                    0,
                    GU_FALSE);

                sceGuEnable(GU_BLEND);

                sceGuBlendFunc(
                    GU_ADD,
                    GU_SRC_ALPHA,
                    GU_ONE_MINUS_SRC_ALPHA,
                    0,
                    0);

                break;

            default:
                sceGuTexMode(
                    GU_PSM_5650,
                    0,
                    0,
                    GU_FALSE);
                break;
        }

        sceGuTexImage(
            0,
            tex->width,
            tex->height,
            tex->bufferWidth,
            tex->data.data());

        sceGuTexFunc(
            GU_TFX_REPLACE,
            GU_TCC_RGBA);

        sceGuTexFilter(
            GU_LINEAR,
            GU_LINEAR);

        sceGuTexWrap(
            GU_REPEAT,
            GU_REPEAT);

        sceGuTexFlush();

        return true;
    }
}