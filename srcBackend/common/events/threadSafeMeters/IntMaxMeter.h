#pragma once
/**
 * @file IntMaxMeter.h
 * @brief Thread-safe meter tracking the global maximum value and its frequency.
 *
 * Each thread keeps a local (max,count) pair. On `flush()` (via `MeterUpdate`), the local
 * maximum and its occurrence count are merged into the global maximum and a global count of
 * how many times that maximum has occurred.
 */
#include <atomic>
#include <cstdint>
#include <mutex>

#include "MeterUpdate.h"

/**
 * @brief Tracks the maximum observed integer and how often it occurred across threads.
 */
class IntMaxMeter
{
public:
    /**
     * @brief Construct and allocate a unique TLS slot id; auto-subscribes to flush.
     */
    IntMaxMeter() : _id(allocateId())
    {
    };

    /**
     * @brief Update local thread's (max,count) with a new value.
     * @details Will be merged with global values automatically at the end of the batch.
     * @param newValue Value to consider for maximum.
     */
    void update(uint64_t newValue);

    /**
     * @brief Merge the calling thread's (max,count) into the global state.
     */
    void flush();

    uint64_t globalMax{0}; // highest value observed across all threads
    uint64_t globalCount{0}; // number of observations equal to globalMax

    /**
     * @brief Read the local max.
     */
    uint64_t getLocalMax();

    /**
     * @brief Read the local count.
     */
    uint64_t getLocalCount();

private:
    /**
     * @brief Access per-thread arrays for (max,count) and subscribe to flush notifications.
     */
    std::pair<uint64_t*, uint64_t*> tls();

    /**
     * @brief Allocate a unique slot index for TLS arrays across meters of this type.
     */
    static size_t allocateId();


    const size_t _id;
    std::mutex mutex;
};

