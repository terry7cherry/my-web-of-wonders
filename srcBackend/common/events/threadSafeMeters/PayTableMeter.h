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
#include "../../utils/utils.h"


class PayTableMeter
{
public:
    static constexpr std::size_t MAX_SYMBOLS = 50;
    static constexpr std::size_t MAX_OAK = 20;
    std::uint64_t global[MAX_SYMBOLS][MAX_OAK]{}; // zero-initialized

    PayTableMeter()
        : _id(allocateId())
    {
    }

    // Same API shape as DistributionMeter: single int key + value
    void add(Symbol symbol, size_t oak, std::uint64_t toAdd = 1);

    std::uint64_t* operator[](Symbol symbol);

    void flush();

private:
    // 2D global aggregated store
    std::mutex mutex;

    // TLS buffer per thread; each meter instance indexes into the array via _id
    using LocalArray = std::uint64_t[MAX_SYMBOLS][MAX_OAK];

    LocalArray* tls();

    static std::size_t allocateId();

    static void checkWithinBounds(const Symbol symbol);

    static void checkWithinBounds(const Symbol symbol, const size_t oak);

    const std::size_t _id;
};
