#pragma once

#include <fstream>
#include <filesystem>
#include <mutex>

#include "../../utils/utils.h"

class DistributionMeter
{
public:
    DistributionMeter()
        : _id(allocateId())
    {
        globalMap.reserve(5000);
    }


    void add(const int win, const uint64_t toAdd = 1)
    {
        std::unordered_map<int, uint64_t>& localMap = tls()[_id];
        localMap[win] += toAdd;
    }

    void globalAdd(const int win, const uint64_t toAdd = 1)
    {
        globalMap[win] += toAdd;
    }

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
    std::mutex mutex;
    std::unordered_map<int, uint64_t> globalMap{};

    std::unordered_map<int, uint64_t>* tls()
    {
        thread_local std::unordered_map<int, uint64_t> tls[10] = {};
        thread_local bool registered[100] = {};
        if (!registered[_id]) [[unlikely]]
        {
            registered[_id] = true;
            MeterUpdate::subscribe([this]() { this->flush(); });
            tls[_id].reserve(5000);
        }
        return tls;
    }

    static size_t allocateId()
    {
        static std::atomic<size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }

    const size_t _id;
};
