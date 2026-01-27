#pragma once
/**
 * @file PayTableMeter.h
 * @brief Thread-safe (per-thread buffered) 2D distribution meter with global aggregation.
 *
 * API mirrors DistributionMeter (int -> uint64_t) but uses a fixed-size 2D array as the
 * backing store. Keys are mapped to 2D indices via: keyA = key / MAX_OAK, keyB = key % MAX_OAK.
 */

#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#include "MeterUpdate.h"


class PayTableMeter
{
public:
    static constexpr std::size_t MAX_SYMBOLS = 50;
    static constexpr std::size_t MAX_OAK = 10;
    std::uint64_t global[MAX_SYMBOLS][MAX_OAK]{}; // zero-initialized

    PayTableMeter()
        : _id(allocateId())
    {
    }

    // Same API shape as DistributionMeter: single int key + value
    void add(const Symbol symbol, const size_t oak, const std::uint64_t toAdd = 1)
    {
        auto& local = tls()[_id];
        checkWithinBounds(symbol, oak);
        local[symbol][oak - 1] += toAdd;
    }

    std::uint64_t* operator[](const Symbol symbol)
    {
        auto& local = tls()[_id];
        checkWithinBounds(symbol);
        return local[symbol];
    }

    void flush()
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

private:
    // 2D global aggregated store
    std::mutex mutex;

    // TLS buffer per thread; each meter instance indexes into the array via _id
    using LocalArray = std::uint64_t[MAX_SYMBOLS][MAX_OAK];

    LocalArray* tls()
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

    static std::size_t allocateId()
    {
        static std::atomic<std::size_t> nextId{0};
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }

    static void checkWithinBounds(const Symbol symbol)
    {
        if (symbol >= MAX_SYMBOLS)
            throw std::runtime_error(
                "Symbol index " + std::to_string(symbol) +
                " out of bounds for PayTableMeter. Check this is correct or increase MAX_SYMBOLS in srcBackend/common/events/threadSafeMeters/PayTableMeter.h");
    }

    static void checkWithinBounds(const Symbol symbol, const size_t oak)
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


    const std::size_t _id;
};
