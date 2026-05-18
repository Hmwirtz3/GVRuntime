#include "Modules/MessageRelay/MessageRelay.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <vector>
#include <string>
#include <cstring>

namespace GV
{
    static std::vector<MessageRelayData> g_messageRelays;

    static void Align16(const uint8_t*& ptr, const uint8_t* endPtr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;

        const uint8_t* aligned = (const uint8_t*)p;

        if (aligned <= endPtr)
            ptr = aligned;
    }

    static bool ReadChunkHeaderSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        GV_ChunkHeader& outHeader)
    {
        if (ptr + sizeof(GV_ChunkHeader) > endPtr)
            return false;

        std::memcpy(&outHeader, ptr, sizeof(GV_ChunkHeader));

        return true;
    }

    static bool ReadUInt32Safe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        uint32_t& outValue)
    {
        if (ptr + sizeof(uint32_t) > endPtr)
            return false;

        std::memcpy(&outValue, ptr, sizeof(uint32_t));

        ptr += sizeof(uint32_t);

        return true;
    }

    static bool ReadStringSafe(
        const uint8_t*& ptr,
        const uint8_t* endPtr,
        std::string& outValue)
    {
        uint32_t len = 0;

        if (!ReadUInt32Safe(ptr, endPtr, len))
            return false;

        if (ptr + len > endPtr + 16)
            return false;

        outValue.assign((const char*)ptr, len);

        ptr += len;

        return true;
    }

    void MessageRelay::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base = bytes.data();
        const uint8_t* ptr = base + start;
        const uint8_t* endPtr = base + end;

        if (start >= bytes.size() || end > bytes.size() || ptr >= endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        GV_ChunkHeader header{};

        if (!ReadChunkHeaderSafe(ptr, endPtr, header))
            return;

        if (header.type != GV_CHUNK_MESSAGE_RELAY)
            return;

        const uint8_t* innerStart = ptr;
        const uint8_t* innerEnd = innerStart + header.size;

        if (innerEnd > endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = 0;

        if (!ReadUInt32Safe(ptr, innerEnd, paramCount))
            return;

        if (paramCount < 6)
            return;

        MessageRelayData r{};

        if (!ReadStringSafe(ptr, innerEnd, r.listenMessage)) return;
        if (!ReadStringSafe(ptr, innerEnd, r.firstMessage)) return;
        if (!ReadStringSafe(ptr, innerEnd, r.secondMessage)) return;
        if (!ReadStringSafe(ptr, innerEnd, r.thirdMessage)) return;
        if (!ReadStringSafe(ptr, innerEnd, r.fourthMessage)) return;
        if (!ReadStringSafe(ptr, innerEnd, r.fifthMessage)) return;

        ptr = innerEnd;

        Align16(ptr, endPtr);

        uint32_t index = (uint32_t)g_messageRelays.size();

        g_messageRelays.push_back(r);

        if (!r.listenMessage.empty())
        {
            MessageHandler::Register(
                r.listenMessage,
                GV_CHUNK_MESSAGE_RELAY,
                index
            );
        }
    }

    void MessageRelay::HandleMessage(
        uint32_t index,
        const std::string& msg,
        uint32_t senderType,
        uint32_t senderIndex,
        const void* payload,
        uint32_t payloadSize)
    {
        if (index >= g_messageRelays.size())
            return;

        MessageRelayData& r = g_messageRelays[index];

        if (msg != r.listenMessage)
            return;

        if (!r.firstMessage.empty())
        {
            MessageHandler::Send(
                r.firstMessage,
                GV_CHUNK_MESSAGE_RELAY,
                index
            );
        }

        if (!r.secondMessage.empty())
        {
            MessageHandler::Send(
                r.secondMessage,
                GV_CHUNK_MESSAGE_RELAY,
                index
            );
        }

        if (!r.thirdMessage.empty())
        {
            MessageHandler::Send(
                r.thirdMessage,
                GV_CHUNK_MESSAGE_RELAY,
                index
            );
        }

        if (!r.fourthMessage.empty())
        {
            MessageHandler::Send(
                r.fourthMessage,
                GV_CHUNK_MESSAGE_RELAY,
                index
            );
        }

        if (!r.fifthMessage.empty())
        {
            MessageHandler::Send(
                r.fifthMessage,
                GV_CHUNK_MESSAGE_RELAY,
                index
            );
        }
    }
}