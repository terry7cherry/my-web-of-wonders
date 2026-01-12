#pragma once

#include "../grids/Grid.h"
#include "eventTemplates/GridEventTemplate.h"

class ReelSpinEvent : public GridEventTemplate<ReelSpinEvent>
{
public:
    explicit ReelSpinEvent(const Grid& grid) :
        GridEventTemplate(grid)
    {
    }

    static void updateMeters(const Grid& grid [[maybe_unused]])
    {
    }

protected:
    Json::Value toJson() const override
    {
        Json::Value eventDetails = GridEventTemplate::toJson();
        eventDetails["eventType"] = "reelSpin";
        return eventDetails;
    }
};
