#pragma once
/**
 * @file ReelSpinWithStickySymbolsEvent.h
 * @brief Reel spin event for frontend that specifies any sticky symbols.
 */

#include "ReelSpinEvent.h"

/**
 *@brief Reel spin event with sticky symbols that is auto-handled in the frontend.
*/
class ReelSpinWithStickySymbolsEvent : public ReelSpinEvent
{
public:
    const std::vector<std::pair<size_t, size_t>> stickySymbols;

    /**
     * @brief Construct reel spin event with sticky symbols.
     * @param grid Grid object to take symbols from.
     * @param maths GameMaths object for symbol names.
     * @param stickyPositions Vector of <reel, row> pairs representing sticky symbol positions.
     */
    explicit ReelSpinWithStickySymbolsEvent(const Grid& grid,
                                            const GameMaths& maths,
                                            const std::vector<std::pair<size_t, size_t>>& stickyPositions) :
        ReelSpinEvent(grid, maths), stickySymbols(stickyPositions)
    {
    }

    /**
     * @brief Optional metrics hook (unused).
     * @param grid Grid object to take symbols from.
     * @param maths GameMaths object for symbol names.
     * @param stickyPositions Vector of <reel, row> pairs representing sticky symbol positions.
     * @details Note this is required because updateMeters arguments must match constructor.
     */
    static void updateMeters(const Grid& grid [[maybe_unused]],
                             const GameMaths& maths [[maybe_unused]],
                             const std::vector<std::pair<size_t, size_t>>& stickyPositions [[maybe_unused]])
    {
    }

protected:
    Json::Value toJson() const override
    {
        Json::Value eventDetails = ReelSpinEvent::toJson();
        eventDetails["eventType"] = "reelSpinWithStickySymbols";
        eventDetails["stickyPositions"] = Json::Value(Json::arrayValue);
        for (const auto& [reel, row] : stickySymbols)
        {
            Json::Value position;
            position["reel"] = Json::Value(static_cast<Json::UInt64>(reel));
            position["row"] = Json::Value(static_cast<Json::UInt64>(row));
            eventDetails["stickyPositions"].append(position);
        }
        return eventDetails;
    }
};

