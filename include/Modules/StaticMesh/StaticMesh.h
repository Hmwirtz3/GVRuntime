#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace GV
{
    #pragma pack(push, 1)
    struct PSPVertex
    {
        int16_t u, v;
        uint32_t color;
        float x, y, z;
    };
    #pragma pack(pop)

    struct StaticSubmesh
    {
        PSPVertex* vertices = nullptr;
        uint32_t vertexCount = 0;
        uint32_t textureID = 0;
    };

    struct StaticMeshInstance
    {
        std::vector<StaticSubmesh> submeshes;

        float posX = 0.0f;
        float posY = 0.0f;
        float posZ = 0.0f;

        float rotX = 0.0f;
        float rotY = 0.0f;
        float rotZ = 0.0f;

        float scaleX = 1.0f;
        float scaleY = 1.0f;
        float scaleZ = 1.0f;

        bool visible = true;

        std::string activateMessage;
    };

    struct MeshGpuData
    {
        uint32_t textureID;
        uint32_t vertexCount;
        PSPVertex* verts;
    };

    class StaticMesh
    {
    public:
        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end
        );

        static void BuildRenderData(
            uint32_t meshIndex,
            uint32_t submeshIndex,
            void* dst
        );

        static void HandleMessage(uint32_t index, const std::string& msg);

        static uint32_t GetCount();
        static const StaticMeshInstance& Get(uint32_t index);
    };
}