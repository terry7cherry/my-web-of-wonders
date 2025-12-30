#pragma once

#include <unordered_map>
#include <stdexcept>
#include "AutoCacheEntry.h"
#include "CallSite.h"
#include "CallSiteHash.h"

template <typename T>
class TypeConfig
{
public:
    void insert(std::string key, T value)
    {
        data_.emplace(std::move(key), std::move(value));
    }

    void insert(std::pair<std::string, T> pair)
    {
        data_.emplace(std::move(pair.first), std::move(pair.second));
    }

    const T& at(const std::string& key,
                CallSite cs = CallSite::current()) const
    {
        auto& entry = cache_[cs];

        // HOT PATH: stable and same key
        if (entry.state == EntryState::Stable &&
            key == entry.first_key)
        {
            return *entry.cached_ptr;
        }

        // VARIANT: always lookup
        if (entry.state == EntryState::Variant)
        {
            return lookup(key);
        }

        // FIRST USE
        if (entry.state == EntryState::Unknown)
        {
            const T& value = lookup(key);
            entry.first_key = key;
            entry.cached_ptr = &value;
            entry.state = EntryState::Stable;
            return value;
        }

        // STABLE → VARIANT transition
        entry.state = EntryState::Variant;
        entry.cached_ptr = nullptr;

#ifndef NDEBUG
        // Optional diagnostic hook
#endif

        return lookup(key);
    }

private:
    using EntryState = AutoCacheEntry<T>::State;

    const T& lookup(const std::string& key) const
    {
        auto it = data_.find(key);
        if (it == data_.end())
            throw std::runtime_error("Config key not found");
        return it->second;
    }

    std::unordered_map<std::string, T> data_;
    mutable std::unordered_map<CallSite,
                               AutoCacheEntry<T>,
                               CallSiteHash> cache_;
};
