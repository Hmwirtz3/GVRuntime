#include "Modules/TextureDictionary/TextureDictionary.h"

#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"

#include <cstdio>
#include <cstdint>
#include <pspgu.h>
#include <pspiofilemgr.h>
#include <stdarg.h>

namespace GV
{
    
    static void WriteLog(const char* fmt, ...)
    {
        SceUID fd = sceIoOpen("TxDictlog.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
        if (fd < 0) return;

        char buf[256];
        buf[0] = '\0';

        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if (len > 0 && len < (int)sizeof(buf))
            sceIoWrite(fd, buf, len);

        sceIoClose(fd);
    }

    
    static void Align16(const uint8_t*& ptr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;
        ptr = (const uint8_t*)p;
    }

   
    #define MAX_TEXTURES 256

    struct TextureRuntime
    {
        uint32_t width  = 0;
        uint32_t height = 0;
        uint32_t format = 0;
        const void* data = nullptr;
    };

    static TextureRuntime g_textures[MAX_TEXTURES];


    void TextureDictionary::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        printf("TextureDictionary::Load [%lu - %lu]\n",
            (unsigned long)start,
            (unsigned long)end);

        WriteLog("[DISPATCH] TextureDictionary [0x%lX - 0x%lX]\n",
            (unsigned long)start,
            (unsigned long)end);

        const uint8_t* ptr = bytes.data() + start;
        const uint8_t* endPtr = bytes.data() + end;

        if (ptr >= endPtr)
        {
            WriteLog("[TextureDictionary] Invalid range\n");
            return;
        }

        const uint8_t* dictStart = ptr;

        const GV_ChunkHeader* dictHeader = (const GV_ChunkHeader*)ptr;
        ptr += sizeof(GV_ChunkHeader);

        WriteLog("Outer Chunk Type=0x%lX Size=0x%lX Version=0x%lX\n",
            (unsigned long)dictHeader->type,
            (unsigned long)dictHeader->size,
            (unsigned long)dictHeader->version);

        if (dictHeader->type != GV_CHUNK_TEXDICTIONARY)
        {
            WriteLog("[TextureDictionary] ERROR: Expected 0x%X got 0x%lX\n",
                GV_CHUNK_TEXDICTIONARY,
                (unsigned long)dictHeader->type);
            return;
        }

        const uint8_t* dictEnd = dictStart + dictHeader->size;
        if (dictEnd > endPtr)
            dictEnd = endPtr;

        Align16(ptr);

        while (ptr < dictEnd)
        {
            const uint8_t* chunkStart = ptr;

            const GV_ChunkHeader* header = (const GV_ChunkHeader*)ptr;
            ptr += sizeof(GV_ChunkHeader);

            Align16(ptr);

            WriteLog("Child Chunk Type=0x%lX Size=0x%lX Version=0x%lX\n",
                (unsigned long)header->type,
                (unsigned long)header->size,
                (unsigned long)header->version);

            if (header->type == GV_CHUNK_TEXTURE_NATIVE)
            {
                const uint8_t* chunkEnd = chunkStart + header->size;

                uint32_t textureID = ReadUInt32(ptr);
                uint32_t width     = ReadUInt32(ptr);
                uint32_t height    = ReadUInt32(ptr);
                uint32_t format    = ReadUInt32(ptr);
                uint32_t dataSize  = ReadUInt32(ptr);

                WriteLog("Texture ID=0x%lX W=%lu H=%lu Format=0x%lX Size=%lu\n",
                    (unsigned long)textureID,
                    (unsigned long)width,
                    (unsigned long)height,
                    (unsigned long)format,
                    (unsigned long)dataSize);

                const uint8_t* pixelData = ptr;

                if (textureID < MAX_TEXTURES)
                {
                    g_textures[textureID].width  = width;
                    g_textures[textureID].height = height;
                    g_textures[textureID].format = format;
                    g_textures[textureID].data   = pixelData;
                }

                ptr += dataSize;
                ptr = chunkEnd;
            }
            else
            {
                ptr += (header->size - sizeof(GV_ChunkHeader));
            }

            Align16(ptr);
        }

        WriteLog("[TextureDictionary] Done\n");
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
    case 0: // RGB565
        sceGuTexMode(GU_PSM_5650, 0, 0, GU_FALSE);
        break;

    case 1: // RGBA4444
        sceGuTexMode(GU_PSM_4444, 0, 0, GU_FALSE);

        
        sceGuEnable(GU_BLEND);
        sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
        break;

    default:
        // Fallback (safe default)
        sceGuTexMode(GU_PSM_5650, 0, 0, GU_FALSE);
        break;
    }

    sceGuTexImage(0, tex.width, tex.height, tex.width, tex.data);

    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
    sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuTexFlush();
}
}