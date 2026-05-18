#include "Modules/ZoneTrigger/ZoneTrigger.h"

#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <vector>
#include <string>
#include <cstring>

namespace GV
{
    static std::vector<ZoneTriggerData> g_zoneTriggers;

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

    void ZoneTrigger::Load(
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

        if (header.type != GV_CHUNK_ZONE_TRIGGER)
            return;

        const uint8_t* innerStart = ptr;
        const uint8_t* innerEnd = innerStart + header.size;

        if (innerEnd > endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = 0;

        if (!ReadUInt32Safe(ptr, innerEnd, paramCount))
            return;

        if (paramCount < 2)
            return;

        ZoneTriggerData z{};

        if (!ReadStringSafe(ptr, innerEnd, z.sceneFile))
            return;

        if (!ReadStringSafe(ptr, innerEnd, z.loadMessage))
            return;

        ptr = innerEnd;

        Align16(ptr, endPtr);

        uint32_t index = (uint32_t)g_zoneTriggers.size();

        g_zoneTriggers.push_back(z);

        if (!z.loadMessage.empty())
        {
            MessageHandler::Register(
                z.loadMessage,
                GV_CHUNK_ZONE_TRIGGER,
                index
            );
        }
    }

    void ZoneTrigger::HandleMessage(
        uint32_t index,
        const std::string& msg,
        uint32_t,
        uint32_t,
        const void*,
        uint32_t)
    {
        if (index >= g_zoneTriggers.size())
            return;

        ZoneTriggerData& z = g_zoneTriggers[index];

        if (msg != z.loadMessage)
            return;

        if (z.sceneFile.empty())
            return;

        SceneLoader::Load(
            z.sceneFile.c_str(),
            z.scene
        );
    }
}