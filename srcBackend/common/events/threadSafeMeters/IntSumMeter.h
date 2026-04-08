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
    IntSumMeter& operator++();

    /**
     * @brief Post-increment by 1.
     */
    IntSumMeter& operator++(int);

    /**
     * @brief Add v to the current thread's local sum.
     */
    IntSumMeter& operator+=(uint64_t v);

    /**
     * @brief Add to the calling thread's local counter.
     * @details Global value will be updated at the end of the batch.
     */
    void increment(const uint64_t toAdd);

    /**
     * @brief Atomically transfer the calling thread's local sum to the global total and clear local.
     */
    void flush();

    /**
     * @brief Read the aggregated global total.
     */
    uint64_t global() const;

    /**
     * @brief Read the stored value (across all threads).
     */
    const uint64_t& getValue() const
    {
        return global();
    }

    /**
     * @brief Read the total on this thread (cleared after each batch/session)
     */
    uint64_t getLocalValue();

private:
    /**
     * @brief Access per-thread storage and subscribe this meter to global flush notifications.
     */
    uint64_t* tls();

    /**
     * @brief Allocate a unique slot index for TLS arrays across meters of this type.
     */
    static size_t allocateId();

    std::atomic<uint64_t> global_{0};

    const size_t _id;
};

