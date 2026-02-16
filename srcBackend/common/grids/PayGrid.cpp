#include "PayGrid.h"

PayGrid::PayGrid(const size_t reels, const size_t rows, const PayTable& payTable)
    : Grid(reels, rows),
      payTable(payTable)
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
    unsigned int reelsToCheck = 0;

    reelsToCheck = payTable.minWinSymbols;

    for (size_t reel = 0; quickWinMask && reel < reelsToCheck; ++reel)
    {
        uint32_t reelMask = 0;
        for (int row = 0; row < reelHeightVect[reel]; ++row)
        {
            const Symbol symId = getReelSymbol(reel, row);
            reelMask |= payTable.symbolVect[symId].payMask;
        }
        quickWinMask &= reelMask;
    }
    return quickWinMask;
}

unsigned int PayGrid::combineMultipliers(const unsigned int previousMultiplier,
                                         const unsigned int newMultiplier,
                                         const MultipliersCombineType combineType)
{
    if (newMultiplier <= 1)
        return previousMultiplier;
    if (combineType == MultipliersCombineType::multiply)
        return previousMultiplier * newMultiplier;
    if (previousMultiplier <= 1)
        return newMultiplier;
    return previousMultiplier + newMultiplier;
}
