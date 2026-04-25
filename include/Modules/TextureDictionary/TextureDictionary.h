#pragma once
#include <cstdint>
#include <vector>

namespace GV
{
    

    class TextureDictionary
    {
    public:
        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end);

        static void Bind(uint32_t textureID);
    };
}