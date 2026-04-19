#include "Framework/ScenesHandler/SceneLoader.h"
#include "Framework/LogicUnitHandler/LogicUnitDispatch.h"
#include "Framework/Chunk/ChunkTypes.h"

#include <cstdio>
#include <pspiofilemgr.h>
#include <stdarg.h>

namespace GV
{
    static SceUID gLogFd = -1;

    static void WriteLog(const char* fmt, ...)
    {
        if (gLogFd < 0)
            return;

        char buf[256];
        buf[0] = '\0';

        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if (len > 0 && len < (int)sizeof(buf))
        {
            sceIoWrite(gLogFd, buf, len);
        }
    }

    uint32_t SceneLoader::Align16(uint32_t value)
    {
        return (value + 15u) & ~15u;
    }

    bool SceneLoader::LoadFileBytes(const std::string& path, std::vector<uint8_t>& outBytes)
    {
        FILE* file = std::fopen(path.c_str(), "rb");
        if (!file)
            return false;

        std::fseek(file, 0, SEEK_END);
        long fileSize = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);

        outBytes.resize((size_t)fileSize);
        std::fread(outBytes.data(), 1, (size_t)fileSize, file);
        std::fclose(file);

        WriteLog("Loaded: %ld bytes\n", fileSize);

        return true;
    }

    bool SceneLoader::TraverseRange(
        const std::vector<uint8_t>& bytes,
        uint32_t rangeStart,
        uint32_t rangeEnd,
        int depth,
        int parentIndex,
        std::vector<SceneChunk>& outChunks,
        std::vector<SceneDispatchItem>& dispatchItems)
    {
        uint32_t cursor = rangeStart;
        uint32_t fileSize = (uint32_t)bytes.size();

        while (cursor < rangeEnd)
        {
            if (cursor + sizeof(GV_ChunkHeader) > fileSize)
                return false;

            const GV_ChunkHeader* header =
                (const GV_ChunkHeader*)&bytes[cursor];

            uint32_t chunkStart = cursor;
            uint32_t payloadStart = chunkStart + sizeof(GV_ChunkHeader) + 4;
            uint32_t chunkEnd = chunkStart + header->size;
            uint32_t alignedEnd = Align16(chunkEnd);

            WriteLog("Chunk type=0x%08X start=%u end=%u\n",
                header->type, chunkStart, chunkEnd);

            if (header->type == 0x0015)
            {
                uint32_t inner = payloadStart;

                while (inner < chunkEnd)
                {
                    const GV_ChunkHeader* h =
                        (const GV_ChunkHeader*)&bytes[inner];

                    uint32_t innerStart = inner;
                    uint32_t innerPayload = innerStart + sizeof(GV_ChunkHeader) + 4;
                    uint32_t innerEnd = innerStart + h->size;
                    uint32_t innerAligned = Align16(innerEnd);

                    WriteLog("  InsideScene type=0x%08X start=%u end=%u\n",
                        h->type, innerStart, innerEnd);

                    if (h->type == 0x0016)
                    {
                        WriteLog("  TextureDictionary start=%u end=%u\n",
                            innerStart, innerEnd);

                        SceneDispatchItem item;
                        item.type = 0x0016;
                        item.start = innerStart;
                        item.end = innerEnd;
                        dispatchItems.push_back(item);
                    }

                    if (h->type == 0x0024)
                    {
                        if (innerPayload + sizeof(GV_ChunkHeader) <= innerEnd)
                        {
                            const GV_ChunkHeader* child =
                                (const GV_ChunkHeader*)&bytes[innerPayload];

                            WriteLog("  SceneObject first type=0x%08X\n",
                                child->type);

                            SceneDispatchItem item;
                            item.type = child->type;
                            item.start = innerStart;
                            item.end = innerEnd;
                            dispatchItems.push_back(item);
                        }
                    }

                    inner = innerAligned;
                }
            }

            cursor = alignedEnd;

            if (cursor > rangeEnd)
                return false;
        }

        return true;
    }

    bool SceneLoader::Load(const std::string& path, SceneFile& outScene)
    {
        gLogFd = sceIoOpen("log.txt",
            PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

        WriteLog("=== LOAD START ===\n");

        outScene.Clear();

        if (!LoadFileBytes(path, outScene.bytes))
            return false;

        if (!TraverseRange(
                outScene.bytes,
                0,
                (uint32_t)outScene.bytes.size(),
                0,
                -1,
                outScene.chunks,
                outScene.dispatchItems))
        {
            WriteLog("Traverse failed\n");
            return false;
        }

        WriteLog("=== DISPATCH START ===\n");

        LogicUnitDispatch::Dispatch(
            outScene.bytes,
            outScene.dispatchItems);

        WriteLog("=== DISPATCH END ===\n");

        WriteLog("=== LOAD END ===\n");

        if (gLogFd >= 0)
            sceIoClose(gLogFd);

        gLogFd = -1;

        return true;
    }
}