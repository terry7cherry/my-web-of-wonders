#pragma once
#include <map>
#include <string>
#include <vector>

#include "PayGrid.h"

#define MAX_LINE_TEXT_LEN 45

class LineWin
{
public:
    size_t symbol = 0;
    size_t oak = 0;
    size_t wildCount = 0;
    size_t multiplier = 1;
    size_t win = 0;
    size_t totalWin = 0;
    WinDirections direction = WinDirections::none;
    const std::vector<size_t>* payLine = nullptr;
    int payLineNum = -1;
    std::string winType;

    LineWin()
    {
        reset(true);
    }

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

    size_t applyMultiplier(size_t newMultiplier, MultipliersCombineType combineType);

    /****************************************/
    // Display methods
    /****************************************/

    // Generates text that describes the LineWin
    // returns string in the format, "Line 7 L-R: 3 H1 = 40 (x2) = 80"
    std::string getWinText(std::string szSymbol, bool showDirection = true) const;


    // Generates text that describes the LineWin
    // returns string in the format, "Line 7: 3 H1 = 40 [x2 line multiplier = 80]"
    std::string getWinText(const std::map<int, std::string>& symbolMap,
                           bool showDirection = false) const;
};
