#pragma once
/**
 * @file IntSumMeter.h
 * @brief Thread-local summation meter with lock-free global aggregation on flush.
 *
 * Each thread accumulates a 64-bit sum. When `flush()` is triggered at the end of the batch
 * (via `MeterUpdate` subscription), the local sum is atomically added to the global total
 * and the local slot is reset to zero. Increment-style operators are provided for convenience.
 */
#include <atomic>
#include <cstdint>
#include "MeterUpdate.h"

/**
 * @brief Accumulates integer sums per thread and exposes a global total.
 */
class IntSumMeter
{
public:
    /**
     * @brief Construct and allocate a unique TLS slot id; auto-subscribes to flush.
     */
    IntSumMeter() : _id(allocateId())
    {
    };

    /**
     * @brief Pre-increment by 1.
     */
    IntSumMeter& operator++()
    {
        increment(1);
        return *this;
    }

    /**
     * @brief Post-increment by 1.
     */
    IntSumMeter& operator++(int)
    {
        increment(1);
        return *this;
    }

    /**
     * @brief Add v to the current thread's local sum.
     */
    IntSumMeter& operator+=(const uint64_t v)
    {
        increment(v);
        return *this;
    }

    /**
     * @brief Add to the calling thread's local counter.
     * @details Global value will be updated at the end of the batch.
     */
    void increment(const uint64_t toAdd)
    {
        tls()[_id] += toAdd;
    }

    /**
     * @brief Atomically transfer the calling thread's local sum to the global total and clear local.
     */
    void flush()
    {
        if (tls()[_id] == 0) return;
        global_.fetch_add(tls()[_id], std::memory_order_relaxed);
        tls()[_id] = 0;
    }

    /**
     * @brief Read the aggregated global total.
     */
    uint64_t global() const
    {
        return global_.load(std::memory_order_relaxed);
    }

private:
    /**
     * @brief Access per-thread storage and subscribe this meter to global flush notifications.
     */
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

    /**
     * @brief Allocate a unique slot index for TLS arrays across meters of this type.
     */
    static size_t allocateId()
    {
        static std::atomic<size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }

    std::atomic<uint64_t> global_{0};

    const size_t _id;
};

