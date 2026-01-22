#pragma once
/**
 * @file ReelSpinEvent.h
 * @brief Event emitted after reels have spun. For frontend.
 */

#include "../grids/Grid.h"
#include "eventTemplates/GridEventTemplate.h"

/**
 * @brief Reel Spin event that is autohandled in the frontend.
 */
class ReelSpinEvent : public GridEventTemplate<ReelSpinEvent>
{
public:
    /**
     * @brief Construct from a grid snapshot.
     * @param grid Grid object to take symbols from.
     */
    explicit ReelSpinEvent(const Grid& grid) :
        GridEventTemplate(grid)
    {
    }

    /**
     * @brief Optional metrics hook (unused).
     * @param grid Grid object to take symbols from.
     * @details Note this is required because updateMeters arguments must match constructor.
     */
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
