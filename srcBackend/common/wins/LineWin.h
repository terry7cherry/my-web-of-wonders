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

    std::unique_ptr<Win> clone() const override
    {
        return std::make_unique<LineWin>(*this);
    }

    void setPositions()
    {
        if (payLine == nullptr) [[unlikely]] throw std::runtime_error(
            "Attempt to set positions on line win before payline is set.");
        for (size_t reel = 0; reel < oak; reel++)
            positions.emplace_back(reel, (*payLine)[reel]);
    }
};
