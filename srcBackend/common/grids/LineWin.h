#pragma once
#include <map>
#include <string>
#include <vector>

#include "PayGrid.h"

#define MAX_LINE_TEXT_LEN 45

class LineWin
{
public:
    uint8_t symbol = 0;
    unsigned int oak = 0;
    unsigned int wildCount = 0;
    unsigned int multiplier = 1;
    unsigned int win = 0;
    unsigned int totalWin = 0;
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

    unsigned int applyMultiplier(unsigned int newMultiplier, MultipliersCombineType combineType);

    /****************************************/
    // Display methods
    /****************************************/

    // Generates text that describes the LineWin
    // returns string in the format, "Line 7 L-R: 3 H1 = 40 (x2) = 80"
    std::string getWinText(std::string szSymbol, bool showDirection = true) const;


    // Generates text that describes the LineWin
    // returns string in the format, "Line 7: 3 H1 = 40 [x2 line multiplier = 80]"
    std::string getWinText(const std::map<uint8_t, std::string>& symbolMap,
                           bool showDirection = false) const;
};
