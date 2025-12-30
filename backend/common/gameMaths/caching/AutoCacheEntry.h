#pragma once

#include <string>

template <typename T>
struct AutoCacheEntry
{
    enum class State : uint8_t
    {
        Unknown,
        Stable,
        Variant
    };

    State state = State::Unknown;
    std::string first_key;
    const T* cached_ptr = nullptr;
};
