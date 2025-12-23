#include "LineWin.h"
#include <algorithm>

int LineWin::applyMultiplier(const int newMultiplier, const MultipliersCombineType combineType)
{
    if (newMultiplier <= 0)
        reportError("Win.applyMultiplier", "Passed multiplier <= 0");
    else if (multiplier <= 0)
        reportError("Win.applyMultiplier", "Win multiplier <= 0");
    else
    {
        // update the win multiplier and win
        if (combineType == MultipliersCombineType::add)
            multiplier = (multiplier <= 1)
                             ? newMultiplier
                             : multiplier + newMultiplier;
        else if (combineType == MultipliersCombineType::multiply)
            multiplier *= newMultiplier;
        totalWin = win * multiplier;
    }
    return totalWin;
}

std::string LineWin::getWinText(std::string szSymbol, const bool showDirection) const
{
    char buffer[MAX_LINE_TEXT_LEN];

    // remove spaces from symbol
    const auto end_pos = std::ranges::remove(szSymbol, ' ').begin();
    szSymbol.erase(end_pos, szSymbol.end());

    int space = MAX_LINE_TEXT_LEN;
    int size = 0;

    if (winType == "line")
    {
        if (showDirection)
        {
            size = std::snprintf(
                buffer, space,
                "Line%2d %s: %d %s = %d ",
                payLineNum,
                (direction == WinDirections::left_right) ? "L-R" : "R-L",
                oak,
                szSymbol.c_str(),
                win
            );
        }
        else
        {
            size = std::snprintf(
                buffer, space,
                "Line%2d: %d %s = %d ",
                payLineNum,
                oak,
                szSymbol.c_str(),
                win
            );
        }
    }
    else
    {
        size = std::snprintf(
            buffer, space,
            "Scatter: %d %s = %d ",
            oak,
            szSymbol.c_str(),
            win
        );
    }

    if (size < 0) size = 0;
    if (size > space) size = space;
    space -= size;

    if (multiplier > 1 && space > 0)
    {
        int added = std::snprintf(
            buffer + size, space,
            "(x%d) = %d",
            multiplier,
            totalWin
        );
        if (added < 0) added = 0;
        if (added > space) added = space;
        size += added;
    }

    return std::string(buffer, size);
}

std::string LineWin::getWinText(const std::map<int, std::string>& symbolMap,
                                const bool showDirection) const
{
    const std::string szSymbol = symbolMap.at(symbol);
    return getWinText(szSymbol, showDirection);
}
