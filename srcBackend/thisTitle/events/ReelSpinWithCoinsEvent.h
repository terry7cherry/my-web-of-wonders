#pragma once

#include "../../common/events/ReelSpinEvent.h"
#include "../CustomGrid.h"

class ReelSpinWithCoinsEvent : public ReelSpinEvent
{
public:
    std::vector<std::vector<unsigned int>> coinValues;
    unsigned int betMultiplier;

    explicit ReelSpinWithCoinsEvent(const CustomGrid& grid, const GameMaths& maths,
                                    const unsigned int betMultiplier)
        : ReelSpinEvent(grid, maths),
          coinValues(grid.coinValues),
          betMultiplier(betMultiplier)
    {
    }

    static void updateMeters(const CustomGrid& grid, const GameMaths& maths,
                             const unsigned int betMultiplier)
    {
        ReelSpinEvent::updateMeters(grid, maths);
    }

protected:
    Json::Value toJson() const override
    {
        Json::Value eventDetails = ReelSpinEvent::toJson();
        eventDetails["coinValues"] = Json::Value(Json::arrayValue);
        for (const auto& row : coinValues)
        {
            Json::Value jsonRow(Json::arrayValue);
            for (const auto& coinValue : row)
                jsonRow.append(coinValue * betMultiplier);
            eventDetails["coinValues"].append(jsonRow);
        }
        return eventDetails;
    }
};
