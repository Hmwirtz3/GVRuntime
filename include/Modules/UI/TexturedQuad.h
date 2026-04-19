#pragma once

#include <vector>
#include <cstdint>
#include <string>

#ifdef GV_Editor
BEGIN_LOGIC_UNIT(TexturedQuad, GV_CHUNK_TEXTURE)

UI_SEPARATOR("Transform")

UI_PARAM_FLOAT(posX, 0.0, "X position")
UI_PARAM_FLOAT(posY, 0.0, "Y position")

UI_SEPARATOR("Rendering")

UI_PARAM_INT(width, 32, "Width in pixels")
UI_PARAM_INT(height, 32, "Height in pixels")
UI_PARAM_ASSET(texture, "", "Texture file")
UI_PARAM_BOOL(visible, true, "Visibility")

END_LOGIC_UNIT
#endif

namespace GV
{
    struct TexturedQuadData
    {
        float posX;
        float posY;

        int width;
        int height;

        std::string activateMessage;
        

        uint32_t textureID;

        bool visible;
    };

    struct QuadVertex
{
    float u, v;
    float x, y, z;
    //float u, v;
};

struct QuadGpuData
{
    QuadVertex verts[6];
    uint32_t textureID;
};

    class TexturedQuad
    {
    public:
        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end
        );

        static void HandleMessage(uint32_t index, const std::string& msg);
        static void BuildRenderData(uint32_t index, void* dst);
        static uint32_t GetCount();
        static const TexturedQuadData& Get(uint32_t index);
    };
}