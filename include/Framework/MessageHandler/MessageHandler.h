#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace GV
{
    struct MessageTarget
    {
        uint32_t type;
        uint32_t index;
    };

    class MessageHandler
    {
    public:
        static void Register(const std::string& msg, uint32_t receiverType, uint32_t receiverIndex);
        static void Send(const std::string& msg);
        static void Clear();

    private:
        static std::unordered_map<std::string, std::vector<MessageTarget>> s_routes;
    };
}