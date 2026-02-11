#pragma once
#include <vector>
#include <string>
#include "Win.h"

enum class WinDirections
{
    none, left_right, right_left, both
};

class LineWin : public Win
{
public:
    WinDirections direction = WinDirections::none;
    const std::vector<size_t>* payLine = nullptr;
    int payLineNum = -1;

    LineWin()
    {
        LineWin::reset(true);
        winType = "line";
    }

    void reset(const bool force [[maybe_unused]] = false) override
    {
        direction = WinDirections::none;
    }

    Json::Value toJson(const std::vector<std::string>& symbolNames) const override
    {
        Json::Value winAsJson = Win::toJson(symbolNames);
        winAsJson["direction"] = static_cast<int>(direction);
        winAsJson["payLine"] = Json::Value(Json::arrayValue);
        for (const auto& aperture : *payLine)
            winAsJson["payLine"].append(aperture);
        return winAsJson;
    }

    std::string getWinText(std::string szSymbol, const bool showDirection) const override
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
                    static_cast<int>(oak),
                    szSymbol.c_str(),
                    static_cast<int>(win)
                );
            }
            else
            {
                size = std::snprintf(
                    buffer, space,
                    "Line%2d: %d %s = %d ",
                    payLineNum,
                    static_cast<int>(oak),
                    szSymbol.c_str(),
                    static_cast<int>(win)
                );
            }
        }
        else
        {
            size = std::snprintf(
                buffer, space,
                "Scatter: %d %s = %d ",
                static_cast<int>(oak),
                szSymbol.c_str(),
                static_cast<int>(win)
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
                static_cast<int>(multiplier),
                static_cast<int>(totalWin)
            );
            if (added < 0) added = 0;
            if (added > space) added = space;
            size += added;
        }

        return std::string(buffer, size);
    }

    std::string getWinText(const std::map<Symbol, std::string>& symbolMap,
                           const bool showDirection) const override
    {
        const std::string szSymbol = symbolMap.at(symbol);
        return getWinText(szSymbol, showDirection);
    }

    std::unique_ptr<Win> clone() const override
    {
        return std::make_unique<LineWin>(*this);
    }
};
