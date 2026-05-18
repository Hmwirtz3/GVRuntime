#pragma once

#include <vector>
#include <string>
#include <cstdint>

#ifdef GV_EDITOR
BEGIN_LOGIC_UNIT(Crosshair, GV_CHUNK_CROSSHAIR)

    UI_SEPARATOR("General")

    UI_PARAM_STRING(state, "Default", "CrosshairStateName")
    UI_PARAM_ASSET(texture, "", "Path to texture")

    UI_PARAM_BOOL(defaultCrosshair, true, "This is default crosshair")

    UI_PARAM_FLOAT(width, 32.0f, "width of texture")
    UI_PARAM_FLOAT(height, 32.0f, "Height of texture")

END_LOGIC_UNIT

#endif

namespace GV
{
    struct CrosshairData
    {
        std::string state;

        uint32_t textureID = 0;

        float width = 32.0f;
        float height = 32.0f;

        bool defaultCrosshair = false;
        bool isVisible = true;
    };

    class Crosshair
    {
    public:

        static void Load(const std::vector<uint8_t>& bytes,
                         uint32_t start,
                         uint32_t end);

        static void HandleMessage(uint32_t index,
                                  const std::string& msg,
                                  uint32_t senderType,
                                  uint32_t senderIndex,
                                  const void* payload,
                                  uint32_t payloadSize);

        static uint32_t GetActive();

        static uint32_t GetCount();

        static const CrosshairData& Get(uint32_t index);

        static void BuildRenderData(uint32_t index,
                                    void* dst);
    };
}