#include "LineGrid.h"

size_t LineGrid::evaluateWins(const size_t betMultiplier, [[maybe_unused]] bool doMeters,
                              [[maybe_unused]] bool useDebugLine)
{
    size_t totalWin = 0;

    winVector.clear();

    if ((quickWinMask = quickWinCheck()))
    {
        int symbolToMatch;
        int startSymbol;
        size_t wildCount = 0;
        size_t consecutiveWilds;
        size_t number;
        size_t wildMultiplier;
        size_t lineMultiplier;
        const std::vector<SymbolData>& symbolVect = lpPayTable->symbolVect;
        for (int line = 0; line < lpPayLines->lineCount; ++line)
        {
            bool blnCheckLine;
            size_t row = lpPayLines->getRow(line, 0);
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
                row = lpPayLines->getRow(line, reel);
                int nextSym = reelSymbolVect[reel][row];
                const SymbolData& nextSymData = symbolVect[nextSym];

                if (nextSymData.isScatter && !nextSymData.isWild)
                    break;
                if (symbolVect[symbolToMatch].isWild || nextSymData.isWild
                    || nextSym == symbolToMatch)
                {
                    number++;
                    if (multiplierVect[reel][row] > 0)
                        lineMultiplier *= multiplierVect[reel][row];
                    if (symbolVect[symbolToMatch].isWild)
                    {
                        if (nextSymData.isWild)
                        {
                            consecutiveWilds++;
                            if (multiplierVect[reel][row] > 0)
                                wildMultiplier *= multiplierVect[reel][row];
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

            if (number >= lpPayTable->minPaySymbolsVect[symbolToMatch])
            {
                if (consecutiveWilds >= lpPayTable->minWinSymbols)
                {
                    // Wilds make a payout. If wins are equal pay more symbols in preference to less wilds
                    if ((wildMultiplier * lpPayTable->getWin(
                            startSymbol, consecutiveWilds)) >
                        (lineMultiplier * lpPayTable->getWin(
                            symbolToMatch, number)))
                    {
                        //  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
                        symbolToMatch = startSymbol;
                        number = consecutiveWilds;
                        lineMultiplier = wildMultiplier;
                    }
                }

                size_t win = betMultiplier * lpPayTable->getWin(
                    symbolToMatch, number);
                if (win)
                {
                    totalWin += (win * lineMultiplier);
                    //if (doMeters)
                    //	lpPayTable->meterWin(symbolToMatch, number);

                    // add additional line win data - possibly only needed for playable

                    LineWin lineWin;
                    lineWin.payLine = this->getPayLine(line);
                    lineWin.payLineNum = line;
                    lineWin.win = lineWin.totalWin = win;
                    if (lineMultiplier > 1)
                    {
                        lineWin.multiplier = lineMultiplier;
                        lineWin.totalWin *= lineMultiplier;
                    }
                    lineWin.oak = number;
                    lineWin.wildCount = wildCount;
                    lineWin.symbol = symbolToMatch;
                    lineWin.direction = WinDirections::left_right;
                    lineWin.winType = "line";
                    winVector.push_back(lineWin);
                }
            }
        }
    }

    for (size_t scatterSymbol : lpPayTable->scatterSymVect)
    {
        int symbolCount = this->getSymbolCount(scatterSymbol);
        if (symbolCount < lpPayTable->minPaySymbolsVect[scatterSymbol])
            continue;
        if (size_t win = betMultiplier * lpPayLines->lineCount * lpPayTable->
            getWin(scatterSymbol, symbolCount))
        {
            totalWin += win;
            //if (doMeters)
            //	lpPayTable->meterWin(scatterSymbol, symbolCount);

            LineWin scatterWin;
            scatterWin.win = scatterWin.totalWin = win;
            scatterWin.oak = symbolCount;
            scatterWin.symbol = scatterSymbol;
            scatterWin.winType = "scatter";
            winVector.push_back(scatterWin);
        }
    }
    return totalWin;
}

int LineGrid::setWinningSymbols()
{
    // check each active payline and set all winning positions in isWinVect
    for (LineWin& lineWin : winVector)
    {
        if (lineWin.winType == "scatter")
        {
            if (lineWin.direction == WinDirections::left_right)
            {
                for (int reel = 0; reel < lineWin.oak; reel++)
                    isWinVect[reel][(*lineWin.payLine)[reel]] = true;
            }
            else
            {
                for (size_t reel = maxReels - lineWin.oak; reel < maxReels; reel++)
                    isWinVect[reel][(*lineWin.payLine)[reel]] = true;
            }
        }
        else
        {
            const size_t& symbolId = lineWin.symbol;
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
        if ((multiplierVect[reel][(*lineWin.payLine)[reel]]) > 0)
            multiplierCount++;
    }
    return multiplierCount;
}

int LineGrid::countActiveWinMultipliers()
{
    std::vector<std::vector<bool>> activeMultiplierVect(maxReels);
    for (int reel = 0; reel < maxReels; reel++)
        activeMultiplierVect[reel].resize(maxRows, false);

    for (size_t i = 0; i < winVector.size(); i++)
    {
        LineWin& win = winVector[i];
        const size_t startReel = (win.direction == WinDirections::left_right)
                                     ? 0
                                     : maxReels - win.oak;
        const size_t endReel = (win.direction == WinDirections::left_right)
                                   ? win.oak
                                   : maxReels;
        for (size_t reel = startReel; multiplierVect.size() && reel < endReel;
             reel++)
        {
            const size_t row = lpPayLines->getRow(win.payLine, reel);
            if (multiplierVect[reel][row] > 0)
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

size_t LineGrid::applyGridMultiplier(const MultipliersCombineType combineType,
                                     const int specialMultiplier)
{
    const int applyMultiplier = specialMultiplier
                                    ? specialMultiplier
                                    : this->gridMultiplier;
    size_t totalWin = 0;
    if (applyMultiplier > 1)
    {
        for (size_t i = 0; i < winVector.size(); i++)
            totalWin += winVector[i].applyMultiplier(
                applyMultiplier, combineType);
    }
    return totalWin;
}

size_t LineGrid::applyWinMultipliers(const MultipliersCombineType combineType)
{
    size_t startReel = 0, endReel = 0;
    size_t totalWin = 0;
    for (LineWin& win : winVector)
    {
        size_t winMultiplier = win.multiplier;

        if (win.winType == "line")
        {
            startReel = 0;
            endReel = win.oak;
        }
        else if (win.direction == WinDirections::right_left)
        {
            startReel = maxReels - win.oak;
            endReel = maxReels;
        }

        for (size_t reel = startReel; reel < endReel; reel++)
        {
            const size_t cellMultiplier = multiplierVect[reel][lpPayLines->getRow(
                win.payLine, reel)];
            if (cellMultiplier > 0)
            {
                if (combineType == MultipliersCombineType::multiply)
                    winMultiplier *= cellMultiplier;
                else if (combineType == MultipliersCombineType::add)
                    winMultiplier += cellMultiplier;
            }
        }
        totalWin += (winMultiplier > 1)
                        ? win.applyMultiplier(winMultiplier, combineType)
                        : win.totalWin;
    }
    return totalWin;
}

int LineGrid::showWins(int startRow, const int col, const bool showTitle,
                       const bool showDirection) const
{
    if (!mTextMap.size())
    {
        reportError("LineGrid::showWins",
                    "Call 'Grid::setTextMap()' before using this method");
        return 0;
    }

    if (showTitle && winVector.size())
    {
        gameConsole.setCursorPos(startRow++, col);
        gameConsole.coutStyle(szWinLineHeader, CONSOLE_INVERT);
    }

    int row = startRow;
    for (int i = 0; i < lpPayLines->lineCount; i++)
    {
        // clear previous output
        gameConsole.setCursorPos(row++, col);
        std::cout << szBlankWin;
    }

    row = startRow;
    for (auto& win : winVector)
    {
        // output current win lines
        if (win.winType == "line")
        {
            if (win.payLine)
            {
                gameConsole.setCursorPos(row++, col);
                std::cout << win.getWinText(mTextMap, showDirection);
            }
        }
        // Returns after one win, is this correct?
        return row - startRow;
    }
    return 0;
}


std::vector<std::string> LineGrid::getWinTextVect(
    std::map<int, std::string>& symbolMap, bool showDirection) const
{
    std::vector<std::string> winTextVect;
    if (!mTextMap.size())
        reportError("LineGrid::getWinTextVect",
                    "Call 'Grid::setTextMap()' before using this method");
    else
        for (auto win : winVector)
            winTextVect.push_back(win.getWinText(mTextMap));
    return winTextVect;
}
