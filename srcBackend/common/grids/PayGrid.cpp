#include "PayGrid.h"

PayGrid::PayGrid(const size_t reels, const size_t rows, const PayTable& payTable)
    : Grid(reels, rows),
      lpPayTable(&payTable)
{
    isWinVect.resize(maxReels);
}

PayGrid::PayGrid(const GameMaths& maths)
    : PayGrid(maths.ints.at("numReels"), maths.ints.at("numRows"), maths.payTables.at("default"))
{
}

void PayGrid::reset()
{
    Grid::reset();
    for (int reel = 0; reel < maxReels; reel++)
        isWinVect[reel].assign(maxRows, false);
    winIndexVect.clear();
}

uint32_t PayGrid::quickWinCheck()
{
    quickWinMask = 0xFFFFFFFF;
    int reelsToCheck = 0;

    reelsToCheck = lpPayTable->minWinSymbols;

    for (int reel = 0; quickWinMask && reel < reelsToCheck; ++reel)
    {
        uint32_t reelMask = 0;
        for (int row = 0; row < reelHeightVect[reel]; ++row)
        {
            const Symbol symId = getReelSymbol(reel, row);
            reelMask |= lpPayTable->symbolVect[symId].payMask;
        }
        quickWinMask &= reelMask;
    }
    return quickWinMask;
}
