#pragma once
#include <vector>
#include <string>
#include "Win.h"

class ScatterWin : public Win
{
public:
    const std::vector<size_t>* payLine = nullptr;
    int payLineNum = -1;
    std::vector<std::pair<size_t, size_t>> positions{};

    ScatterWin()
    {
        winType = "scatter";
    }

    Json::Value toJson(const std::vector<std::string>& symbolNames) const override
    {
        Json::Value winAsJson = Win::toJson(symbolNames);
        winAsJson["positions"] = Json::Value(Json::arrayValue);
        for (const auto& [reel, row] : positions)
        {
            Json::Value position;
            position["reel"] = reel;
            position["row"] = row;
            winAsJson["positions"].append(position);
        }
        return winAsJson;
    }

    std::unique_ptr<Win> clone() const override
    {
        return std::make_unique<ScatterWin>(*this);
    }
};
