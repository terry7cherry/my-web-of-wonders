#pragma once
//#define SIMPLE
//#define SINGLETHREAD

#include <fstream>
#include <filesystem>
#include <numeric>

#include "../../utils/Timer.h"

class DistributionMeter
{
public:
    DistributionMeter()
#ifndef SIMPLE
        : _id(allocateId())
#endif
    {
        globalMap.reserve(5000);
    }

    void add(const int win)
    {
#ifdef SIMPLE
#ifndef SINGLETHREAD
        std::lock_guard<std::mutex> lock(mutex);
#endif
        globalAdd(win);
#else
        std::unordered_map<int, uint64_t>& localMap = tls()[_id];
        ++localMap[win];
#endif
    }

    void globalAdd(const int win, const uint64_t toAdd = 1)
    {
        globalMap[win] += toAdd;
    }

#ifndef SIMPLE
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
#endif

    void fileOutput(const uint64_t totalIncludingZero, const bool descendingOrder,
                    const bool doConsoleProgress)
    {
        std::ofstream outfile;
        const std::filesystem::path filePath = std::filesystem::path(GAME_SOURCE_DIR) / "Distributions" /
            "testDistribution.csv";
        const std::string szFilename = "testDistribution.csv";

        outfile.open(filePath);
        if (!outfile.is_open())
        {
            reportError("Distribution.fileOutput", "Can't open file: " + szFilename + ". Output aborted",
                        "Warning");
            return;
        }

        if (doConsoleProgress)
            std::cout << "Writing Distribution file: " << szFilename << ". . .";

        // Copy unordered_map to a vector for sorting
        std::vector<std::pair<int, uint64_t>> sortedData(globalMap.begin(), globalMap.end());

        const uint64_t totalStored = std::accumulate(sortedData.begin(), sortedData.end(), 0ULL,
                                                     [](auto sum, const auto& pair)
                                                     {
                                                         return sum + pair.second;
                                                     });
        const uint64_t zeroCount = totalIncludingZero - totalStored;
        sortedData.emplace_back(0, zeroCount);

        if (descendingOrder)
        {
            // Sort descending by key (win amount)
            std::ranges::sort(sortedData, [](const auto& a, const auto& b)
            {
                return a.first > b.first;
            });
        }
        else
        {
            // Sort ascending by key (win amount)
            std::ranges::sort(sortedData, [](const auto& a, const auto& b)
            {
                return a.first < b.first;
            });
        }


        for (const auto& [win, count] : sortedData)
        {
            outfile << std::to_string(win) << ", " << std::to_string(count) << std::endl;
        }

        outfile.close();

        if (doConsoleProgress)
            std::cout << "Done\r\n";
    }

private:
    std::mutex mutex;
    std::unordered_map<int, uint64_t> globalMap{};
#ifndef SIMPLE

    std::unordered_map<int, uint64_t>* tls()
    {
        thread_local std::unordered_map<int, uint64_t> tls[10] = {};
        thread_local bool registered[10] = {};
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
#endif
};
