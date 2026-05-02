#include "Modules/AreaTriggerBox/AreaTriggerBox.h"

#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Modules/Camera/CameraSystem.h"

#include <vector>
#include <string>
#include <cstring>

static uint32_t g_triggerCount = 0;

namespace GV
{
    static std::vector<AreaTriggerBoxInstance> g_triggers;

    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    static void ReadMessageString(const uint8_t*& ptr, std::string& out)
    {
        uint32_t len = ReadUInt32(ptr);
        out.assign((const char*)ptr, len);
        ptr += len;
    }

    void AreaTriggerBox::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* ptr = bytes.data() + start;

        ptr += sizeof(GV_ChunkHeader) + 4;
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = ReadUInt32(ptr);

        if (paramCount < 9)
            return;

        AreaTriggerBoxInstance b{};
        b.visible = true;
        b.isInside = false;
        b.hasTriggered = false;

        b.posX = ReadFloat(ptr);
        b.posY = ReadFloat(ptr);
        b.posZ = ReadFloat(ptr);

        b.sizeX = ReadFloat(ptr);
        b.sizeY = ReadFloat(ptr);
        b.sizeZ = ReadFloat(ptr);

        ReadMessageString(ptr, b.onEnter);
        ReadMessageString(ptr, b.onExit);

        b.triggerOnce = ReadBool(ptr);

        Align16(ptr);

        uint32_t index = g_triggerCount++;

        if (g_triggers.size() <= index)
            g_triggers.resize(index + 1);

        g_triggers[index] = b;
    }

    void AreaTriggerBox::Update()
    {
        const Camera& cam = CameraSystem::GetActiveCameraConst();

        const float px = cam.posX;
        const float py = cam.posY;
        const float pz = cam.posZ;

        for (uint32_t i = 0; i < g_triggers.size(); ++i)
        {
            AreaTriggerBoxInstance& b = g_triggers[i];

            if (!b.visible)
                continue;

            float hx = b.sizeX * 0.5f;
            float hy = b.sizeY * 0.5f;
            float hz = b.sizeZ * 0.5f;

            bool insideNow =
                px >= (b.posX - hx) && px <= (b.posX + hx) &&
                py >= (b.posY - hy) && py <= (b.posY + hy) &&
                pz >= (b.posZ - hz) && pz <= (b.posZ + hz);

            if (insideNow)
            {
                if (!b.isInside)
                {
                    if (!b.triggerOnce || !b.hasTriggered)
                    {
                        if (!b.onEnter.empty())
                            MessageHandler::Send(b.onEnter);

                        if (b.triggerOnce)
                            b.hasTriggered = true;
                    }
                }

                b.isInside = true;
            }
            else
            {
                if (b.isInside)
                {
                    if (!b.onExit.empty())
                        MessageHandler::Send(b.onExit);
                }

                b.isInside = false;

                if (!b.triggerOnce)
                    b.hasTriggered = false;
            }
        }
    }

    void AreaTriggerBox::HandleMessage(uint32_t index, const std::string& msg)
    {
        if (index >= g_triggers.size())
            return;

        AreaTriggerBoxInstance& b = g_triggers[index];

        if (msg == b.onEnter)
            b.visible = true;

        if (msg == b.onExit)
            b.visible = false;
    }

    uint32_t AreaTriggerBox::GetCount()
    {
        return (uint32_t)g_triggers.size();
    }

    const AreaTriggerBoxInstance& AreaTriggerBox::Get(uint32_t index)
    {
        return g_triggers[index];
    }
}