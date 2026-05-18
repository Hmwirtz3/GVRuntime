#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace GV
{
    struct AreaTriggerBoxInstance
    {
        float posX = 0.0f;
        float posY = 0.0f;
        float posZ = 0.0f;

        float sizeX = 1.0f;
        float sizeY = 1.0f;
        float sizeZ = 1.0f;

        std::string onEnter;
        std::string onExit;

        bool triggerOnce = false;
        bool visible = true;

        bool isInside = false;
        bool hasTriggered = false;
    };

    class AreaTriggerBox
    {
    public:
        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end);

        static void Update(); // ← FIXED

        static void HandleMessage(uint32_t index,
                          const std::string& msg,
                          uint32_t senderType,
                          uint32_t senderIndex,
                          const void* payload,
                          uint32_t payloadSize);

        static uint32_t GetCount();
        static const AreaTriggerBoxInstance& Get(uint32_t index);
    };
}

#ifdef GV_Editor

BEGIN_LOGIC_UNIT(AreaTriggerBox, GV_CHUNK_AREA_TRIGGER_BOX)

UI_SEPARATOR("Bounds")

UI_PARAM_FLOAT(posX, 0.0f, "Position X")
UI_PARAM_FLOAT(posY, 0.0f, "Position Y")
UI_PARAM_FLOAT(posZ, 0.0f, "Position Z")

UI_PARAM_FLOAT(sizeX, 1.0f, "Size X")
UI_PARAM_FLOAT(sizeY, 1.0f, "Size Y")
UI_PARAM_FLOAT(sizeZ, 1.0f, "Size Z")

UI_SEPARATOR("Behavior")

UI_MESSAGE(onEnter, "", "Fired when something enters")
UI_MESSAGE(onExit, "", "Fired when something exits")

UI_PARAM_BOOL(triggerOnce, false, "Only trigger once")

END_LOGIC_UNIT

#endif