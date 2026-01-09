#pragma once

#include <fstream>
#include <filesystem>
#include <mutex>

#include "../../utils/Timer.h"

class DistributionMeter
{
public:
    DistributionMeter()
        : _id(allocateId())
    {
        globalMap.reserve(5000);
    }

    void add(const int win)
    {
        std::unordered_map<int, uint64_t>& localMap = tls()[_id];
        ++localMap[win];
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

    void fileOutput(const bool descendingOrder,
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
