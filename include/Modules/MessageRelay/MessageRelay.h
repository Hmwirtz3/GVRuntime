#pragma once

#include <vector>
#include <string>
#include <cstdint>

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(MessageRelay, GV_CHUNK_MESSAGE_RELAY)

    UI_SEPARATOR("Message Listener")

    UI_PARAM_STRING(messageToListen, "", "Message relay listens for")

    UI_SEPARATOR("Messages To Send")

    UI_PARAM_STRING(firstMessageToSend, "", "First message")
    UI_PARAM_STRING(secondMessageToSend, "", "Second message")
    UI_PARAM_STRING(thirdMessageToSend, "", "Third message")
    UI_PARAM_STRING(fourthMessageToSend, "", "Fourth message")
    UI_PARAM_STRING(fifthMessageToSend, "", "Fifth message")

END_LOGIC_UNIT

#endif

namespace GV
{
    struct MessageRelayData
    {
        std::string listenMessage;

        std::string firstMessage;
        std::string secondMessage;
        std::string thirdMessage;
        std::string fourthMessage;
        std::string fifthMessage;
    };

    class MessageRelay
    {
    public:

        static void Load(
            const std::vector<uint8_t>& bytes,
            uint32_t start,
            uint32_t end);

        static void HandleMessage(
            uint32_t index,
            const std::string& msg,
            uint32_t senderType,
            uint32_t senderIndex,
            const void* payload,
            uint32_t payloadSize);
    };
}