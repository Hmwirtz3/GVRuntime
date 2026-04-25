#include "Modules/UI/TexturedQuad.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <vector>
#include <string>
#include <cstring>

namespace GV
{
    static std::vector<TexturedQuadData> g_quads;

    static void Align16(const uint8_t*& ptr, const uint8_t* endPtr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;

        const uint8_t* aligned = (const uint8_t*)p;

        if (aligned <= endPtr)
            ptr = aligned;
    }

    static bool ReadChunkHeaderSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        GV_ChunkHeader& outHeader)
    {
        if (ptr + sizeof(GV_ChunkHeader) > endPtr)
            return false;

        std::memcpy(&outHeader, ptr, sizeof(GV_ChunkHeader));
        return true;
    }

    static bool ReadUInt32Safe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        uint32_t& outValue)
    {
        if (ptr + sizeof(uint32_t) > endPtr)
            return false;

        std::memcpy(&outValue, ptr, sizeof(uint32_t));
        ptr += sizeof(uint32_t);
        return true;
    }

    static bool ReadIntSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        int& outValue)
    {
        if (ptr + sizeof(int) > endPtr)
            return false;

        std::memcpy(&outValue, ptr, sizeof(int));
        ptr += sizeof(int);
        return true;
    }

    static bool ReadFloatSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        float& outValue)
    {
        if (ptr + sizeof(float) > endPtr)
            return false;

        std::memcpy(&outValue, ptr, sizeof(float));
        ptr += sizeof(float);
        return true;
    }

    static bool ReadBoolSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        bool& outValue)
    {
        if (ptr + sizeof(uint8_t) > endPtr)
            return false;

        uint8_t v = *ptr;
        ptr += sizeof(uint8_t);

        outValue = (v != 0);
        return true;
    }

    static bool ReadStringSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        std::string& outValue)
    {
        uint32_t len = 0;

        if (!ReadUInt32Safe(ptr, endPtr, len))
            return false;

        if (ptr + len > endPtr)
            return false;

        outValue.assign((const char*)ptr, len);
        ptr += len;

        return true;
    }

    void TexturedQuad::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base = bytes.data();
        const uint8_t* ptr = base + start;
        const uint8_t* endPtr = base + end;

        if (start >= bytes.size() || end > bytes.size() || ptr >= endPtr)
            return;

        // Skip SceneObject header
        ptr += sizeof(GV_ChunkHeader) + 4;

        // Read inner chunk header
        GV_ChunkHeader header{};
        if (!ReadChunkHeaderSafe(ptr, endPtr, header))
            return;

        if (header.type != GV_CHUNK_TEXTURE)
            return;

        const uint8_t* innerStart = ptr;
        const uint8_t* innerEnd = innerStart + header.size;

        if (innerEnd > endPtr)
            return;

        // Move into chunk payload
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = 0;
        if (!ReadUInt32Safe(ptr, innerEnd, paramCount))
            return;

        if (paramCount < 7)
            return;

        TexturedQuadData q{};

        if (!ReadFloatSafe(ptr, innerEnd, q.posX)) return;
        if (!ReadFloatSafe(ptr, innerEnd, q.posY)) return;
        if (!ReadIntSafe(ptr, innerEnd, q.width)) return;
        if (!ReadIntSafe(ptr, innerEnd, q.height)) return;
        if (!ReadBoolSafe(ptr, innerEnd, q.visible)) return;
        if (!ReadStringSafe(ptr, innerEnd, q.activateMessage)) return;
        if (!ReadStringSafe(ptr, innerEnd, q.deactivateMessage)) return;

        // Move to payload after chunk
        ptr = innerEnd;
        Align16(ptr, endPtr);

        if (!ReadUInt32Safe(ptr, endPtr, q.textureID))
            return;

        uint32_t index = (uint32_t)g_quads.size();
        g_quads.push_back(q);

        if (!q.activateMessage.empty())
            MessageHandler::Register(q.activateMessage, GV_CHUNK_TEXTURE, index);

        if (!q.deactivateMessage.empty())
            MessageHandler::Register(q.deactivateMessage, GV_CHUNK_TEXTURE, index);
    }

    void TexturedQuad::HandleMessage(uint32_t index, const std::string& msg)
    {
        if (index >= g_quads.size())
            return;

        TexturedQuadData& q = g_quads[index];

        if (msg == q.activateMessage)
            q.visible = true;

        if (msg == q.deactivateMessage)
            q.visible = false;
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