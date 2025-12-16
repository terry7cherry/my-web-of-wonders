#pragma once

#include "BasicEvent.h"

#include <vector>

#include "../grids/Grid.h"

static constexpr int ReelSpinMaxReels = 10;
static constexpr int ReelSpinMaxRows = 10;

class ReelSpinEvent : public BasicEvent {
public:
    explicit ReelSpinEvent(const Grid &grid)
#ifdef FRONTEND
    : numReels (grid.maxReels),
    numRows (grid.maxRows)
	{
		for (size_t reelNum = 0; reelNum < numReels; ++reelNum) {
			for (size_t rowNum = 0; rowNum < numRows; ++rowNum) {
				symbols[reelNum * numRows + rowNum] = grid.getReelSymbol(reelNum, rowNum);
			}
		}
#else
    {
#endif
    }

protected:
    static constexpr std::string_view EventType = "reelSpin";

    std::string_view getEventType() const override {
        return EventType;
    }

#ifdef FRONTEND
    Json::Value getDetailsAsJson() const override {
        Json::Value eventDetails;

        Json::Value symbolsAsJson(Json::arrayValue);

        for (size_t reelNum = 0; reelNum < numReels; ++reelNum) {
            Json::Value reel(Json::arrayValue);
            for (size_t rowNum = 0; rowNum < numRows; ++rowNum) {
                reel.append(symbols[reelNum * numRows + rowNum]);
            }
            symbolsAsJson.append(reel);
        }
        eventDetails["reelSymbolArray"] = symbolsAsJson;
        return eventDetails;
    }
#endif

private:
#ifdef FRONTEND
    std::array<int, ReelSpinMaxReels * ReelSpinMaxRows> symbols{};
    int numReels;
    int numRows;
#endif
};