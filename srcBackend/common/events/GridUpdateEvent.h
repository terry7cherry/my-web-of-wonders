#pragma once

#include "eventTemplates/GridEventTemplate.h"
#include "../grids/Grid.h"

class GridUpdateEvent : public GridEventTemplate<GridUpdateEvent>
{
public:
    explicit GridUpdateEvent(const Grid& grid, std::string message = "Grid updated!")
        : GridEventTemplate(grid),
          message(std::move(message))
    {
    }

    static void updateMeters(const Grid& grid[[maybe_unused]],
                             std::string message [[maybe_unused]] = "Grid updated!")
    {
    }

protected :
    Json::Value toJson() const override
    {
        Json::Value eventDetails = GridEventTemplate::toJson();
        eventDetails["eventType"] = "gridUpdate";
        eventDetails["message"] = message;
        return eventDetails;
    }

private :
    std::string message;
};
