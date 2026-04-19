

#pragma once

#include <vector>
#include <cstdint>

namespace GV
{
    struct SceneDispatchItem;


    class LogicUnitDispatch
    {
    public:
        static void Dispatch(
            const std::vector<uint8_t>& bytes,
            const std::vector<SceneDispatchItem>& items
        );
    };
}