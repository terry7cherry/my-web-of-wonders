#pragma once
#include <ranges>
#include <vector>
#include <unordered_map>

template <typename T>
class ThresholdTable
{
public:
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

    T at(const int key) const
    {
        return key > maxKey ? maxValue : values.at(key);
    }

private:
    std::vector<T> values;
    int maxKey;
    T maxValue;
};
