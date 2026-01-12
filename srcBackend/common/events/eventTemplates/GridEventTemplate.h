#pragma once

#include <json/json.h>
#include "./BasicEvent.h"
#include "../../grids/Grid.h"

static constexpr int GridUpdateMaxReels = 10;
static constexpr int GridUpdateMaxRows = 10;

template <typename T>
class GridEventTemplate : public BasicEvent
{
public:
    explicit GridEventTemplate(const Grid& grid)
        : numReels(grid.maxReels),
          numRows(grid.maxRows)
    {
        for (size_t reelNum = 0; reelNum < numReels; ++reelNum)
            for (size_t rowNum = 0; rowNum < numRows; ++rowNum)
                symbols[reelNum * numRows + rowNum] = grid.getReelSymbol(reelNum, rowNum);
    }

    static void updateMeters(const Grid& grid[[maybe_unused]])
    {
    }

protected :
    Json::Value toJson() const override
    {
        Json::Value eventDetails;
        eventDetails["eventType"] = "gridTemplate";
        Json::Value symbolsAsJson(Json::arrayValue);
        for (size_t reelNum = 0; reelNum < numReels; ++reelNum)
        {
            Json::Value reel(Json::arrayValue);
            for (size_t rowNum = 0; rowNum < numRows; ++rowNum)
            {
                reel.append(symbols[reelNum * numRows + rowNum]);
            }
            symbolsAsJson.append(reel);
        }
        eventDetails["reelSymbolArray"] = symbolsAsJson;
        return eventDetails;
    };

private :
    std::array<int, GridUpdateMaxReels * GridUpdateMaxRows> symbols{};
    size_t numReels;
    size_t numRows;
    std::string message;
};
