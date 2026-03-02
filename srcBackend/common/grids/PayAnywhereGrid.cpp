#include "PayAnywhereGrid.h"
#include "../wins/ScatterWin.h"

unsigned int PayAnywhereGrid::evaluateWins(const unsigned int betMultiplier,
                                           const MultipliersCombineType combineType)
{
    unsigned int totalWin = 0;

    winVector.clear();

    unsigned int scatterWildCount = 0;
    for (const Symbol scatterWildSymbol : payTable.scatterWildSymVect)
        scatterWildCount += this->getSymbolCount(scatterWildSymbol);

    for (const Symbol scatterSymbol : payTable.scatterSymVect)
    {
        const unsigned int symbolCount = this->getSymbolCount(scatterSymbol) + scatterWildCount;
        if (symbolCount < payTable.minPaySymbolsVect[scatterSymbol])
            continue;
        if (const unsigned int win = betMultiplier * payTable.getWin(scatterSymbol, symbolCount))
        {
            ScatterWin scatterWin;
            scatterWin.win = win;
            unsigned int scatterMultiplier = 1;
            unsigned int wildMultiplier = 1;
            unsigned int scattersToFind = symbolCount;
            for (size_t reel = 0; reel < reelSymbolVect.size(); reel++)
            {
                for (size_t row = 0; row < reelSymbolVect[reel].size(); row++)
                    if (reelSymbolVect[reel][row] == scatterSymbol || payTable.isScatterWild(
                        reelSymbolVect[reel][row]))
                    {
                        scattersToFind--;
                        scatterWin.positions.emplace_back(reel, row);
                        scatterMultiplier = combineMultipliers(scatterMultiplier, multiplierVect[reel][row],
                                                               combineType);
                        if (payTable.isScatterWild(reelSymbolVect[reel][row]))
                            wildMultiplier = combineMultipliers(wildMultiplier, multiplierVect[reel][row],
                                                                combineType);
                        if (scattersToFind == 0) break;
                    }
                if (scattersToFind == 0) break;
            }

            scatterWin.totalWin = win * scatterMultiplier;
            scatterWin.oak = symbolCount;
            scatterWin.symbol = scatterSymbol;
            scatterWin.winType = "scatter";

            totalWin += scatterWin.totalWin;
            winVector.emplace_back(std::make_unique<ScatterWin>(scatterWin));
        }
    }
    return totalWin;
}

int PayAnywhereGrid::setWinningSymbols()
{
    // check each active payline and set all winning positions in isWinVect
    for (const auto& winPtr : winVector)
    {
        const Win& win = *winPtr;
        for (const auto& [reel, row] : win.positions)
            isWinVect[reel][row] = true;
    }
    winIndexVect.clear();
    int index = 0;
    for (int row = 0; row < maxRows; row++)
    {
        for (int reel = 0; reel < maxReels; reel++, index++)
        {
            if (isWinVect[reel][row])
                winIndexVect.push_back(index);
        }
    }

    return static_cast<int>(winIndexVect.size());
}

int PayAnywhereGrid::countWinMultipliers(const ScatterWin& scatterWin) const
{
    int multiplierCount = 0;

    for (const auto& [reel, row] : scatterWin.positions)
        if (multiplierVect[reel][row] > 1)
            multiplierCount++;

    return multiplierCount;
}

int PayAnywhereGrid::countActiveWinMultipliers() const
{
    std::vector<std::vector<bool>> activeMultiplierVect(maxReels);
    for (int reel = 0; reel < maxReels; reel++)
        activeMultiplierVect[reel].resize(maxRows, false);

    for (const auto& i : winVector)
    {
        if (i->winType != "line") continue;
        const auto& lineWin = dynamic_cast<const LineWin&>(*i);
        const size_t startReel = (lineWin.direction == WinDirections::left_right)
                                     ? 0
                                     : maxReels - lineWin.oak;
        const size_t endReel = (lineWin.direction == WinDirections::left_right)
                                   ? lineWin.oak
                                   : maxReels;
        for (size_t reel = startReel; !multiplierVect.empty() && reel < endReel;
             reel++)
        {
            const size_t row = PayLines::getRow(lineWin.payLine, reel);
            if (multiplierVect[reel][row] > 1)
                activeMultiplierVect[reel][row] = true;
        }
    }

    int activeCount = 0;
    for (int reel = 0; reel < maxReels; reel++)
    {
        for (int row = 0; row < maxRows; row++)
        {
            if (activeMultiplierVect[reel][row])
                activeCount++;
        }
    }

    return activeCount;
}

unsigned int PayAnywhereGrid::applyWinMultipliers(const MultipliersCombineType combineType)
{
    size_t startReel = 0, endReel = 0;
    unsigned int totalWin = 0;
    for (const auto& win : winVector)
    {
        if (win->winType != "line") continue;
        auto& lineWin = dynamic_cast<LineWin&>(*win);
        unsigned int winMultiplier = lineWin.multiplier;

        if (lineWin.direction == WinDirections::right_left)
        {
            startReel = maxReels - lineWin.oak;
            endReel = maxReels;
        }
        else
        {
            startReel = 0;
            endReel = lineWin.oak;
        }

        for (size_t reel = startReel; reel < endReel; reel++)
        {
            const unsigned int cellMultiplier = multiplierVect[reel][PayLines::getRow(
                lineWin.payLine, reel)];
            winMultiplier = combineMultipliers(winMultiplier, cellMultiplier, combineType);
        }
        totalWin += (winMultiplier > 1)
                        ? lineWin.applyMultiplier(winMultiplier, combineType)
                        : lineWin.totalWin;
    }
    return totalWin;
}

