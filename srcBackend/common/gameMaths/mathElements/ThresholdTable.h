#pragma once
/**
 * @file ThresholdTable.h
 * @brief Piecewise-constant lookup by integer thresholds with O(1) access. Designed for free spins trigger from scatter symbols and the like.
 * @details Also useful for trails where certain trail positions award particular features.
 */
#include <ranges>
#include <vector>
#include <unordered_map>

template <typename T>
class ThresholdTable
{
public:
    /**
     * @brief Build a dense lookup table from a sparse set of threshold->value entries.
     *
     * For any key k, the effective value is the value at the greatest threshold t <= k.
     * Undefined keys below the smallest threshold map to 0.
     */
    ThresholdTable(const std::unordered_map<int, T>& thresholds)
    {
        if (thresholds.empty())
        {
            throw std::runtime_error("ThresholdTable cannot be initialized with empty map.");
        }

        auto it = std::ranges::max_element(thresholds, {}, [](const auto& pair) { return pair.first; });
        maxKey = it->first;
        maxValue = it->second;

        for (size_t i = 0; i <= static_cast<size_t>(maxKey); i++)
        {
            int maxKeyLessThanOrEqualToI = std::numeric_limits<int>::min();
            bool keyFound = false;
            for (const auto& key : thresholds | std::views::keys)
            {
                if (key > maxKeyLessThanOrEqualToI && key <= i)
                {
                    maxKeyLessThanOrEqualToI = key;
                    keyFound = true;
                }
            }
            values.push_back(keyFound ? thresholds.at(maxKeyLessThanOrEqualToI) : 0);
        }
    }

    /**
     * @brief Get value for an integer key using threshold semantics.
     * @param key Key to lookup (e.g., num scatters).
     * @return Either value at index or:
     *         If larger than max key, max value returned.
     *         If smaller than smallest key, 0 returned.
     *         If between two keys, value at smaller key returned.
     */
    T at(const int key) const
    {
        return key > maxKey ? maxValue : values.at(key);
    }

    /**
     * @brief Get value for an integer key using threshold semantics.
     * @param key Key to lookup (e.g., num scatters).
     * @return Either value at index or:
     *         If larger than max key, max value returned.
     *         If smaller than smallest key, 0 returned.
     *         If between two keys, value at smaller key returned.
     */
    T at(const unsigned int key) const
    {
        return key > maxKey ? maxValue : values.at(key);
    }

    /**
     * @brief Get value for an integer key using threshold semantics.
     * @param key Key to lookup (e.g., num scatters).
     * @return Either value at index or:
     *         If larger than max key, max value returned.
     *         If smaller than smallest key, 0 returned.
     *         If between two keys, value at smaller key returned.
     */
    T operator[](const int key) const
    {
        return at(key);
    }

    /**
     * @brief Get value for an integer key using threshold semantics.
     * @param key Key to lookup (e.g., num scatters).
     * @return Either value at index or:
     *         If larger than max key, max value returned.
     *         If smaller than smallest key, 0 returned.
     *         If between two keys, value at smaller key returned.
     */
    T operator[](const unsigned int key) const
    {
        return at(key);
    }

private:
    std::vector<T> values;
    size_t maxKey;
    T maxValue;
};
