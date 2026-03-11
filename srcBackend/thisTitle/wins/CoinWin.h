#pragma once

#include "../../common/wins/Win.h"
#include "../CollectorPositions.h"

class CoinWin : public Win
{
public:
    CollectorPosition collectorPosition{};

    CoinWin()
    {
        winType = "coin";
    }

    Json::Value toJson(const std::vector<std::string>& symbolNames) const override
    {
        Json::Value winAsJson = Win::toJson(symbolNames);
        winAsJson["collectorPosition"] = collectorPositionNames[collectorPosition];
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
        return std::make_unique<CoinWin>(*this);
    }
};
