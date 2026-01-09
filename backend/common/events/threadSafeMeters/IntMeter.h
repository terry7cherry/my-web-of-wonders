#pragma once
#include <atomic>
#include <cstdint>
#include "MeterUpdate.h"

class IntMeter
{
public:
    IntMeter() : _id(allocateId())
    {
    };

    // Pre-increment
    IntMeter& operator++()
    {
        increment(1);
        return *this;
    }

    IntMeter& operator++(int)
    {
        increment(1);
        return *this;
    }

    // Addition assignment
    IntMeter& operator+=(const uint64_t v)
    {
        increment(v);
        return *this;
    }

    void increment(const uint64_t toAdd)
    {
        tls()[_id] += toAdd;
    }

    void flush()
    {
        if (tls()[_id] == 0) return;
        global_.fetch_add(tls()[_id], std::memory_order_relaxed);
        tls()[_id] = 0;
    }

    uint64_t global() const
    {
        return global_.load(std::memory_order_relaxed);
    }

private:
    uint64_t* tls()
    {
        thread_local uint64_t tls[100] = {};
        thread_local bool registered[100] = {};
        if (!registered[_id]) [[unlikely]]
        {
            MeterUpdate::subscribe([this]() { this->flush(); });
            registered[_id] = true;
        }
        return tls;
    }

    static size_t allocateId()
    {
        static std::atomic<size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }

    std::atomic<uint64_t> global_{0};

    const size_t _id;
};

