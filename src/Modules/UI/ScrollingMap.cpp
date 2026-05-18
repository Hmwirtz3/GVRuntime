#include "Modules/UI/ScrollingMap.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <cstring>
#include <cmath>

namespace GV
{
    static std::vector<ScrollingMapData> g_maps;

    static void Align16(const uint8_t*& ptr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;
        ptr = (const uint8_t*)p;
    }

    static std::string ReadStringValue(const uint8_t*& ptr)
    {
        uint32_t len = ReadUInt32(ptr);

        std::string out;

        if (len > 0)
            out.assign((const char*)ptr, len);

        ptr += len;

        return out;
    }

    static bool StringsMatch(const std::string& a, const std::string& b)
    {
        return std::strcmp(a.c_str(), b.c_str()) == 0;
    }

    void ScrollingMap::Load(const std::vector<uint8_t>& bytes, uint32_t start, uint32_t end)
    {
        const uint8_t* base = bytes.data();
        const uint8_t* ptr = base + start;

        if (ptr >= base + end)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = ReadUInt32(ptr);

        if (paramCount < 12)
            return;

        ScrollingMapData m{};

        m.posX = ReadFloat(ptr);
        m.posY = ReadFloat(ptr);

        m.width = ReadInt(ptr);
        m.height = ReadInt(ptr);

        m.visible = ReadBool(ptr);

        m.scrollSpeedX = ReadFloat(ptr);
        m.scrollSpeedY = ReadFloat(ptr);

        m.uvScaleX = ReadFloat(ptr);
        m.uvScaleY = ReadFloat(ptr);

        m.activateMessage = ReadStringValue(ptr);
        m.deactivateMessage = ReadStringValue(ptr);
        m.toggleMessage = ReadStringValue(ptr);

        Align16(ptr);

        m.textureID = ReadUInt32(ptr);

        m.currentOffsetX = 0.0f;
        m.currentOffsetY = 0.0f;

        uint32_t index = (uint32_t)g_maps.size();

        g_maps.push_back(m);

        auto reg = [&](const std::string& s)
        {
            if (!s.empty())
                MessageHandler::Register(s, GV_CHUNK_SCROLLING_MAP, index);
        };

        reg(m.activateMessage);
        reg(m.deactivateMessage);
        reg(m.toggleMessage);
    }

    void ScrollingMap::HandleMessage(uint32_t index, const std::string& msg, uint32_t, uint32_t, const void*, uint32_t)
    {
        if (index >= g_maps.size())
            return;

        ScrollingMapData& m = g_maps[index];

        if (StringsMatch(msg, m.activateMessage))
            m.visible = true;

        if (StringsMatch(msg, m.deactivateMessage))
            m.visible = false;

        if (StringsMatch(msg, m.toggleMessage))
            m.visible = !m.visible;
    }

    void ScrollingMap::Update(float deltaTime)
    {
        for (uint32_t i = 0; i < g_maps.size(); i++)
        {
            ScrollingMapData& m = g_maps[i];

            if (!m.visible)
                continue;

            const float texW = 2048.0f;
            const float texH = 2048.0f;

            float sampleW = (float)m.width * m.uvScaleX;
            float sampleH = (float)m.height * m.uvScaleY;

            float maxOffsetX = texW - sampleW;
            float maxOffsetY = texH - sampleH;

            if (maxOffsetX < 0.0f)
                maxOffsetX = 0.0f;

            if (maxOffsetY < 0.0f)
                maxOffsetY = 0.0f;

            m.currentOffsetX += m.scrollSpeedX * deltaTime;
            m.currentOffsetY += m.scrollSpeedY * deltaTime;

            if (maxOffsetX > 0.0f)
            {
                m.currentOffsetX = std::fmod(m.currentOffsetX, maxOffsetX);

                if (m.currentOffsetX < 0.0f)
                    m.currentOffsetX += maxOffsetX;
            }
            else
            {
                m.currentOffsetX = 0.0f;
            }

            if (maxOffsetY > 0.0f)
            {
                m.currentOffsetY = std::fmod(m.currentOffsetY, maxOffsetY);

                if (m.currentOffsetY < 0.0f)
                    m.currentOffsetY += maxOffsetY;
            }
            else
            {
                m.currentOffsetY = 0.0f;
            }
        }
    }

    void ScrollingMap::BuildRenderData(uint32_t index, void* dst)
    {
        if (index >= g_maps.size() || !dst)
            return;

        const ScrollingMapData& m = g_maps[index];

        QuadGpuData* out = (QuadGpuData*)dst;

        float x0 = m.posX;
        float y0 = m.posY;

        float x1 = x0 + (float)m.width;
        float y1 = y0 + (float)m.height;

        float sampleW = (float)m.width * m.uvScaleX;
        float sampleH = (float)m.height * m.uvScaleY;

        float u0 = m.currentOffsetX;
        float v0 = m.currentOffsetY;

        float u1 = u0 + sampleW;
        float v1 = v0 + sampleH;

        out->verts[0] = { u0, v0, x0, y0, 0.0f };
        out->verts[1] = { u1, v0, x1, y0, 0.0f };
        out->verts[2] = { u1, v1, x1, y1, 0.0f };

        out->verts[3] = { u0, v0, x0, y0, 0.0f };
        out->verts[4] = { u1, v1, x1, y1, 0.0f };
        out->verts[5] = { u0, v1, x0, y1, 0.0f };

        out->textureID = m.textureID;
    }

    uint32_t ScrollingMap::GetCount()
    {
        return (uint32_t)g_maps.size();
    }

    const ScrollingMapData& ScrollingMap::Get(uint32_t index)
    {
        return g_maps[index];
    }
}