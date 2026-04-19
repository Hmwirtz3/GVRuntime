#pragma once

#include <vector>
#include <cstdint>

namespace GV
{
    class StaticMesh
    {
    public:
        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end
        );
    };
}