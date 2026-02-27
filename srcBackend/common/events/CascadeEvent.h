#pragma once
/**
 * @file CascadeEvent.h
 * @brief Event emitted when a cascade occurs.
 */

#include "eventTemplates/GridEventTemplate.h"

/**
 * @brief Reports a cascade event including positions of symbols removed and new grid.
 * @details Handled automatically with animations on the frontend.
 */
class CascadeEvent : public GridEventTemplate<CascadeEvent>
{
public:
    /** @brief reel-row pairs representing the positions of symbols removed. */
    std::vector<std::pair<size_t, size_t>> removedSymbols;

    /**
     * @param removedSymbols Reel-row pairs representing the positions of symbols Removed.
     * @param grid Grid object from which to get updates board state.
     * @param maths GameMaths object for symbol names.
     */
    explicit CascadeEvent(const std::vector<std::pair<size_t, size_t>>& removedSymbols,
                          const Grid& grid,
                          const GameMaths& maths)
        : GridEventTemplate(grid, maths),
          removedSymbols(removedSymbols)
    {
    }

    /**
     * @param removedSymbols Reel-row pairs representing the positions of symbols Removed.
     * @param grid Grid object from which to get updates board state.
     * @param maths GameMaths object for symbol names.
     */
    static void updateMeters(const std::vector<std::pair<size_t, size_t>>& removedSymbols [[maybe_unused]],
                             const Grid& grid [[maybe_unused]],
                             const GameMaths& maths [[maybe_unused]])
    {
    }

protected:
    Json::Value toJson() const override
    {
        Json::Value eventDetails = GridEventTemplate::toJson();
        eventDetails["eventType"] = "cascade";
        eventDetails["removedSymbols"] = Json::Value();
        for (const auto& [reel, row] : removedSymbols)
        {
            Json::Value position;
            position["reel"] = Json::Value(static_cast<Json::UInt64>(reel));
            position["row"] = Json::Value(static_cast<Json::UInt64>(row));
            eventDetails["removedSymbols"].append(position);
        }
        return eventDetails;
    }
};
