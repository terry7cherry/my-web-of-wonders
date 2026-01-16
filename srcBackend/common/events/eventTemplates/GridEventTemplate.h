#pragma once

#include <json/json.h>
#include "./BasicEvent.h"
#include "../../grids/Grid.h"


template <typename T>
class GridEventTemplate : public BasicEvent
{
public:
    explicit GridEventTemplate(const Grid& grid)
        : numReels(grid.maxReels),
          numRows(grid.maxRows)
    {
        for (size_t reelNum = 0; reelNum < numReels; ++reelNum)
        {
            std::vector<Symbol> reelSymbols(numRows);
            for (size_t rowNum = 0; rowNum < numRows; ++rowNum)
                reelSymbols[rowNum] = grid.getReelSymbol(reelNum, rowNum);
            symbols.push_back(reelSymbols);
        }
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
            Json::Value reel = BasicEvent::toJsonArray(symbols[reelNum]);
            symbolsAsJson.append(reel);
        }
        eventDetails["reelSymbolArray"] = symbolsAsJson;
        return eventDetails;
    };

private :
    std::vector<std::vector<Symbol>> symbols{};
    size_t numReels;
    size_t numRows;
    std::string message;
};
