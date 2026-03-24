#include "DistributionMeter.h"

#include <ranges>

#include "MeterUpdate.h"

#include <unordered_map>

void DistributionMeter::add(const int key, const uint64_t toAdd)
{
    std::unordered_map<int, uint64_t>& localMap = tls()[_id];
    localMap[key] += toAdd;
}


uint64_t& DistributionMeter::operator[](const int key)
{
    std::unordered_map<int, uint64_t>& localMap = tls()[_id];
    return localMap[key];
}

void DistributionMeter::globalAdd(const int key, const uint64_t toAdd)
{
    globalMap[key] += toAdd;
}

void DistributionMeter::flush()
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

std::vector<std::pair<int, uint64_t>> DistributionMeter::getSortedData(const bool descending) const
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

void DistributionMeter::addReport(
    ReportBuffer& buffer, const std::string& title, const std::string& keyLabel,
    const bool descending) const
{
    const auto& data = getSortedData(descending);

    uint64_t total = 0;
    for (const auto& snd : data | std::views::values)
    {
        total += snd;
    }


    for (const auto& [key, count] : data)
    {
        buffer.addRow(
            title,
            {keyLabel, "HitRate", "%", "Hits"},
            {
                std::to_string(key),
                buffer.toFloat(static_cast<long double>(total) / count, 4),
                buffer.toPercent(static_cast<long double>(count) / total),
                std::to_string(count)
            }
        );
    }
}

std::unordered_map<int, uint64_t> DistributionMeter::getLocalData()
{
    return tls()[_id];
}


std::unordered_map<int, uint64_t>* DistributionMeter::tls()
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

size_t DistributionMeter::allocateId()
{
    static std::atomic<size_t> nextId{0};
    return nextId.fetch_add(1, std::memory_order_relaxed);
}
