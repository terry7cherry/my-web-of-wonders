#include "IntMaxMeter.h"

void IntMaxMeter::update(const uint64_t newValue)
{
    if (newValue > tls().first[_id])
    {
        tls().first[_id] = newValue;
        tls().second[_id] = 1;
    }
    else if (newValue == tls().first[_id])
    {
        tls().second[_id]++;
    }
}

void IntMaxMeter::flush()
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
    {
        globalCount += localCounts[_id];
    }
    else
    {
        localMaxes[_id] = globalMax;
    }
    localCounts[_id] = 0;
}

uint64_t IntMaxMeter::getLocalMax()
{
    return tls().first[_id];
}

uint64_t IntMaxMeter::getLocalCount()
{
    return tls().second[_id];
}

std::pair<uint64_t*, uint64_t*> IntMaxMeter::tls()
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

size_t IntMaxMeter::allocateId()
{
    static std::atomic<size_t> nextId{0};
    return nextId.fetch_add(1, std::memory_order_relaxed);
}
