#pragma once

#include "../../common/events/CascadeEvent.h"
#include "../CustomGrid.h"

class CascadeWithCoinsEvent : public CascadeEvent
{
public:
    std::vector<std::vector<unsigned int>> coinValues;

    explicit CascadeWithCoinsEvent(
        const std::vector<std::pair<size_t, size_t>>& removedSymbols,
        const CustomGrid& grid,
        const GameMaths& maths) :
        CascadeEvent(removedSymbols, grid, maths),
        coinValues(grid.coinValues)
    {
    }

    static void updateMeters(
        const std::vector<std::pair<size_t, size_t>>& removedSymbols,
        const CustomGrid& grid,
        const GameMaths& maths)
    {
        CascadeEvent::updateMeters(removedSymbols, grid, maths);
    }

protected:
    Json::Value toJson() const override
    {
        Json::Value eventDetails = CascadeEvent::toJson();
        eventDetails["coinValues"] = Json::Value(Json::arrayValue);
        for (const auto& row : coinValues)
        {
            Json::Value jsonRow(Json::arrayValue);
            for (const auto& coinValue : row)
                jsonRow.append(coinValue);
            eventDetails["coinValues"].append(jsonRow);
        }
        eventDetails["removedSymbolsPerReel"] = Json::Value(Json::arrayValue);
        for (size_t i = 0; i < numReels; i++)
            eventDetails["removedSymbolsPerReel"].append(0);
        for (const auto& reel : removedSymbols | std::views::keys)
            eventDetails["removedSymbolsPerReel"][static_cast<Json::ArrayIndex>(reel)] =
                eventDetails["removedSymbolsPerReel"][static_cast<Json::ArrayIndex>(reel)].asInt() + 1;
        return eventDetails;
    }
};
