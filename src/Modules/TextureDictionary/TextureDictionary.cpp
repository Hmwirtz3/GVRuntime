#include "Modules/TextureDictionary/TextureDictionary.h"

#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"

#include <cstdint>
#include <pspgu.h>

namespace GV
{
    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    #define MAX_TEXTURES 256

    struct TextureRuntime
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t bufferWidth = 0;
        uint32_t format = 0;
        const void* data = nullptr;
    };

    static TextureRuntime g_textures[MAX_TEXTURES];

    void TextureDictionary::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* ptr = bytes.data() + start;
        const uint8_t* endPtr = bytes.data() + end;

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

            if (header->type == GV_CHUNK_TEXTURE_NATIVE)
            {
                uint32_t textureID = ReadUInt32(ptr);
                uint32_t width     = ReadUInt32(ptr);
                uint32_t height    = ReadUInt32(ptr);
                uint32_t format    = ReadUInt32(ptr);
                uint32_t dataSize  = ReadUInt32(ptr);

                Align16(ptr);
                const uint8_t* pixelData = ptr;

                if (textureID < MAX_TEXTURES)
                {
                    g_textures[textureID].width       = width;
                    g_textures[textureID].height      = height;
                    g_textures[textureID].format      = format;
                    g_textures[textureID].data        = pixelData;
                    g_textures[textureID].bufferWidth = width;
                }

                ptr += dataSize;
            }
            else
            {
                ptr += (header->size - sizeof(GV_ChunkHeader) - 4);
            }

            ptr = chunkEnd;
            Align16(ptr);
        }
    }

    void TextureDictionary::Bind(uint32_t textureID)
    {
        if (textureID >= MAX_TEXTURES)
            return;

        const TextureRuntime& tex = g_textures[textureID];

        if (!tex.data)
            return;

        sceGuEnable(GU_TEXTURE_2D);

        switch (tex.format)
        {
        case 0:
            sceGuTexMode(GU_PSM_5650, 0, 0, GU_FALSE);
            break;

        case 1:
            sceGuTexMode(GU_PSM_4444, 0, 0, GU_FALSE);
            sceGuEnable(GU_BLEND);
            sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
            break;

        default:
            sceGuTexMode(GU_PSM_5650, 0, 0, GU_FALSE);
            break;
        }

        sceGuTexImage(0, tex.width, tex.height, tex.bufferWidth, tex.data);

        sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
        sceGuTexWrap(GU_REPEAT, GU_REPEAT);
        sceGuTexFlush();
    }
}