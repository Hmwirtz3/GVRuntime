#include "Modules/UI/Crosshair.h"
#include "Modules/UI/TexturedQuad.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <vector>
#include <string>
#include <cstring>

namespace GV
{
    static std::vector<CrosshairData> g_crosshairs;
    static uint32_t g_activeCrosshair = 0;

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

    void Crosshair::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base = bytes.data();
        const uint8_t* ptr = base + start;
        const uint8_t* endPtr = base + end;

        if (start >= bytes.size() || end > bytes.size() || ptr >= endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        GV_ChunkHeader header{};

        if (!ReadChunkHeaderSafe(ptr, endPtr, header))
            return;

        if (header.type != GV_CHUNK_CROSSHAIR)
            return;

        const uint8_t* innerStart = ptr;
        const uint8_t* innerEnd = innerStart + header.size;

        if (innerEnd > endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = 0;

        if (!ReadUInt32Safe(ptr, innerEnd, paramCount))
            return;

        if (paramCount < 4)
            return;

        CrosshairData c{};

        if (!ReadStringSafe(ptr, innerEnd, c.state))
            return;

        if (!ReadBoolSafe(ptr, innerEnd, c.defaultCrosshair))
            return;

        if (!ReadFloatSafe(ptr, innerEnd, c.width))
            return;

        if (!ReadFloatSafe(ptr, innerEnd, c.height))
            return;

        ptr = innerEnd;

        Align16(ptr, endPtr);

        if (!ReadUInt32Safe(ptr, endPtr, c.textureID))
            return;

        c.isVisible = false;

        uint32_t index = (uint32_t)g_crosshairs.size();

        g_crosshairs.push_back(c);

        if (c.defaultCrosshair)
        {
            g_activeCrosshair = index;
            g_crosshairs[index].isVisible = true;
        }

        if (!c.state.empty())
        {
            MessageHandler::Register(
                c.state,
                GV_CHUNK_CROSSHAIR,
                index
            );
        }
    }

    void Crosshair::HandleMessage(
        uint32_t index,
        const std::string& msg,
        uint32_t,
        uint32_t,
        const void*,
        uint32_t)
    {
        if (index >= g_crosshairs.size())
            return;

        CrosshairData& c = g_crosshairs[index];

        if (msg == c.state)
        {
            if (g_activeCrosshair < g_crosshairs.size())
                g_crosshairs[g_activeCrosshair].isVisible = false;

            g_activeCrosshair = index;

            g_crosshairs[index].isVisible = true;
        }
    }

    void Crosshair::BuildRenderData(uint32_t index, void* dst)
    {
        if (index >= g_crosshairs.size() || !dst)
            return;

        const CrosshairData& c = g_crosshairs[index];

        QuadGpuData* out = (QuadGpuData*)dst;

        const float w = c.width;
        const float h = c.height;

        const float x0 = (480.0f * 0.5f) - (w * 0.5f);
        const float y0 = (272.0f * 0.5f) - (h * 0.5f);

        const float x1 = x0 + w;
        const float y1 = y0 + h;

        out->verts[0] = { 0.0f, 0.0f, x0, y0, 0.0f };
        out->verts[1] = { w,    0.0f, x1, y0, 0.0f };
        out->verts[2] = { w,    h,    x1, y1, 0.0f };

        out->verts[3] = { 0.0f, 0.0f, x0, y0, 0.0f };
        out->verts[4] = { w,    h,    x1, y1, 0.0f };
        out->verts[5] = { 0.0f, h,    x0, y1, 0.0f };

        out->textureID = c.textureID;
    }

    uint32_t Crosshair::GetActive()
    {
        return g_activeCrosshair;
    }

    uint32_t Crosshair::GetCount()
    {
        return (uint32_t)g_crosshairs.size();
    }

    const CrosshairData& Crosshair::Get(uint32_t index)
    {
        return g_crosshairs[index];
    }
}