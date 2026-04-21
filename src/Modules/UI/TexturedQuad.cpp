#include "Modules/UI/TexturedQuad.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <cstdio>
#include <vector>
#include <string>

namespace GV
{
    static std::vector<TexturedQuadData> g_quads;

    static void Align16(const uint8_t*& ptr)
    {
        uintptr_t p = ((uintptr_t)ptr + 15) & ~15;
        ptr = (const uint8_t*)p;
    }

    void TexturedQuad::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* ptr = bytes.data() + start;
        const uint8_t* endPtr = bytes.data() + end;

        if (ptr >= endPtr)
            return;

        const GV_ChunkHeader* objHeader = (const GV_ChunkHeader*)ptr;
        (void)objHeader;
        ptr += sizeof(GV_ChunkHeader);

        const GV_ChunkHeader* header = (const GV_ChunkHeader*)ptr;
        (void)header;
        ptr += sizeof(GV_ChunkHeader);

        Align16(ptr);

        uint32_t paramCount = ReadUInt32(ptr);
        (void)paramCount;

        TexturedQuadData q{};

        q.posX = ReadFloat(ptr);
        q.posY = ReadFloat(ptr);
        q.width = ReadInt(ptr);
        q.height = ReadInt(ptr);
        q.visible = ReadBool(ptr);

        const char* activateMsg = ReadString(ptr);
        q.activateMessage = activateMsg;

        Align16(ptr);

        q.textureID = ReadUInt32(ptr);

        uint32_t index = (uint32_t)g_quads.size();
        g_quads.push_back(q);

        MessageHandler::Register(q.activateMessage, GV_CHUNK_TEXTURE, index);
    }

    void TexturedQuad::HandleMessage(uint32_t index, const std::string& msg)
    {
        if (index >= g_quads.size())
            return;

        TexturedQuadData& q = g_quads[index];

        if (msg == q.activateMessage)
        {
            q.visible = true;
        }
    }

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