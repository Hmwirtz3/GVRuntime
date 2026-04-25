#include "Framework/Utils/RenderCache.h"

namespace GV
{
    RenderCache::RenderCache()
        : m_frameIndex(0)
    {
        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            m_entries[i].valid = false;
            m_entries[i].dirty = false;
            m_entries[i].type = RCACHE_NONE;
            m_entries[i].ownerIndex = 0;
            m_entries[i].ptr = 0;
            m_entries[i].size = 0;
            m_entries[i].lastUsedFrame = 0;
        }
    }

    void RenderCache::Init(void* memory, uint32_t size)
    {
        m_allocator.Init(memory, size);
        Reset();
    }

    void RenderCache::Reset()
    {
        m_allocator.Reset();
        m_frameIndex = 0;

        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            m_entries[i].valid = false;
            m_entries[i].dirty = false;
            m_entries[i].type = RCACHE_NONE;
            m_entries[i].ownerIndex = 0;
            m_entries[i].ptr = 0;
            m_entries[i].size = 0;
            m_entries[i].lastUsedFrame = 0;
        }
    }

    void RenderCache::BeginFrame()
    {
        m_frameIndex++;
    }

    RenderCache::Entry* RenderCache::Find(uint32_t type, uint32_t ownerIndex)
    {
        int32_t index = FindEntryIndex(type, ownerIndex);
        if (index < 0)
            return 0;

        return &m_entries[index];
    }

    RenderCache::Entry* RenderCache::Get(uint32_t type, uint32_t ownerIndex)
    {
        int32_t index = FindEntryIndex(type, ownerIndex);
        if (index < 0)
            return 0;

        m_entries[index].lastUsedFrame = m_frameIndex;
        return &m_entries[index];
    }

    void* RenderCache::Allocate(uint32_t type, uint32_t ownerIndex, uint32_t size, uint32_t alignment)
    {
        if (size == 0)
            return 0;

        int32_t existingIndex = FindEntryIndex(type, ownerIndex);
        if (existingIndex >= 0)
        {
            Entry& existing = m_entries[existingIndex];

            if (existing.valid &&
                existing.ptr &&
                existing.size >= size)
            {
                existing.dirty = true;
                existing.lastUsedFrame = m_frameIndex;
                existing.size = size;
                return existing.ptr;
            }

            InvalidateEntry(existingIndex);
        }

        if (!EnsureCapacity(size, alignment))
            return 0;

        void* ptr = m_allocator.Allocate(size, alignment);
        if (!ptr)
            return 0;

        int32_t slot = FindFreeEntrySlot();
        if (slot < 0)
        {
            m_allocator.Free(ptr);
            return 0;
        }

        Entry& e = m_entries[slot];
        e.valid = true;
        e.dirty = true;
        e.type = type;
        e.ownerIndex = ownerIndex;
        e.ptr = ptr;
        e.size = size;
        e.lastUsedFrame = m_frameIndex;

        return ptr;
    }

    void RenderCache::Free(uint32_t type, uint32_t ownerIndex)
    {
        int32_t index = FindEntryIndex(type, ownerIndex);
        if (index < 0)
            return;

        InvalidateEntry(index);
    }

    void RenderCache::MarkDirty(uint32_t type, uint32_t ownerIndex)
    {
        int32_t index = FindEntryIndex(type, ownerIndex);
        if (index < 0)
            return;

        m_entries[index].dirty = true;
    }

    void RenderCache::MarkAllDirty(uint32_t type)
    {
        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            if (!m_entries[i].valid)
                continue;

            if (m_entries[i].type == type)
                m_entries[i].dirty = true;
        }
    }

    void RenderCache::InvalidateAll(uint32_t type)
    {
        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            if (!m_entries[i].valid)
                continue;

            if (m_entries[i].type == type)
                InvalidateEntry(i);
        }
    }

    bool RenderCache::EnsureCapacity(uint32_t size, uint32_t alignment)
    {
        auto CanFitNow = [&]() -> bool
        {
            if (m_allocator.GetFreeBytes() < size)
                return false;

            void* test = m_allocator.Allocate(size, alignment);
            if (!test)
                return false;

            m_allocator.Free(test);
            return true;
        };

        if (CanFitNow())
            return true;

        while (true)
        {
            int32_t lru = FindLRUEntryIndex();
            if (lru < 0)
                return false;

            InvalidateEntry(lru);

            if (CanFitNow())
                return true;
        }
    }

    uint32_t RenderCache::GetFrameIndex() const
    {
        return m_frameIndex;
    }

    uint32_t RenderCache::GetEntryCount() const
    {
        uint32_t count = 0;

        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            if (m_entries[i].valid)
                count++;
        }

        return count;
    }

    const RenderCache::Entry* RenderCache::GetEntries() const
    {
        return m_entries;
    }

    uint32_t RenderCache::GetUsedBytes() const
    {
        return m_allocator.GetUsedBytes();
    }

    uint32_t RenderCache::GetFreeBytes() const
    {
        return m_allocator.GetFreeBytes();
    }

    int32_t RenderCache::FindFreeEntrySlot() const
    {
        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            if (!m_entries[i].valid)
                return (int32_t)i;
        }

        return -1;
    }

    int32_t RenderCache::FindEntryIndex(uint32_t type, uint32_t ownerIndex) const
    {
        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            const Entry& e = m_entries[i];

            if (!e.valid)
                continue;

            if (e.type == type && e.ownerIndex == ownerIndex)
                return (int32_t)i;
        }

        return -1;
    }

    int32_t RenderCache::FindLRUEntryIndex() const
    {
        int32_t found = -1;
        uint32_t oldestFrame = 0xFFFFFFFF;

        for (uint32_t i = 0; i < MAX_ENTRIES; i++)
        {
            const Entry& e = m_entries[i];

            if (!e.valid)
                continue;

            if (e.lastUsedFrame < oldestFrame)
            {
                oldestFrame = e.lastUsedFrame;
                found = (int32_t)i;
            }
        }

        return found;
    }

    void RenderCache::InvalidateEntry(int32_t index)
    {
        if (index < 0 || index >= (int32_t)MAX_ENTRIES)
            return;

        Entry& e = m_entries[index];
        if (!e.valid)
            return;

        if (e.ptr)
            m_allocator.Free(e.ptr);

        e.valid = false;
        e.dirty = false;
        e.type = RCACHE_NONE;
        e.ownerIndex = 0;
        e.ptr = 0;
        e.size = 0;
        e.lastUsedFrame = 0;
    }
}