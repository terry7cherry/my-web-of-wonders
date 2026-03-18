#include "IntSumMeter.h"

IntSumMeter& IntSumMeter::operator++()
{
    increment(1);
    return *this;
}

IntSumMeter& IntSumMeter::operator++(int)
{
    increment(1);
    return *this;
}

IntSumMeter& IntSumMeter::operator+=(const uint64_t v)
{
    increment(v);
    return *this;
}

void IntSumMeter::increment(const uint64_t toAdd)
{
    tls()[_id] += toAdd;
}

void IntSumMeter::flush()
{
    if (tls()[_id] == 0) return;
    global_.fetch_add(tls()[_id], std::memory_order_relaxed);
    tls()[_id] = 0;
}

uint64_t IntSumMeter::global() const
{
    return global_.load(std::memory_order_relaxed);
}

uint64_t* IntSumMeter::tls()
{
    thread_local uint64_t tls[100] = {};
    thread_local bool registered[100] = {};
    if (!registered[_id]) [[unlikely]]
    {
        MeterUpdate::subscribe([this]() { this->flush(); });
        registered[_id] = true;
    }
    return tls;
}

size_t IntSumMeter::allocateId()
{
    static std::atomic<size_t> nextId{0};
    return nextId.fetch_add(1, std::memory_order_relaxed);
}
