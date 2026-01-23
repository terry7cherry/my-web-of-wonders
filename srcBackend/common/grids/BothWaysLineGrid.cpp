#include "BothWaysLineGrid.h"

uint32_t BothWaysLineGrid::quickWinCheck()
{
    size_t stopReel = 0;
    const std::vector<SymbolData>& symbolVect = lpPayTable->symbolVect;

    quickWinMask = lrMask = rlMask = 0; // default no wins
    if (lpPayTable)
        stopReel = lpPayTable->minWinSymbols;
    else
    {
        reportError("PayGrid:quickWinCheck", "No pay table set");
        return quickWinMask;
    }
    Symbol symId = SYMBOL_NONE;

    // check left_right
    lrMask = UINT32_MAX;
    for (int reel = 0; reel < stopReel; ++reel)
    {
        uint32_t reelMask = 0;
        for (int row = 0; row < reelHeightVect[reel]; ++row)
        {
            if ((symId = getReelSymbol(reel, row)) != SYMBOL_NONE)
                reelMask |= symbolVect[symId].payMask;
        }
        lrMask &= reelMask;
    }

    // check right_left
    stopReel = maxReels - lpPayTable->minWinSymbols;
    rlMask = UINT32_MAX;
    for (size_t reel = maxReels - 1; reel >= stopReel; --reel)
    {
        uint32_t reelMask = 0;
        for (int row = 0; row < reelHeightVect[reel]; ++row)
        {
            if ((symId = getReelSymbol(reel, row)) != SYMBOL_NONE)
                reelMask |= symbolVect[symId].payMask;
        }
        rlMask &= reelMask;
    }

    quickWinMask = lrMask | rlMask;

    return quickWinMask;
}

int BothWaysLineGrid::evaluateWins(const int betMultiplier, const bool payMaxOakTwice)
{
    int totalWin = 0;

    winVector.clear();
    quickWinCheck();
    if (lrMask)
        totalWin = lrWinCheck(betMultiplier);
    if (rlMask)
        totalWin += rlWinCheck(betMultiplier);

    if (!payMaxOakTwice && totalWin)
        totalWin -= removeMaxOakDuplicates();
    return totalWin;
}
