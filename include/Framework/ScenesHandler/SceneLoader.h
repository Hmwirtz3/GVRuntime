#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace GV
{

     struct SceneDispatchItem
{
    uint32_t type;
    uint32_t start;
    uint32_t end;
};

    struct SceneChunk
    {
        uint32_t type;
        uint32_t size;
        uint32_t version;
        uint32_t fileOffset;
        uint32_t payloadOffset;
        uint32_t endOffset;
        uint32_t alignedEndOffset;
        int depth;
        int parentIndex;
        std::vector<int> children;
    };

 

    struct SceneFile
    {
        std::vector<uint8_t> bytes;
        std::vector<SceneChunk> chunks;
        std::vector<SceneDispatchItem> dispatchItems;

        void Clear()
        {
            bytes.clear();
            chunks.clear();
            dispatchItems.clear();
        }
    };

    class SceneLoader
    {
    public:
        static bool Load(const std::string& path, SceneFile& outScene);

    private:
        static uint32_t Align16(uint32_t value);

        static bool LoadFileBytes(
            const std::string& path,
            std::vector<uint8_t>& outBytes);

        static bool TraverseRange(
            const std::vector<uint8_t>& bytes,
            uint32_t rangeStart,
            uint32_t rangeEnd,
            int depth,
            int parentIndex,
            std::vector<SceneChunk>& outChunks,
            std::vector<SceneDispatchItem>& dispatchItems);
    };
}