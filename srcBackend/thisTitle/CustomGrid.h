#pragma once

#include "../common/grids/PayAnywhereGrid.h"
#include "CollectorPositions.h"
#include "wins/CoinWin.h"

class CustomGrid : public PayAnywhereGrid
{
public:
    std::vector<std::vector<unsigned int>> coinValues;
    std::vector<std::vector<int>> coinMultiplierGrid{};
    CollectorPosition mainCollectorPosition{};
    unsigned int collectorMultipliers[4] = {0, 0, 0, 0};

    explicit CustomGrid(const GameMaths& maths)
        : PayAnywhereGrid(maths)
    {
        coinValues.resize(maxReels, std::vector<unsigned int>(maxRows));
    }

    void reset() override
    {
        PayAnywhereGrid::reset();
        resetCoinValues();
        for (size_t i = 0; i < 4; i++)
            collectorMultipliers[i] = 0;
    }

    void resetCoinValues()
    {
        for (size_t reel = 0; reel < maxReels; reel++)
            for (size_t row = 0; row < maxRows; row++)
                coinValues[reel][row] = 0;
    }

    void removeOneSymbol(const size_t reel, const size_t row) override
    {
        PayAnywhereGrid::removeOneSymbol(reel, row);
        coinValues[reel].erase(coinValues[reel].begin() + row);
    }

    void addSymbolsToTopOfOneReel(const size_t reel, const size_t numSymbols) override
    {
        PayAnywhereGrid::addSymbolsToTopOfOneReel(reel, numSymbols);
        coinValues[reel].insert(coinValues[reel].begin(), numSymbols, 0);
    }

    CollectorPosition getCollectorWithMultiplier(const unsigned int multiplier) const
    {
        for (size_t i = 0; i < 4; i++)
            if (collectorMultipliers[i] == multiplier)
                return static_cast<CollectorPosition>(i);
        throw std::runtime_error(
            "Tried to award coin with mutliplier value that does not match a collector.");
    }

    unsigned int evaluateCoinWins(const unsigned int betMultiplier)
    {
        unsigned int totalWin = 0;
        for (size_t reel = 0; reel < maxReels; reel++)
            for (size_t row = 0; row < maxRows; row++)
                if (coinValues[reel][row] > 0 && coinMultiplierGrid[reel][row] > 0)
                {
                    CoinWin coinWin;
                    coinWin.win = coinWin.totalWin = coinValues[reel][row] * betMultiplier;
                    coinWin.positions.emplace_back(reel, row);
                    coinWin.multiplier = coinMultiplierGrid[reel][row];
                    coinWin.collectorPosition = getCollectorWithMultiplier(coinWin.multiplier);
                    coinWin.totalWin *= coinMultiplierGrid[reel][row];
                    winVector.emplace_back(std::make_unique<CoinWin>(coinWin));
                    totalWin += coinWin.totalWin;
                }
        return totalWin;
    }
};
