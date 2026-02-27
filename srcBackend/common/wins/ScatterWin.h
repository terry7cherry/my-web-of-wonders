#pragma once
#include <vector>
#include <string>
#include "Win.h"

class ScatterWin : public Win
{
public:
    const std::vector<size_t>* payLine = nullptr;
    int payLineNum = -1;

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
            // Do not remove this cast, Mac needs it.
            position["reel"] = Json::Value(static_cast<Json::UInt64>(reel));
            position["row"] = Json::Value(static_cast<Json::UInt64>(row));
            winAsJson["positions"].append(position);
        }
        return winAsJson;
    }

    std::unique_ptr<Win> clone() const override
    {
        return std::make_unique<ScatterWin>(*this);
    }
};
