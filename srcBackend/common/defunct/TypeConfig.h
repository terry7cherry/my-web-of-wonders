#pragma once
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

#include <absl/container/flat_hash_map.h>
#include <stdexcept>
#include "AutoCacheEntry.h"
#include "CallSite.h"
#include "CallSiteHash.h"

#define SIMPLE_TYPE_CONFIG

#ifdef SIMPLE_TYPE_CONFIG

template <typename T>
class TypeConfig
{
public:
    explicit TypeConfig() : TypeConfig({})
    {
    }

    explicit TypeConfig(const std::vector<std::string>& typeNames)
        : typeNames(typeNames)
    {
    }

    void reserve(size_t count)
    {
        data.reserve(count);
    }

    void insert(std::string key, T value)
    {
        data.emplace(std::move(key), std::move(value));
    }

    void insert(std::pair<std::string, T> pair)
    {
        data.emplace(std::move(pair.first), std::move(pair.second));
    }

    const T& at(const std::string& key) const
    {
        auto it = data.find(key);
        if (ABSL_PREDICT_TRUE(it != data.end()))
            return it->second;
        std::cerr << "Config with name '" + key + "' and type " + getTypeString() + "not found.\n"
            << "Note that for int, double, string, and bool types, the name is the 'name' property in the JSON tag, concatenated with the string in the left column of the table.\n"
            << "Ensure this config element is specified for the current rtp and betType or that it is covered by a more general definition."
            << std::endl;
        throw std::runtime_error("Config key " + key + " not found");
    }

    const T& operator[](const std::string& key) const
    {
        return at(key);
    }

private:
    std::string getTypeString() const
    {
        if (typeNames.empty()) return "UNKNOWN";
        if (typeNames.size() == 1) return typeNames[0];
        std::string result = typeNames[0];
        for (size_t i = 1; i < typeNames.size() - 1; i++) result += ", " + typeNames[i];
        return result + " or " + typeNames.back();
    }

    absl::flat_hash_map<std::string, T> data;
    std::vector<std::string> typeNames;
};

#else


template <typename T>
class TypeConfig
{
public:
    explicit TypeConfig() : TypeConfig({})
    {
    }

    explicit TypeConfig(const std::vector<std::string>& typeNames)
        : typeNames(typeNames)
    {
    }

    void reserve(size_t count)
    {
        data_.reserve(count);
    }

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

return lookup (key);
    }

const T& operator[](const std::string& key) const
{
    return at(key);
}

private:
using EntryState = AutoCacheEntry<T>::State;

const T& lookup(const std::string& key) const
{
    auto it = data_.find(key);
    if (it == data_.end())
    {
        std::cerr << "Game math failed to find requested math element with key: " << key << std::endl;
        throw std::runtime_error("Config key not found");
    }
    return it->second;
}

absl::flat_hash_map<std::string, T> data_;
mutable absl::flat_hash_map<CallSite,
                            AutoCacheEntry<T>,
                            CallSiteHash> cache_;
std::vector<std::string> typeNames;
};
#endif
