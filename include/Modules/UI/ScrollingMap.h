#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "Modules/UI/TexturedQuad.h"

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(ScrollingMap, GV_CHUNK_SCROLLING_MAP)

    UI_SEPARATOR("General")

    UI_PARAM_FLOAT(posX, 0.0f, "")

    UI_PARAM_FLOAT(posY, 0.0f, "")

    UI_PARAM_INT(width, 480, "")

    UI_PARAM_INT(height, 272, "")

    UI_PARAM_BOOL(visible, true, "")

    UI_SEPARATOR("Scrolling")

    UI_PARAM_FLOAT(scrollSpeedX, 0.0004f, "")

    UI_PARAM_FLOAT(scrollSpeedY, 0.0001f, "")

    UI_PARAM_FLOAT(uvScaleX, 1.0f, "")

    UI_PARAM_FLOAT(uvScaleY, 1.0f, "")

    UI_SEPARATOR("Messages")

    UI_MESSAGE(activateMessage, "", "")

    UI_MESSAGE(deactivateMessage, "", "")

    UI_MESSAGE(toggleMessage, "", "")

    UI_SEPARATOR("Asset")

    UI_PARAM_ASSET(texture, "", "")

END_LOGIC_UNIT

#endif

namespace GV
{
    struct ScrollingMapData
    {
        float posX = 0.0f;
        float posY = 0.0f;

        int width = 480;
        int height = 272;

        bool visible = true;

        float scrollSpeedX = 0.0f;
        float scrollSpeedY = 0.0f;

        float uvScaleX = 1.0f;
        float uvScaleY = 1.0f;

        float currentOffsetX = 0.0f;
        float currentOffsetY = 0.0f;

        std::string activateMessage;
        std::string deactivateMessage;
        std::string toggleMessage;

        uint32_t textureID = 0;
    };

    class ScrollingMap
    {
    public:

        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end
        );

        static void HandleMessage(
            uint32_t index,
            const std::string& msg,
            uint32_t senderType,
            uint32_t senderIndex,
            const void* payload,
            uint32_t payloadSize
        );

        static void Update(float deltaTime);

        static void BuildRenderData(
            uint32_t index,
            void* dst
        );

        static uint32_t GetCount();

        static const ScrollingMapData& Get(uint32_t index);
    };
}