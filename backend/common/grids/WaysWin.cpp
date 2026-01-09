#include <algorithm>
#include <cstdio>   // for std::snprintf

// cpp-dev-library
#include "../headers/pch.h"
#include "WaysWin.h"

// basic constructor with no starting win positions
WaysWin::WaysWin(const uint8_t winSymbol, const int maxReelCount, const int maxRowCount)
    : maxReels(maxReelCount), maxRows(maxRowCount), symbol(winSymbol) // set member variables
{
    if (maxReels)
        resizeVectors();
}

// default constructor for creating arrays
WaysWin::WaysWin() : WaysWin(SYMBOL_NONE, 0, 0)
{
}

void WaysWin::resizeVectors(void)
{
    reelCountVect.resize(maxReels, 0);
    wildCountVect.resize(maxReels, 0);
    winPosVect.resize(maxReels);
    for (int i = 0; i < maxReels; i++)
        winPosVect[i].resize(maxRows, 0);
}

// Identifies a win as all wilds 
// Call once if 1st symbol is wild
void WaysWin::setWildWin(bool isWild)
{
    isWildWin = isWild;
}

// Add new position (reel, row) for the winning symbol. 
// Parameter isWild identifies the position as a symbol or a wild
void WaysWin::addPosition(int reel, int row, bool isWild)
{
    reelCountVect[reel]++;
    if (isWild)
        wildCountVect[reel]++;

    int winIndex = reel + (row * maxReels);
    winMask |= (static_cast<uint64>(1) << winIndex);
    winPosVect[reel][row] = isWild ? 2 : 1;
}

// Calculates the number of winning ways and returns that value
int WaysWin::setWays(void)
{
    winWays = wildWays = 1;
    for (oak = 0; oak < maxReels && reelCountVect[oak]; oak++)
    {
        wildWays *= wildCountVect[oak];
        winWays *= reelCountVect[oak];
    }
    if (!isWildWin)
        winWays -= wildWays;
    return winWays;
}

// Calculates the total win and returns it
int WaysWin::setWin(PayTable* lpPayTable, int betMultiplier)
{
    if (setWays() > 0)
    {
        winVal = lpPayTable->getWin(symbol, oak) * betMultiplier;
        if (winVal) totalWinVal = winVal * winWays * multiplier;
    }
    return totalWinVal;
}

// Generates text that describes the WayWin
// returns string in the format, "3 H1(100) x2 Ways [x2 multiplier ] = 400"
std::string WaysWin::getWinText(std::string szSymbol)
{
    char buffer[120];

    // remove spaces from symbol
    auto end_pos = std::remove(szSymbol.begin(), szSymbol.end(), ' ');
    szSymbol.erase(end_pos, szSymbol.end());

    int space = 120;
    int size = std::snprintf(buffer, space, "%d %s(%d) x%d Ways ",
                             oak, szSymbol.c_str(), winVal, winWays);
    if (size < 0) size = 0;
    if (size > space) size = space;
    space -= size;

    if (multiplier > 1 && space > 0)
    {
        int added = std::snprintf(&buffer[size], space, "x%d multiplier ", multiplier);
        if (added < 0) added = 0;
        if (added > space) added = space;
        size += added;
        space -= added;
    }

    if (space > 0)
    {
        int added = std::snprintf(&buffer[size], space, "= %d", totalWinVal);
        if (added < 0) added = 0;
        if (added > space) added = space;
        size += added;
        space -= added;
    }

    return std::string(buffer, size);
}

// Generates text that describes the WayWin
// returns string in the format, "3 H1(100) x2 ways [x2 multiplier ] = 400"
std::string WaysWin::getWinText(std::map<int, std::string>& symbolMap)
{
    std::string szSymbol = symbolMap[symbol];
    return getWinText(szSymbol);
}

