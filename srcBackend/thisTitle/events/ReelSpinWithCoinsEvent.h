#pragma once

#include "../../common/events/ReelSpinEvent.h"
#include "../CustomGrid.h"

class ReelSpinWithCoinsEvent : public ReelSpinEvent
{
public:
    std::vector<std::vector<unsigned int>> coinValues;

    explicit ReelSpinWithCoinsEvent(const CustomGrid& grid, const GameMaths& maths)
    :ReelSpinEvent(grid, maths),
    coinValues(grid.coinValues)
    {

    }

    static void updateMeters(const CustomGrid& grid, const GameMaths& maths)
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
                jsonRow.append(coinValue);
            eventDetails["coinValues"].append(jsonRow);
        }
        return eventDetails;
    }
};