#pragma once
/**
 * @file LineWin.h
 * @brief Represents a single payline win with metadata and formatting helpers.
 */
#include <map>
#include <string>
#include <vector>

#include "PayGrid.h"

#define MAX_LINE_TEXT_LEN 45

/**
 * @brief Encapsulates a line win (symbol, of-a-kind, multipliers, direction, and text helpers).
 */
class Win
{
public:
    Symbol symbol = 0;
    unsigned int oak = 0;
    unsigned int wildCount = 0;
    unsigned int multiplier = 1;
    unsigned int win = 0;
    unsigned int totalWin = 0;
    WinDirections direction = WinDirections::none;
    const std::vector<size_t>* payLine = nullptr;
    int payLineNum = -1;
    std::string winType;

    /**
     * @brief Construct an empty win and reset state.
     */
    Win()
    {
        reset(true);
    }

    /**
     * @brief Reset internal fields to default values.
     * @param force If true, reset even when `win` is zero.
     */
    void reset(bool force = false)
    {
        if (win || force)
        {
            symbol = 0;
            multiplier = 1;
            oak = wildCount = win = totalWin = 0;
            direction = WinDirections::none;
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

    /**
     * @brief Format the win as text.
     * @return For example: "Line 7 L-R: 3 H1 = 40 (x2) = 80".
     */
    std::string getWinText(std::string szSymbol, bool showDirection = true) const;


    /**
     * @brief Format the win as text using a symbol map.
     * @return For example: "Line 7: 3 H1 = 40 [x2 line multiplier = 80]".
     */
    std::string getWinText(const std::map<Symbol, std::string>& symbolMap,
                           bool showDirection = false) const;
};
