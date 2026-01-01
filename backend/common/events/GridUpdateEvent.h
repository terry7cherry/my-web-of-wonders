#pragma once

#include "BasicEvent.h"

#include <utility>

#include "../grids/Grid.h"

static constexpr int GridUpdateMaxReels = 10;
static constexpr int GridUpdateMaxRows = 10;

class GridUpdateEvent : public BasicEvent
{
public:
    explicit GridUpdateEvent(const Grid& grid, std::string message = "Grid updated!")
        : numReels(grid.maxReels),
          numRows(grid.maxRows),
          message(std::move(message))
    {
        for (size_t reelNum = 0; reelNum < numReels; ++reelNum)
        {
            for (size_t rowNum = 0; rowNum < numRows; ++rowNum)
            {
                symbols[reelNum * numRows + rowNum] = grid.getReelSymbol(reelNum, rowNum);
            }
        }
    }

    static void updateMeters(const Grid& grid, std::string message = "Grid updated!")
    {
    }

protected :
    static constexpr std::string_view EventType = "gridUpdate";

    std::string_view getEventType() const override
    {
        return EventType;
    }

    Json::Value getDetailsAsJson() const override
    {
        Json::Value eventDetails;

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
        eventDetails["message"] = std::string(message);
        return eventDetails;
    }

private :
    std::array<int, GridUpdateMaxReels * GridUpdateMaxRows> symbols{};
    int numReels;
    int numRows;
    std::string message;
};
