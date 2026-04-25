#pragma once

#include "Framework/Utils/Allocator.h"
#include <cstdint>

namespace GV
{
    enum
    {
        RCACHE_NONE = 0,
        RCACHE_TEXTURED_QUAD,
        RCACHE_STATIC_MESH
    };

    class RenderCache
    {
    public:
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

        static const uint32_t MAX_ENTRIES = 1024;

        RenderCache();

        void Init(void* memory, uint32_t size);
        void Reset();
        void BeginFrame();

        Entry* Find(uint32_t type, uint32_t ownerIndex);
        Entry* Get(uint32_t type, uint32_t ownerIndex);

        void* Allocate(uint32_t type, uint32_t ownerIndex, uint32_t size, uint32_t alignment);
        void Free(uint32_t type, uint32_t ownerIndex);

        void MarkDirty(uint32_t type, uint32_t ownerIndex);
        void MarkAllDirty(uint32_t type);
        void InvalidateAll(uint32_t type);

        bool EnsureCapacity(uint32_t size, uint32_t alignment);

        uint32_t GetFrameIndex() const;
        uint32_t GetEntryCount() const;
        const Entry* GetEntries() const;

        uint32_t GetUsedBytes() const;
        uint32_t GetFreeBytes() const;

    private:
        int32_t FindFreeEntrySlot() const;
        int32_t FindEntryIndex(uint32_t type, uint32_t ownerIndex) const;
        int32_t FindLRUEntryIndex() const;
        void InvalidateEntry(int32_t index);

    private:
        Allocator m_allocator;
        uint32_t m_frameIndex;
        Entry m_entries[MAX_ENTRIES];
    };
}