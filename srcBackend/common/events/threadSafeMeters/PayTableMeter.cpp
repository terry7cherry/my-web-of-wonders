#include "PayTableMeter.h"
#include <stdexcept>
#include <string>

void PayTableMeter::add(const Symbol symbol, const size_t oak, const std::uint64_t toAdd)
{
    auto& local = tls()[_id];
    checkWithinBounds(symbol, oak);
    // oak is 1-based externally; store at index oak-1
    local[symbol][oak - 1] += toAdd;
}

std::uint64_t* PayTableMeter::operator[](const Symbol symbol)
{
    auto& local = tls()[_id];
    checkWithinBounds(symbol);
    return local[symbol];
}

void PayTableMeter::flush()
{
    auto& local = tls()[_id];
    std::lock_guard lock(mutex);
    for (std::size_t a = 0; a < MAX_SYMBOLS; ++a)
    {
        for (std::size_t b = 0; b < MAX_OAK; ++b)
        {
            if (const auto val = local[a][b])
            {
                global[a][b] += val;
                local[a][b] = 0;
            }
        }
    }
}

const PayTableMeter::LocalArray& PayTableMeter::getLocalArray()
{
    return tls()[_id];
}


PayTableMeter::LocalArray* PayTableMeter::tls()
{
    thread_local LocalArray tlsStorage[100]{}; // zero-initialized
    thread_local bool registered[100]{};
    if (!registered[_id]) [[unlikely]]
    {
        registered[_id] = true;
        MeterUpdate::subscribe([this]() { this->flush(); });
    }
    return tlsStorage;
}

std::size_t PayTableMeter::allocateId()
{
    static std::atomic<std::size_t> nextId{0};
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

void PayTableMeter::checkWithinBounds(const Symbol symbol)
{
    if (symbol >= MAX_SYMBOLS)
        throw std::runtime_error(
            "Symbol index " + std::to_string(symbol) +
            " out of bounds for PayTableMeter. Check this is correct or increase MAX_SYMBOLS in srcBackend/common/events/threadSafeMeters/PayTableMeter.h");
}

void PayTableMeter::checkWithinBounds(const Symbol symbol, const size_t oak)
{
    if (symbol >= MAX_SYMBOLS) [[unlikely]]
        throw std::runtime_error(
            "Symbol index " + std::to_string(symbol) +
            " out of bounds for PayTableMeter. Check this is correct or increase MAX_SYMBOLS in srcBackend/common/events/threadSafeMeters/PayTableMeter.h");
    if (oak > MAX_OAK) [[unlikely]]
        throw std::runtime_error(
            "OAK " + std::to_string(oak) +
            " out of bounds for PayTableMeter. Check this is correct or increase MAX_OAK in srcBackend/common/events/threadSafeMeters/PayTableMeter.h");
}
