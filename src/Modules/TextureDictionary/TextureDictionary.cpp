#include "Modules/TextureDictionary/TextureDictionary.h"

#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"

#include <cstdint>
#include <pspgu.h>
#include <vector>

namespace GV
{
    // --------------------------------------------------
    // Align helper
    // --------------------------------------------------
    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    // --------------------------------------------------
    // Runtime texture storage (unbounded)
    // --------------------------------------------------
    struct TextureRuntime
    {
        uint32_t id = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t bufferWidth = 0;
        uint32_t format = 0;
        const void* data = nullptr;
        uint32_t dataSize = 0;
    };

    static std::vector<TextureRuntime> g_textures;

    // --------------------------------------------------
    // Lookup
    // --------------------------------------------------
    static TextureRuntime* FindTexture(uint32_t id)
    {
        for (size_t i = 0; i < g_textures.size(); i++)
        {
            if (g_textures[i].id == id)
                return &g_textures[i];
        }
        return nullptr;
    }

    // --------------------------------------------------
    // Load (no dropping textures)
    // --------------------------------------------------
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

        g_textures.clear();

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

                TextureRuntime tex{};
                tex.id = textureID;
                tex.width = width;
                tex.height = height;
                tex.format = format;
                tex.data = pixelData;
                tex.bufferWidth = width;
                tex.dataSize = dataSize;

                g_textures.push_back(tex);

                ptr += dataSize;
            }
            else
            {
                uint32_t skip = header->size - sizeof(GV_ChunkHeader) - 4;
                ptr += skip;
            }

            ptr = chunkEnd;
            Align16(ptr);
        }
    }

    // --------------------------------------------------
    // Bind
    // --------------------------------------------------
    void TextureDictionary::Bind(uint32_t textureID)
    {
        TextureRuntime* tex = FindTexture(textureID);

        if (!tex || !tex->data)
            return;

        sceGuEnable(GU_TEXTURE_2D);

        switch (tex->format)
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

        sceGuTexImage(0, tex->width, tex->height, tex->bufferWidth, tex->data);
        sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
        sceGuTexWrap(GU_REPEAT, GU_REPEAT);
        sceGuTexFlush();
    }
}