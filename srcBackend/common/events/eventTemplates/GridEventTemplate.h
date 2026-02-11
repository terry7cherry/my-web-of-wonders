#pragma once
/**
 * @file GridEventTemplate.h
 * @brief Template for events where the frontend needs to receive the grid.
 */

#include <json/json.h>
#include "./BasicEvent.h"
#include "../../grids/Grid.h"


/**
 * @brief Base template for events where the frontend needs to receive the grid.
 * @attention When extending this class, you MUST pass the child class name as a template parameter to get separate stats. You must also set phaseName (use ReelSpinEvent as a guide).
 * @tparam T Derived event type providing `phaseName`.
 */
template <typename T>
class GridEventTemplate : public BasicEvent
{
public:
    std::vector<std::vector<Symbol>> symbols{};
    const std::vector<std::string>& symbolNames;
    size_t numReels;
    size_t numRows;
    std::string message;

    /**
     * @brief Capture current grid symbols at construction.
     * @param grid Grid object from which to get symbols.
     * @param maths Game maths object for symbol names.
     */
    explicit GridEventTemplate(const Grid& grid, const GameMaths& maths)
        : symbolNames(maths.payTables["default"].symbolNames),
          numReels(grid.maxReels),
          numRows(grid.maxRows)
    {
        for (size_t reelNum = 0; reelNum < numReels; ++reelNum)
        {
            std::vector<Symbol> reelSymbols(numRows);
            for (size_t rowNum = 0; rowNum < grid.reelHeightVect[reelNum]; ++rowNum)
                reelSymbols[rowNum] = grid.getReelSymbol(reelNum, rowNum);
            symbols.push_back(reelSymbols);
        }
    }

    /**
     * @brief Optional metrics hook for derived event types.
     * @param grid Grid object from which to get symbols.
     */
    static void updateMeters(const Grid& grid[[maybe_unused]])
    {
    }

protected :
    /**
     * @brief Serialize common grid data to JSON for derived events.
     */
    Json::Value toJson() const override
    {
        Json::Value eventDetails;
        eventDetails["eventType"] = "gridTemplate";
        Json::Value symbolsAsJson(Json::arrayValue);
        for (const auto& reel : symbols)
        {
            Json::Value jsonReel(Json::arrayValue);
            for (const auto& symbol : reel)
                jsonReel.append(symbolNames[symbol]);
            symbolsAsJson.append(jsonReel);
        }
        eventDetails["reelSymbolArray"] = symbolsAsJson;
        return eventDetails;
    };
};
