#pragma once
/**
 * @file MeterUpdate.h
 * @brief Lightweight pub-sub utility to trigger meter flushes across threads.
 *
 * Meters subscribe a `flush()` method). When
 * `triggerUpdate()` is called (e.g., once per simulated game), all callbacks are
 * invoked on the calling thread, allowing meters to aggregate thread-local data
 * into their global state deterministically.
 */
#include <functional>
#include <vector>

/**
 * @brief Static helper providing subscription and broadcast for flush callbacks.
 */
class MeterUpdate
{
public:
    using Callback = std::function<void()>;

    /**
     * @brief Register a callback to be invoked on the next trigger.
     */
    static void subscribe(Callback cb)
    {
        subscribers_.push_back(std::move(cb));
    }

    /**
     * @brief Invoke all registered callbacks (not thread-safe for concurrent mutation).
     */
    static void triggerUpdate()
    {
        for (auto& cb : subscribers_)
        {
            cb();
        }
    }

private:
    inline static thread_local std::vector<Callback> subscribers_;
};
