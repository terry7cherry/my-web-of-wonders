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
    void update(const uint64_t newValue)
    {
        if (newValue > tls().first[_id])
        {
            tls().first[_id] = newValue;
            tls().second[_id] = 1;
        }
        else if (newValue == tls().first[_id])
            tls().second[_id]++;
    }

    /**
     * @brief Merge the calling thread's (max,count) into the global state.
     */
    void flush()
    {
        uint64_t* localMaxes = tls().first;
        uint64_t* localCounts = tls().second;
        std::lock_guard lock(mutex);
        if (localMaxes[_id] > globalMax)
        {
            globalMax = localMaxes[_id];
            globalCount = localCounts[_id];
        }
        else if (tls().first[_id] == globalMax)
            globalCount += localCounts[_id];
        else
        {
            localMaxes[_id] = globalMax;
        }
        localCounts[_id] = 0;
    }

    uint64_t globalMax{0}; // highest value observed across all threads
    uint64_t globalCount{0}; // number of observations equal to globalMax

private:
    /**
     * @brief Access per-thread arrays for (max,count) and subscribe to flush notifications.
     */
    std::pair<uint64_t*, uint64_t*> tls()
    {
        thread_local uint64_t tlsMax[100] = {};
        thread_local uint64_t tlsCount[100] = {};
        thread_local bool registered[100] = {};
        if (!registered[_id]) [[unlikely]]
        {
            MeterUpdate::subscribe([this]() { this->flush(); });
            registered[_id] = true;
        }
        return {tlsMax, tlsCount};
    }

    /**
     * @brief Allocate a unique slot index for TLS arrays across meters of this type.
     */
    static size_t allocateId()
    {
        static std::atomic<size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }


    const size_t _id;
    std::mutex mutex;
};

