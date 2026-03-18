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
#include <vector>

#include "../../stats/ReportBuffer.h"

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
    void add(int key, uint64_t toAdd = 1);

    /**
     * @brief Get reference to value for key, for used with ++ or += operators.
     * @param key Key to look up.
     * @return
     */
    uint64_t& operator[](int key);

    /**
     * @brief Add directly to the global aggregated map (guarded by external locking).
     */
    void globalAdd(int key, uint64_t toAdd = 1);

    /**
     * @brief Merge the calling thread's local counts into the global map and clear local state.
     */
    void flush();

    /**
     * @brief Get the aggregated distribution as sorted pairs of (key,count).
     * @param descending Sort descending by key when true; ascending when false.
     */
    std::vector<std::pair<int, uint64_t>> getSortedData(bool descending = false) const;

    /**
     * @brief Add a report with hit rates, percent, and hits for each entry.
     * @param buffer Buffer to add report to.
     * @param title Report title.
     * @param keyLabel String used as header for key column.
     * @param descending Set to true to order keys in descending order.
     */
    void addReport(ReportBuffer& buffer, const std::string& title, const std::string& keyLabel,
                   bool descending = false) const;

private:
    std::mutex mutex; // guards globalMap during flush
    std::unordered_map<int, uint64_t> globalMap{}; // aggregated distribution across all threads

    /**
     * @brief Access per-thread array of local maps and auto-subscribe to flush callbacks.
     */
    std::unordered_map<int, uint64_t>* tls();

    /**
     * @brief Allocate a unique slot index for TLS arrays across all meters of this type.
     */
    static size_t allocateId();

    const size_t _id;
};
