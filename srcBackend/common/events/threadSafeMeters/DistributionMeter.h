#pragma once
/**
 * @file DistributionMeter.h
 * @brief Thread-safe (per-thread buffered) integer distribution meter with global aggregation.
 *
 * Each thread accumulates counts (or other sums) for integer keys (e.g., win amounts) in a thread-local map.
 * On flush (triggered via `MeterUpdate`), local counts are merged into a global map under a mutex.
 * The aggregated distribution can be exported as a sorted vector of (key,count) pairs.
 */

#include <mutex>

/**
 * @brief Records frequencies for integer values with per-thread buffering and global reduction.
 */
class DistributionMeter
{
public:
    /**
     * @brief Construct a distribution meter and allocate a unique TLS slot id.
     */
    DistributionMeter()
        : _id(allocateId())
    {
        globalMap.reserve(5000);
    }


    /**
     * @brief Add to the local-thread count for a key.
     * @details Global values will be automatically updated at the end of the batch.
     * @param key Integer key (e.g., win amount).
     * @param toAdd Increment to add to the count (defaults to 1).
     */
    void add(const int key, const uint64_t toAdd = 1)
    {
        std::unordered_map<int, uint64_t>& localMap = tls()[_id];
        localMap[key] += toAdd;
    }

    /**
     * @brief Get reference to value for key, for used with ++ or += operators.
     * @param key Key to look up.
     * @return
     */
    uint64_t& operator[](const int key)
    {
        std::unordered_map<int, uint64_t>& localMap = tls()[_id];
        return localMap[key];
    }

    /**
     * @brief Add directly to the global aggregated map (guarded by external locking).
     */
    void globalAdd(const int key, const uint64_t toAdd = 1)
    {
        globalMap[key] += toAdd;
    }

    /**
     * @brief Merge the calling thread's local counts into the global map and clear local state.
     */
    void flush()
    {
        auto& localMap = tls()[_id];
        {
            std::lock_guard lock(mutex);
            for (auto it = localMap.begin(); it != localMap.end(); ++it)
            {
                globalAdd(it->first, it->second);
            }
        }
        localMap.clear();
    }

    /**
     * @brief Get the aggregated distribution as sorted pairs of (key,count).
     * @param descending Sort descending by key when true; ascending when false.
     */
    std::vector<std::pair<int, uint64_t>> getSortedData(const bool descending = false)
    {
        // Copy unordered_map to a vector for sorting
        std::vector<std::pair<int, uint64_t>> sortedData(globalMap.begin(), globalMap.end());
        if (descending)
        {
            // Sort descending by key
            std::ranges::sort(sortedData, [](const auto& a, const auto& b)
            {
                return a.first > b.first;
            });
        }
        else
        {
            // Sort ascending by key
            std::ranges::sort(sortedData, [](const auto& a, const auto& b)
            {
                return a.first < b.first;
            });
        }
        return sortedData;
    }

private:
    std::mutex mutex; // guards globalMap during flush
    std::unordered_map<int, uint64_t> globalMap{}; // aggregated distribution across all threads

    /**
     * @brief Access per-thread array of local maps and auto-subscribe to flush callbacks.
     */
    std::unordered_map<int, uint64_t>* tls()
    {
        thread_local std::unordered_map<int, uint64_t> tls[100] = {};
        thread_local bool registered[100] = {};
        if (!registered[_id]) [[unlikely]]
        {
            registered[_id] = true;
            MeterUpdate::subscribe([this]() { this->flush(); });
            tls[_id].reserve(5000);
        }
        return tls;
    }

    /**
     * @brief Allocate a unique slot index for TLS arrays across all meters of this type.
     */
    static size_t allocateId()
    {
        static std::atomic<size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }

    const size_t _id;
};
