#pragma once
/**
 * @file Win.h
 * @brief Represents a single payline win with metadata and formatting helpers.
 */
#include "../headers/types.h"
#include <map>
#include <string>
#include <memory>
#include <json/json.h>

enum class MultipliersCombineType
{
    none, add, multiply
};

/**
 * @brief Encapsulates a line win (symbol, of-a-kind, multipliers, direction, and text helpers).
 */
class Win
{
public:
    virtual ~Win() = default;
    Symbol symbol = SYMBOL_NONE;
    unsigned int oak = 0;
    unsigned int wildCount = 0;
    unsigned int multiplier = 1;
    unsigned int win = 0;
    unsigned int totalWin = 0;
    std::string winType;
    std::vector<std::pair<size_t, size_t>> positions{};

    /**
     * @brief Construct an empty win and reset state.
     */
    Win()
    {
        Win::reset(true);
    }

    /**
     * @brief Reset internal fields to default values.
     * @param force If true, reset even when `win` is zero.
     */
    virtual void reset(const bool force = false)
    {
        if (win || force)
        {
            symbol = 0;
            multiplier = 1;
            oak = wildCount = win = totalWin = 0;
        }
    }

    /**
     * @brief Combine a new multiplier into this win.
     * @param newMultiplier Multiplier to apply.
     * @param combineType Whether to multiply or add with existing multiplier.
     * @return The effective multiplier after applying.
     */
    unsigned int applyMultiplier(unsigned int newMultiplier, MultipliersCombineType combineType);

    /****************************************/
    // Display methods
    /****************************************/


    // Polymorphic clone to preserve derived data when copying via base pointer
    virtual std::unique_ptr<Win> clone() const { return std::make_unique<Win>(*this); }

    virtual Json::Value toJson(const std::vector<std::string>& symbolNames) const
    {
        Json::Value winAsJson;
        winAsJson["symbol"] = symbolNames[symbol];
        winAsJson["oak"] = oak;
        winAsJson["wildCount"] = wildCount;
        winAsJson["multiplier"] = multiplier;
        winAsJson["win"] = win;
        winAsJson["totalWin"] = totalWin;
        winAsJson["winType"] = winType;
        return winAsJson;
    }
};
