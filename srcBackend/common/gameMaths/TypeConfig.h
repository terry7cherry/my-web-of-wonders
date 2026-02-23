#pragma once
/**
 * @file TypeConfig.h
 * @brief Generic typed configuration map with helpful accessors and error reporting.
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

#include <absl/container/flat_hash_map.h>
#include <stdexcept>

template <typename T>
class TypeConfig
{
public:
    /**
     * @brief Construct an empty config with no type names.
     */
    explicit TypeConfig() : TypeConfig({})
    {
    }

    /**
     * @brief Construct with a list of human-readable type names for messages.
     * @param typeNames List of type strings the PAR sheet parser supports for this type. Used in errors when keys are not found.
     */
    explicit TypeConfig(const std::vector<std::string>& typeNames)
        : typeNames(typeNames)
    {
    }

    /**
     * @brief Reserve capacity for `count` entries.
     */
    void reserve(size_t count)
    {
        data.reserve(count);
    }

    /**
     * @brief Insert a key/value pair (moved in when possible).
     */
    void insert(std::string key, T value)
    {
        data.emplace(std::move(key), std::move(value));
    }

    /**
     * @brief Insert from an existing pair.
     */
    void insert(std::pair<std::string, T> pair)
    {
        data.emplace(std::move(pair.first), std::move(pair.second));
    }

    /**
     * @brief Check if a config element with this name exists.
     * @param key Name of config element (concatenated from "name" parameter and first column value with "." separator for some types.
     * @return Does this key exist?
     */
    bool contains(const std::string& key) const
    {
        return data.contains(key);
    }

    /**
     * @brief Access value by key with helpful error if missing.
     * @param key Name of config element (concatenated from "name" parameter and first column value with "." separator for some types.
     * @throws std::runtime_error if the key is not found.
     */
    const T& at(const std::string& key) const
    {
        auto it = data.find(key);
        if (ABSL_PREDICT_TRUE(it != data.end()))
            return it->second;
        std::cerr << "Config with name '" + key + "' and type " + getTypeString() + " not found.\n"
            << "Note that for int, double, string, and bool types, the name is the 'name' property in the JSON tag, concatenated with the string in the left column of the table.\n"
            << "Ensure this config element is specified for the current rtp and betType or that it is covered by a more general definition."
            << std::endl;
        throw std::runtime_error(
            "Config with name '" + key + "' and type " + getTypeString() + " not found.\n");
    }

    /**
    * @brief Access value by key with helpful error if missing.
     * @param key Name of config element (concatenated from "name" parameter and first column value with "." separator for some types.
     * @throws std::runtime_error if the key is not found.
     */
    const T& operator[](const std::string& key) const
    {
        return at(key);
    }

private:
    /**
     * @brief Get a printable type string (e.g., "int or double").
     */
    std::string getTypeString() const
    {
        if (typeNames.empty()) return "UNKNOWN";
        if (typeNames.size() == 1) return typeNames[0];
        std::string result = typeNames[0];
        for (size_t i = 1; i < typeNames.size() - 1; i++) result += ", " + typeNames[i];
        return result + " or " + typeNames.back();
    }

    std::unordered_map<std::string, T> data;
    std::vector<std::string> typeNames;
};

