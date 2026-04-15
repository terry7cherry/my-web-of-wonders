// Precompiled header
#include "WaysWin.h"

#include "../headers/types.h"
#include "../grids/PayGrid.h"
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
    reelCountVect.assign(maxReels, 0);
    wildCountVect.assign(maxReels, 0);
    winPosVect.resize(maxReels);
    positions.reserve(maxReels * maxRows);
    multiplierForReel.assign(maxReels, 1);
    for (int i = 0; i < maxReels; i++)
        winPosVect[i].assign(maxRows, 0);
}

// Identifies a win as all wilds 
// Call once if 1st symbol is wild
void WaysWin::setWildWin(const bool isWild)
{
    isWildWin = isWild;
}

// Add new position (reel, row) for the winning symbol. 
// Parameter isWild identifies the position as a symbol or a wild
void WaysWin::addPosition(size_t reel, size_t row, const bool isWild, const unsigned int positionMultiplier,
                          const MultipliersCombineType combineType)
{
    positions.emplace_back(reel, row);
    reelCountVect[reel]++;
    if (isWild)
        wildCountVect[reel]++;

    const size_t winIndex = reel + (row * maxReels);
    winMask |= (static_cast<uint64>(1) << winIndex);
    winPosVect[reel][row] = isWild ? 2 : 1;
    if (positionMultiplier != 1 && multiplierForReel[reel] == 1)
    {
        multiplierForReel[reel] = positionMultiplier;
        multiplier = PayGrid::combineMultipliers(
            multiplier, positionMultiplier, combineType);
    }
}

// Get the apertures and wild state of all positions on a reel.
std::vector<std::pair<size_t, bool>> WaysWin::getPositionsOnReel(const size_t reel) const
{
    std::vector<std::pair<size_t, bool>> positionsOnReel;
    for (size_t row = 0; row < maxRows; row++)
    {
        if (winPosVect[reel][row])
            positionsOnReel.push_back({row, winPosVect[reel][row] == 2});
    }
    return positionsOnReel;
}

// Calculates the number of winning ways and returns that value
int WaysWin::setWays()
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

Json::Value WaysWin::toJson(const std::vector<std::string>& symbolNames) const
{
    Json::Value winAsJson = Win::toJson(symbolNames);
    winAsJson["positions"] = Json::Value(Json::arrayValue);
    for (const auto& [reel, row] : positions)
    {
        Json::Value position;
        position["reel"] = Json::Value(static_cast<Json::UInt64>(reel));
        position["row"] = Json::Value(static_cast<Json::UInt64>(row));
        winAsJson["positions"].append(position);
    }
    winAsJson["ways"] = ways;
    return winAsJson;
}
