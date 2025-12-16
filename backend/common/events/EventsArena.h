#pragma once

#include <cstddef>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <new>

class EventsArena
{
public:
    explicit EventsArena(std::size_t initialBlockSize)
        : initialBlockSize_(initialBlockSize)
    {
        blocks_.push_back(makeBlock(initialBlockSize_));
    }

    ~EventsArena()
    {
        for (Block& b : blocks_)
            std::free(b.begin);
    }

    void* allocate(std::size_t size, std::size_t alignment)
    {
        Block& b = blocks_.back();

        if (void* p = tryAllocate(b, size, alignment))
            return p;

        // Allocate a new block (grow strategy: double, but at least big enough)
        std::size_t newBlockSize =
            std::max<std::size_t>(b.capacity * 2, size + alignment);

        blocks_.push_back(makeBlock(newBlockSize));
        return tryAllocate(blocks_.back(), size, alignment);
    }

    void reset()
    {
        // Keep the first block, free the rest
        for (std::size_t i = 1; i < blocks_.size(); ++i)
            std::free(blocks_[i].begin);

        blocks_.resize(1);
        blocks_[0].ptr = blocks_[0].begin;
    }

private:
    struct Block
    {
        std::byte* begin;
        std::byte* ptr;
        std::byte* end;
        std::size_t capacity;
    };

    static Block makeBlock(std::size_t size)
    {
        std::byte* mem = static_cast<std::byte*>(std::malloc(size));
        assert(mem && "EventsArena allocation failed");

        return Block{
            mem,
            mem,
            mem + size,
            size
        };
    }

    static void* tryAllocate(Block& b, std::size_t size, std::size_t alignment)
    {
        void* p = b.ptr;
        std::size_t space = static_cast<std::size_t>(b.end - b.ptr);

        if (!std::align(alignment, size, p, space))
            return nullptr;

        if (static_cast<std::byte*>(p) + size > b.end)
            return nullptr;

        b.ptr = static_cast<std::byte*>(p) + size;
        return p;
    }

private:
    std::vector<Block> blocks_;
    std::size_t initialBlockSize_;
};
