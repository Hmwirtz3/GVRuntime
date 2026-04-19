#pragma once

#include <stdint.h>

namespace GV
{
    class Allocator
    {
    public:
        static const uint32_t DEFAULT_ALIGNMENT = 64;
        static const uint32_t MAX_BLOCKS = 512;

        Allocator();

        void Init(void* memory, uint32_t size);
        void Reset();

        void* Allocate(uint32_t size, uint32_t alignment = DEFAULT_ALIGNMENT);
        void  Free(void* ptr);

        bool Owns(const void* ptr) const;
        bool IsInitialized() const;

        uint32_t GetTotalSize() const;
        uint32_t GetUsedBytes() const;
        uint32_t GetFreeBytes() const;

    private:
        struct Block
        {
            bool valid;
            bool used;

            uint32_t offset;
            uint32_t size;

            int32_t prev;
            int32_t next;
        };

        uint8_t* m_memory;
        uint32_t m_size;
        bool m_initialized;

        Block m_blocks[MAX_BLOCKS];
        int32_t m_head;

    private:
        static uint32_t AlignUp(uint32_t value, uint32_t alignment);

        int32_t FindFreeBlockSlot() const;
        int32_t FindBlockByPointer(const void* ptr) const;

        void InsertBlockAfter(int32_t existingIndex, int32_t newIndex);
        void RemoveBlock(int32_t index);

        void Coalesce(int32_t index);
    };
}