#pragma once
/**
 * @file GridUpdateEvent.h
 * @brief Event emitted when a grid's state is updated.
 */

#include "eventTemplates/GridEventTemplate.h"
#include "../grids/Grid.h"

/**
 * @brief Reports a grid update with a descriptive message.
 * @details Handled automatically on frontend as follows:
 *           - Altered symbols only are updated with scrolling animation.
 *           - Ticker displays message passed in constructor.
 * If you require different frontend functionality it is recommended to create your own event inheriting from GridEventTemplate.
 */
class GridUpdateEvent : public GridEventTemplate<GridUpdateEvent>
{
public:
    /** @brief Message to display in frontend ticker while grid updates. **/
    std::string message;

    /**
     * @param grid Grid object from which to take symbols.
     * @param maths GameMaths object for symbol names.
     * @param message Message to display on frontend ticker.
     */
    explicit GridUpdateEvent(const Grid& grid,
                             const GameMaths& maths,
                             const std::string& message = "Grid updated!")
        : GridEventTemplate(grid, maths),
          message(message)
    {
    }

    /**
     * @param grid Grid object from which to take symbols.
     * @param maths GameMaths object for symbol names.
     * @param message Message to display on frontend ticker.
     */
    static void updateMeters(const Grid& grid [[maybe_unused]],
                             const GameMaths& maths [[maybe_unused]],
                             const std::string& message [[maybe_unused]] = "Grid updated!")
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
};
