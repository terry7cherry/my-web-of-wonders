// Precompiled header
#include "WaysWin.h"

#include "../headers/types.h"
#include <cstdio>   // for std::snprintf

// basic constructor with no starting win positions
WaysWin::WaysWin(const Symbol winSymbol, const size_t maxReelCount, const size_t maxRowCount)
    : maxReels(maxReelCount), maxRows(maxRowCount)
{
    symbol = winSymbol;
    winType = "ways";
    if (maxReels)
        resizeVectors();
}

// default constructor for creating arrays
WaysWin::WaysWin() : WaysWin(SYMBOL_NONE, 0, 0)
{
    winType = "ways";
}

void WaysWin::resizeVectors(void)
{
    reelCountVect.resize(maxReels, 0);
    wildCountVect.resize(maxReels, 0);
    winPosVect.resize(maxReels);
    positions.reserve(maxReels * maxRows);
    for (int i = 0; i < maxReels; i++)
        winPosVect[i].resize(maxRows, 0);
}

// Identifies a win as all wilds 
// Call once if 1st symbol is wild
void WaysWin::setWildWin(const bool isWild)
{
    isWildWin = isWild;
}

// Add new position (reel, row) for the winning symbol. 
// Parameter isWild identifies the position as a symbol or a wild
void WaysWin::addPosition(size_t reel, size_t row, bool isWild)
{
    positions.emplace_back(reel, row);
    reelCountVect[reel]++;
    if (isWild)
        wildCountVect[reel]++;

    const size_t winIndex = reel + (row * maxReels);
    winMask |= (static_cast<uint64>(1) << winIndex);
    winPosVect[reel][row] = isWild ? 2 : 1;
}

// Calculates the number of winning ways and returns that value
int WaysWin::setWays(void)
{
    ways = wildWays = 1;
    for (oak = 0; oak < maxReels && reelCountVect[oak]; oak++)
    {
        wildWays *= wildCountVect[oak];
        ways *= reelCountVect[oak];
    }
    if (!isWildWin)
        ways -= wildWays;
    return ways;
}

// Calculates the total win and returns it
int WaysWin::setWin(const PayTable& payTable, const int betMultiplier)
{
    if (setWays() > 0)
    {
        win = payTable.getWin(symbol, oak) * betMultiplier;
        if (win) totalWin = win * ways * multiplier;
    }
    return totalWin;
}

// Generates text that describes the WayWin
// returns string in the format, "3 H1(100) x2 Ways [x2 multiplier ] = 400"
std::string WaysWin::getWinText(std::string szSymbol) const
{
    char buffer[120];

    // remove spaces from symbol
    const auto end_pos = std::ranges::remove(szSymbol, ' ').begin();
    szSymbol.erase(end_pos, szSymbol.end());

    int space = 120;
    int size = std::snprintf(buffer, space, "%d %s(%d) x%d Ways ",
                             oak, szSymbol.c_str(), win, ways);
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
        int added = std::snprintf(&buffer[size], space, "= %d", totalWin);
        if (added < 0) added = 0;
        if (added > space) added = space;
        size += added;
        space -= added;
    }

    return std::string(buffer, size);
}

// Generates text that describes the WayWin
// returns string in the format, "3 H1(100) x2 ways [x2 multiplier ] = 400"
std::string WaysWin::getWinText(const std::map<int, std::string>& symbolMap) const
{
    const std::string szSymbol = symbolMap.at(symbol);
    return getWinText(szSymbol);
}

Json::Value WaysWin::toJson(const std::vector<std::string>& symbolNames) const
{
    Json::Value winAsJson = Win::toJson(symbolNames);
    winAsJson["positions"] = Json::Value(Json::arrayValue);
    for (const auto& [reel, row] : positions)
    {
        Json::Value position;
        position["reel"] = reel;
        position["row"] = row;
        winAsJson["positions"].append(position);
    }
    return winAsJson;
}
