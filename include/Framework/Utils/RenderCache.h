#pragma once

#include "Framework/Utils/Allocator.h"

#include <stdint.h>

namespace GV
{
    enum RenderCacheType
    {
        RCACHE_NONE = 0,
        RCACHE_TEXTURED_QUAD,
        RCACHE_STATIC_MESH,
        RCACHE_TERRAIN_TILE
    };

    class RenderCache
    {
    public:
        static const uint32_t MAX_ENTRIES = 512;

        struct Entry
        {
            bool valid;
            bool dirty;

            uint32_t type;
            uint32_t ownerIndex;

            void* ptr;
            uint32_t size;

            uint32_t lastUsedFrame;
        };

    public:
        RenderCache();

        void Init(void* memory, uint32_t size);
        void Reset();

        void BeginFrame();

        Entry* Find(uint32_t type, uint32_t ownerIndex);
        Entry* Get(uint32_t type, uint32_t ownerIndex);

        void* Allocate(uint32_t type, uint32_t ownerIndex, uint32_t size, uint32_t alignment = Allocator::DEFAULT_ALIGNMENT);
        void  Free(uint32_t type, uint32_t ownerIndex);

        void  MarkDirty(uint32_t type, uint32_t ownerIndex);
        void  MarkAllDirty(uint32_t type);

        bool  EnsureCapacity(uint32_t size, uint32_t alignment = Allocator::DEFAULT_ALIGNMENT);

        uint32_t GetFrameIndex() const;
        uint32_t GetEntryCount() const;

        const Entry* GetEntries() const;

        uint32_t GetUsedBytes() const;
        uint32_t GetFreeBytes() const;

    private:
        Allocator m_allocator;
        Entry m_entries[MAX_ENTRIES];
        uint32_t m_frameIndex;

    private:
        int32_t FindFreeEntrySlot() const;
        int32_t FindEntryIndex(uint32_t type, uint32_t ownerIndex) const;
        int32_t FindLRUEntryIndex() const;

        void InvalidateEntry(int32_t index);
    };
}