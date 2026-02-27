#include "LineGrid.h"
#include "../wins/LineWin.h"
#include "../wins/ScatterWin.h"

unsigned int LineGrid::evaluateWins(const unsigned int betMultiplier,
                                    const MultipliersCombineType combineType)
{
    unsigned int totalWin = 0;

    winVector.clear();

    if ((quickWinMask = quickWinCheck()))
    {
        Symbol symbolToMatch;
        Symbol startSymbol;
        unsigned int wildCount = 0;
        unsigned int consecutiveWilds;
        unsigned int number;
        unsigned int wildMultiplier;
        unsigned int lineMultiplier;
        const std::vector<SymbolData>& symbolVect = payTable.symbolVect;
        for (int line = 0; line < payLines.lineCount; ++line)
        {
            bool blnCheckLine;
            size_t row = payLines.getRow(line, 0);
            startSymbol = symbolToMatch = reelSymbolVect[0][row];
            const SymbolData& startSymData = symbolVect[startSymbol];

            if (startSymData.isScatter && !startSymData.isWild)
                blnCheckLine = false;
            else
                blnCheckLine = startSymData.isWild || (quickWinMask &
                    startSymData.mask);

            if (!blnCheckLine)
                continue;
            // based on 1st symbol there is no need to check this line

            number = consecutiveWilds = wildCount = 0;
            wildMultiplier = lineMultiplier = 1;
            for (int reel = 0; reel < maxReels; reel++)
            {
                // start from beginning again to account for multipliers
                row = payLines.getRow(line, reel);
                Symbol nextSym = reelSymbolVect[reel][row];
                const SymbolData& nextSymData = symbolVect[nextSym];

                if (nextSymData.isScatter && !nextSymData.isWild)
                    break;
                if (symbolVect[symbolToMatch].isWild || nextSymData.isWild
                    || nextSym == symbolToMatch)
                {
                    number++;
                    lineMultiplier = combineMultipliers(lineMultiplier, multiplierVect[reel][row],
                                                        combineType);
                    if (symbolVect[symbolToMatch].isWild)
                    {
                        if (nextSymData.isWild)
                        {
                            consecutiveWilds++;
                            wildMultiplier = combineMultipliers(wildMultiplier, multiplierVect[reel][row],
                                                                combineType);
                        }
                        else
                            symbolToMatch = nextSym;
                    }
                    if (nextSymData.isWild)
                        wildCount++;
                }
                else
                    break;
            }

            if (number >= payTable.minPaySymbolsVect[symbolToMatch])
            {
                if (consecutiveWilds >= payTable.minWinSymbols)
                {
                    // Wilds make a payout. If wins are equal pay more symbols in preference to fewer wilds
                    if ((wildMultiplier * payTable.getWin(
                            startSymbol, consecutiveWilds)) >
                        (lineMultiplier * payTable.getWin(
                            symbolToMatch, number)))
                    {
                        //  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
                        symbolToMatch = startSymbol;
                        number = consecutiveWilds;
                        lineMultiplier = wildMultiplier;
                    }
                }

                if (unsigned int win = betMultiplier * payTable.getWin(symbolToMatch, number))
                {
                    totalWin += win * lineMultiplier;

                    LineWin lineWin;
                    lineWin.payLine = this->getPayLine(line);
                    lineWin.payLineNum = line;
                    lineWin.win = lineWin.totalWin = win;
                    if (lineMultiplier > 1)
                    {
                        lineWin.multiplier = lineMultiplier;
                        lineWin.totalWin = lineWin.totalWin * lineMultiplier;
                    }
                    lineWin.oak = number;
                    lineWin.wildCount = wildCount;
                    lineWin.symbol = symbolToMatch;
                    lineWin.setPositions();
                    lineWin.direction = WinDirections::left_right;
                    lineWin.winType = "line";
                    winVector.emplace_back(std::make_unique<LineWin>(lineWin));
                }
            }
        }
    }

    for (Symbol scatterSymbol : payTable.scatterSymVect)
    {
        unsigned int symbolCount = this->getSymbolCount(scatterSymbol);
        if (symbolCount < payTable.minPaySymbolsVect[scatterSymbol])
            continue;
        if (unsigned int win = betMultiplier * payTable.getWin(scatterSymbol, symbolCount))
        {
            totalWin += win;

            ScatterWin scatterWin;
            scatterWin.win = scatterWin.totalWin = win;
            scatterWin.oak = symbolCount;
            scatterWin.symbol = scatterSymbol;
            scatterWin.winType = "scatter";

            unsigned int scattersToFind = symbolCount;
            for (size_t reel = 0; reel < reelSymbolVect.size(); reel++)
            {
                for (size_t row = 0; row < reelSymbolVect[reel].size(); row++)
                    if (reelSymbolVect[reel][row] == scatterSymbol)
                    {
                        scattersToFind--;
                        scatterWin.positions.emplace_back(reel, row);
                        if (scattersToFind == 0) break;
                    }
                if (scattersToFind == 0) break;
            }
            winVector.emplace_back(std::make_unique<ScatterWin>(scatterWin));
        }
    }
    return totalWin;
}

int LineGrid::setWinningSymbols()
{
    // check each active payline and set all winning positions in isWinVect
    for (const auto& winPtr : winVector)
    {
        const Win& win = *winPtr;
        if (win.winType == "line")
        {
            const auto& lineWin = dynamic_cast<const LineWin&>(win);
            if (lineWin.direction == WinDirections::left_right)
            {
                for (size_t reel = 0; reel < win.oak; reel++)
                    isWinVect[reel][(*lineWin.payLine)[reel]] = true;
            }
            else
            {
                for (size_t reel = maxReels - win.oak; reel < maxReels; reel++)
                    isWinVect[reel][(*lineWin.payLine)[reel]] = true;
            }
        }
        else
        {
            const size_t& symbolId = win.symbol;
            int index = 0;
            for (int row = 0; row < maxRows; row++)
            {
                for (int reel = 0; reel < maxReels; reel++, index++)
                {
                    if (this->getSymbol(index) == symbolId)
                    {
                        isWinVect[reel][row] = true;
                    }
                }
            }
        }
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

int LineGrid::countWinMultipliers(const LineWin& lineWin) const
{
    int multiplierCount = 0;

    // set reels for win.direction == WinDirections::left_right
    size_t startReel = 0, endReel = lineWin.oak;

    if (lineWin.direction == WinDirections::right_left)
    {
        startReel = maxReels - lineWin.oak;
        endReel = maxReels;
    }

    for (size_t reel = startReel; reel < endReel; reel++)
    {
        if ((multiplierVect[reel][(*lineWin.payLine)[reel]]) > 1)
            multiplierCount++;
    }
    return multiplierCount;
}

int LineGrid::countActiveWinMultipliers() const
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

unsigned int LineGrid::applyGridMultiplier(const MultipliersCombineType combineType,
                                           const std::optional<int> specialMultiplier)
{
    const int applyMultiplier = (specialMultiplier.has_value())
                                    ? specialMultiplier.value()
                                    : this->gridMultiplier;
    unsigned int totalWin = 0;
    if (applyMultiplier > 1)
    {
        for (const auto& i : winVector)
            totalWin += i->applyMultiplier(applyMultiplier, combineType);
    }
    return totalWin;
}

unsigned int LineGrid::applyWinMultipliers(const MultipliersCombineType combineType)
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

