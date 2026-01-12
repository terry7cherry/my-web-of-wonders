#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>

#include "MeterUpdate.h"

class IntMaxMeter
{
public:
    IntMaxMeter() : _id(allocateId())
    {
    };

    void update(const uint64_t win)
    {
        if (win > tls().first[_id])
        {
            tls().first[_id] = win;
            tls().second[_id] = 1;
        }
        else if (win == tls().first[_id])
            tls().second[_id]++;
    }

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

    uint64_t globalMax{0};
    uint64_t globalCount{0};

private:
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

    static size_t allocateId()
    {
        static std::atomic<size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }


    const size_t _id;
    std::mutex mutex;
};

