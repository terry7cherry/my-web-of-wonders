#include "ReelSpinEvent.h"

ReelSpinEvent::ReelSpinEvent(const Grid& grid)
    : numReels(grid.maxReels),
      numRows(grid.maxRows)
{
    for (size_t reelNum = 0; reelNum < numReels; ++reelNum)
    {
        for (size_t rowNum = 0; rowNum < numRows; ++rowNum)
        {
            symbols[reelNum * numRows + rowNum] = grid.getReelSymbol(reelNum, rowNum);
        }
    }
}

Json::Value ReelSpinEvent::getDetailsAsJson() const
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
    return eventDetails;
}
