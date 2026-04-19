#include "Framework/Utils/Allocator.h"

namespace GV
{
    Allocator::Allocator()
        : m_memory(0)
        , m_size(0)
        , m_initialized(false)
        , m_head(-1)
    {
        for (uint32_t i = 0; i < MAX_BLOCKS; i++)
        {
            m_blocks[i].valid  = false;
            m_blocks[i].used   = false;
            m_blocks[i].offset = 0;
            m_blocks[i].size   = 0;
            m_blocks[i].prev   = -1;
            m_blocks[i].next   = -1;
        }
    }

    void Allocator::Init(void* memory, uint32_t size)
    {
        m_memory = (uint8_t*)memory;
        m_size = size;
        m_initialized = true;

        Reset();
    }

    void Allocator::Reset()
    {
        for (uint32_t i = 0; i < MAX_BLOCKS; i++)
        {
            m_blocks[i].valid  = false;
            m_blocks[i].used   = false;
            m_blocks[i].offset = 0;
            m_blocks[i].size   = 0;
            m_blocks[i].prev   = -1;
            m_blocks[i].next   = -1;
        }

        if (!m_initialized || m_memory == 0 || m_size == 0)
        {
            m_head = -1;
            return;
        }

        m_head = 0;
        m_blocks[0].valid  = true;
        m_blocks[0].used   = false;
        m_blocks[0].offset = 0;
        m_blocks[0].size   = m_size;
        m_blocks[0].prev   = -1;
        m_blocks[0].next   = -1;
    }

    void* Allocator::Allocate(uint32_t size, uint32_t alignment)
    {
        if (!m_initialized || m_memory == 0 || size == 0)
            return 0;

        if (alignment == 0)
            alignment = DEFAULT_ALIGNMENT;

        int32_t index = m_head;

        while (index != -1)
        {
            Block& block = m_blocks[index];

            if (!block.valid || block.used)
            {
                index = block.next;
                continue;
            }

            const uint32_t blockStart = block.offset;
            const uint32_t blockEnd   = block.offset + block.size;

            const uint32_t alignedStart = AlignUp(blockStart, alignment);

            if (alignedStart < blockStart || alignedStart > blockEnd)
            {
                index = block.next;
                continue;
            }

            const uint32_t padding = alignedStart - blockStart;

            if (padding > block.size)
            {
                index = block.next;
                continue;
            }

            if (size > (block.size - padding))
            {
                index = block.next;
                continue;
            }

            const uint32_t allocEnd = alignedStart + size;
            const uint32_t trailing = blockEnd - allocEnd;

            // Case 1: exact fit, no prefix, no suffix
            if (padding == 0 && trailing == 0)
            {
                block.used = true;
                return m_memory + block.offset;
            }

            // Case 2: prefix free block + used block + optional suffix free block
            if (padding > 0)
            {
                int32_t usedIndex = FindFreeBlockSlot();
                if (usedIndex == -1)
                    return 0;

                m_blocks[usedIndex].valid  = true;
                m_blocks[usedIndex].used   = true;
                m_blocks[usedIndex].offset = alignedStart;
                m_blocks[usedIndex].size   = size;
                m_blocks[usedIndex].prev   = -1;
                m_blocks[usedIndex].next   = -1;

                InsertBlockAfter(index, usedIndex);

                block.size = padding;

                if (trailing > 0)
                {
                    int32_t tailIndex = FindFreeBlockSlot();
                    if (tailIndex == -1)
                    {
                        RemoveBlock(usedIndex);
                        return 0;
                    }

                    m_blocks[tailIndex].valid  = true;
                    m_blocks[tailIndex].used   = false;
                    m_blocks[tailIndex].offset = allocEnd;
                    m_blocks[tailIndex].size   = trailing;
                    m_blocks[tailIndex].prev   = -1;
                    m_blocks[tailIndex].next   = -1;

                    InsertBlockAfter(usedIndex, tailIndex);
                }

                return m_memory + alignedStart;
            }

            // Case 3: no prefix, but suffix remains
            if (padding == 0 && trailing > 0)
            {
                int32_t tailIndex = FindFreeBlockSlot();
                if (tailIndex == -1)
                    return 0;

                m_blocks[tailIndex].valid  = true;
                m_blocks[tailIndex].used   = false;
                m_blocks[tailIndex].offset = allocEnd;
                m_blocks[tailIndex].size   = trailing;
                m_blocks[tailIndex].prev   = -1;
                m_blocks[tailIndex].next   = -1;

                InsertBlockAfter(index, tailIndex);

                block.used = true;
                block.size = size;

                return m_memory + block.offset;
            }

            index = block.next;
        }

        return 0;
    }

    void Allocator::Free(void* ptr)
    {
        if (!m_initialized || ptr == 0)
            return;

        int32_t index = FindBlockByPointer(ptr);
        if (index == -1)
            return;

        Block& block = m_blocks[index];
        if (!block.valid || !block.used)
            return;

        block.used = false;
        Coalesce(index);
    }

    bool Allocator::Owns(const void* ptr) const
    {
        if (!m_initialized || m_memory == 0 || ptr == 0)
            return false;

        const uint8_t* p = (const uint8_t*)ptr;
        return (p >= m_memory) && (p < (m_memory + m_size));
    }

    bool Allocator::IsInitialized() const
    {
        return m_initialized;
    }

    uint32_t Allocator::GetTotalSize() const
    {
        return m_size;
    }

    uint32_t Allocator::GetUsedBytes() const
    {
        uint32_t used = 0;

        int32_t index = m_head;
        while (index != -1)
        {
            const Block& block = m_blocks[index];
            if (block.valid && block.used)
                used += block.size;

            index = block.next;
        }

        return used;
    }

    uint32_t Allocator::GetFreeBytes() const
    {
        uint32_t freeBytes = 0;

        int32_t index = m_head;
        while (index != -1)
        {
            const Block& block = m_blocks[index];
            if (block.valid && !block.used)
                freeBytes += block.size;

            index = block.next;
        }

        return freeBytes;
    }

    uint32_t Allocator::AlignUp(uint32_t value, uint32_t alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    int32_t Allocator::FindFreeBlockSlot() const
    {
        for (uint32_t i = 0; i < MAX_BLOCKS; i++)
        {
            if (!m_blocks[i].valid)
                return (int32_t)i;
        }

        return -1;
    }

    int32_t Allocator::FindBlockByPointer(const void* ptr) const
    {
        if (!Owns(ptr))
            return -1;

        const uint32_t offset = (uint32_t)((const uint8_t*)ptr - m_memory);

        int32_t index = m_head;
        while (index != -1)
        {
            const Block& block = m_blocks[index];
            if (block.valid && block.used && block.offset == offset)
                return index;

            index = block.next;
        }

        return -1;
    }

    void Allocator::InsertBlockAfter(int32_t existingIndex, int32_t newIndex)
    {
        if (existingIndex < 0 || newIndex < 0)
            return;

        Block& existing = m_blocks[existingIndex];
        Block& added    = m_blocks[newIndex];

        added.prev = existingIndex;
        added.next = existing.next;

        if (existing.next != -1)
            m_blocks[existing.next].prev = newIndex;

        existing.next = newIndex;
    }

    void Allocator::RemoveBlock(int32_t index)
    {
        if (index < 0)
            return;

        Block& block = m_blocks[index];

        if (!block.valid)
            return;

        if (block.prev != -1)
            m_blocks[block.prev].next = block.next;
        else
            m_head = block.next;

        if (block.next != -1)
            m_blocks[block.next].prev = block.prev;

        block.valid  = false;
        block.used   = false;
        block.offset = 0;
        block.size   = 0;
        block.prev   = -1;
        block.next   = -1;
    }

    void Allocator::Coalesce(int32_t index)
    {
        if (index < 0 || index >= (int32_t)MAX_BLOCKS)
            return;

        if (!m_blocks[index].valid || m_blocks[index].used)
            return;

        // Merge with previous free block if adjacent
        int32_t prevIndex = m_blocks[index].prev;
        if (prevIndex != -1)
        {
            Block& prev = m_blocks[prevIndex];
            Block& cur  = m_blocks[index];

            if (prev.valid && !prev.used &&
                (prev.offset + prev.size == cur.offset))
            {
                prev.size += cur.size;
                RemoveBlock(index);
                index = prevIndex;
            }
        }

        // Merge with next free block if adjacent
        int32_t nextIndex = m_blocks[index].next;
        if (nextIndex != -1)
        {
            Block& cur  = m_blocks[index];
            Block& next = m_blocks[nextIndex];

            if (next.valid && !next.used &&
                (cur.offset + cur.size == next.offset))
            {
                cur.size += next.size;
                RemoveBlock(nextIndex);
            }
        }
    }
}