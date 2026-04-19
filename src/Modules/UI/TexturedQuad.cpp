#include "Modules/UI/TexturedQuad.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <cstdio>
#include <vector>
#include <string>
#include <pspiofilemgr.h>
#include <stdarg.h>

namespace GV
{
    // --------------------------------------------------
    // Logging (file-based)
    // --------------------------------------------------
    static void WriteLog(const char* fmt, ...)
    {
        SceUID fd = sceIoOpen("QuadLog.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
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

    // --------------------------------------------------
    // Internal storage
    // --------------------------------------------------
    static std::vector<TexturedQuadData> g_quads;

    // --------------------------------------------------
    // Alignment helper
    // --------------------------------------------------
    static void Align16(const uint8_t*& ptr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;
        ptr = (const uint8_t*)p;
    }

    // --------------------------------------------------
    // Load
    // --------------------------------------------------
    void TexturedQuad::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        printf("TexturedQuad::Load [0x%lX - 0x%lX]\n",
            (unsigned long)start,
            (unsigned long)end);

        WriteLog("[LOAD] TexturedQuad [%lu - %lu]\n",
            (unsigned long)start,
            (unsigned long)end);

        const uint8_t* ptr = bytes.data() + start;
        const uint8_t* endPtr = bytes.data() + end;

        if (ptr >= endPtr)
            return;

        const GV_ChunkHeader* objHeader = (const GV_ChunkHeader*)ptr;
        ptr += sizeof(GV_ChunkHeader);

        const GV_ChunkHeader* header = (const GV_ChunkHeader*)ptr;
        ptr += sizeof(GV_ChunkHeader);

        Align16(ptr);

        uint32_t paramCount = ReadUInt32(ptr);
        (void)paramCount;
        (void)objHeader;
        (void)header;

        TexturedQuadData q{};

        q.posX    = ReadFloat(ptr);
        q.posY    = ReadFloat(ptr);
        q.width   = ReadInt(ptr);
        q.height  = ReadInt(ptr);
        q.visible = ReadBool(ptr);

        const char* activateMsg = ReadString(ptr);
        q.activateMessage = activateMsg;

        Align16(ptr);

        q.textureID = ReadUInt32(ptr);

        uint32_t index = (uint32_t)g_quads.size();

        WriteLog("[LOAD] Quad idx=%lu TexID=%lu Visible=%d Msg=%s\n",
            (unsigned long)index,
            (unsigned long)q.textureID,
            q.visible,
            q.activateMessage.c_str());

        g_quads.push_back(q);

        MessageHandler::Register(q.activateMessage, GV_CHUNK_TEXTURE, index);

        WriteLog("[LOAD] Registered msg='%s' -> type=0x%X index=%lu\n",
            q.activateMessage.c_str(),
            GV_CHUNK_TEXTURE,
            (unsigned long)index);
    }

    // --------------------------------------------------
    // Message handling
    // --------------------------------------------------
    void TexturedQuad::HandleMessage(uint32_t index, const std::string& msg)
    {
        WriteLog("[MSG] Received msg='%s' for index=%lu\n",
            msg.c_str(),
            (unsigned long)index);

        if (index >= g_quads.size())
        {
            WriteLog("[MSG] INVALID INDEX %lu\n", (unsigned long)index);
            return;
        }

        TexturedQuadData& q = g_quads[index];

        WriteLog("[MSG] Quad[%lu] current visible=%d expectedMsg=%s\n",
            (unsigned long)index,
            q.visible,
            q.activateMessage.c_str());

        if (msg == q.activateMessage)
        {
            q.visible = true;

            WriteLog("[MSG] MATCH -> Quad[%lu] SET visible=1\n",
                (unsigned long)index);

            printf("TexturedQuad[%lu] activated via message '%s'\n",
                (unsigned long)index,
                msg.c_str());
        }
        else
        {
            WriteLog("[MSG] NO MATCH (msg=%s, expected=%s)\n",
                msg.c_str(),
                q.activateMessage.c_str());
        }
    }

    // --------------------------------------------------
    // Build GPU data
    // --------------------------------------------------
    void TexturedQuad::BuildRenderData(uint32_t index, void* dst)
    {
        if (index >= g_quads.size() || !dst)
            return;

        const TexturedQuadData& q = g_quads[index];
        QuadGpuData* out = (QuadGpuData*)dst;

        const float x = q.posX;
        const float y = q.posY;
        const float w = (float)q.width;
        const float h = (float)q.height;

        const float x0 = x;
        const float y0 = y;
        const float x1 = x + w;
        const float y1 = y + h;

        out->verts[0] = { 0.0f, 0.0f, x0, y0, 0.0f };
        out->verts[1] = { w,    0.0f, x1, y0, 0.0f };
        out->verts[2] = { w,    h,    x1, y1, 0.0f };

        out->verts[3] = { 0.0f, 0.0f, x0, y0, 0.0f };
        out->verts[4] = { w,    h,    x1, y1, 0.0f };
        out->verts[5] = { 0.0f, h,    x0, y1, 0.0f };

        out->textureID = q.textureID;
    }

    uint32_t TexturedQuad::GetCount()
    {
        return (uint32_t)g_quads.size();
    }

    const TexturedQuadData& TexturedQuad::Get(uint32_t index)
    {
        return g_quads[index];
    }
}