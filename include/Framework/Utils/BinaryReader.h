#pragma once
#include <cstdint>
#include <cstring>

namespace GV
{
    inline float ReadFloat(const uint8_t*& ptr)
    {
        float v;
        std::memcpy(&v, ptr, 4);
        ptr += 4;
        return v;
    }

    inline uint32_t ReadUInt32(const uint8_t*& ptr)
    {
        uint32_t v;
        std::memcpy(&v, ptr, 4);
        ptr += 4;
        return v;
    }

    inline int ReadInt(const uint8_t*& ptr)
    {
        int v;
        std::memcpy(&v, ptr, 4);
        ptr += 4;
        return v;
    }

    inline bool ReadBool(const uint8_t*& ptr)
    {
        uint8_t v = *ptr;
        ptr += 1;
        return v != 0;
    }

    inline const char* ReadString(const uint8_t*& ptr)
    {
        uint32_t len;
        std::memcpy(&len, ptr, 4);
        ptr += 4;

        const char* str = (const char*)ptr;

        ptr += len;

        return str;
    }
}