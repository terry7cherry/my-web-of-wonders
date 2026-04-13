#pragma once

#include "GameLogic.h"
#include "state/GameCycle.h"
#include "forces/simForces.h"
#include "../common/spin/SpinResult.h"
#include "../common/events/BaseGameCompleteEvent.h"
#include "../common/events/GameCycleCompleteEvent.h"
#include "../common/events/WinAnalysisEvent.h"
#include "../common/events/GridUpdateEvent.h"
#include "../common/events/SpinCompleteEvent.h"
#include "../common/events/FreeSpinsTriggerEvent.h"
#include "../common/events/FreeSpinsRetriggerEvent.h"
#include "./events/ReelSpinWithCoinsEvent.h"
#include "./events/BonusSymbolCollectionEvent.h"
#include "events/CascadeWithCoinsEvent.h"
#include "./events/CoinCollectionEvent.h"
#include "./events/FreeSpinsCollectionTriggerEvent.h"
#include "./events/CollectorActivationEvent.h"
#include "./events/BaseCoinsPhaseCompleteEvent.h"
#include "./events/BasePayAnywherePhaseCompleteEvent.h"
#include "CollectorPositions.h"
#include "../common/events/FreeSpinsCompleteEvent.h"
#include "../common/events/NewFreeSpinEvent.h"
#include "events/CollectorAwardedBonusEvent.h"
#include "events/EnhancedFreeSpins1CompleteEvent.h"
#include "events/EnhancedFreeSpins2CompleteEvent.h"
#include "events/EnhancedFreeSpins3CompleteEvent.h"
#include "events/EnhancedFreeSpinsCompleteEvent.h"
#include "events/FreeSpinsCoinsPayoutEvent.h"
#include "events/FreeSpinsPayAnywherePhaseCompleteEvent.h"
#include "events/StandardFreeSpinsCompleteEvent.h"
#include "events/MaxedOutMultipliersEvent.h"

namespace GameLogic
{
    unsigned int playGame(const GameMaths& maths, GameCycle& cycle)
    {
        updateMainCollectorPosition(maths, cycle);

        if (cycle.session.simForce >= Forces::ForceStandardFreeSpins
            && cycle.session.simForce <= Forces::ForceEnhancedFreeSpins3)
            handleFreeSpinsForces(maths, cycle);
        else
        {
            cycle.totalWin += doBaseGame(maths, cycle);
            unsigned int fsSymbolsToTrigger = maths.ints["freeSpinsSymbolsToTrigger"];
            cycle.fsTrigger = cycle.baseNumScatters == fsSymbolsToTrigger;
            if (!cycle.fsTrigger && cycle.baseNumScatters > 0)
                collectBonusSymbols(maths, cycle);
        }

        cycle.recordEvent<BaseGameCompleteEvent>(cycle.baseGameWin);


        if (cycle.fsTrigger)
        {
            cycle.totalWin += doFreeSpins(maths, cycle);
            recordFreeSpinsCompleteEvents(maths, cycle);
        }

        cycle.recordEvent<GameCycleCompleteEvent>(cycle.totalWin, maths, cycle.session.bet);
        updateDeadSpinsAndPositionsWithCoins(maths, cycle);
        return cycle.totalWin;
    }

    void updateMainCollectorPosition(const GameMaths& maths, GameCycle& cycle)
    {
        if (cycle.session.deadSpins >= 2)
        {
            const auto& positionsWithCoins = cycle.session.positionsWithCoinsOrBonuses;
            if (positionsWithCoins.size() == 1)
                cycle.session.mainCollectorPosition = positionsWithCoins.
                    at(0);
            else if (positionsWithCoins.size() > 1)
                cycle.session.mainCollectorPosition = positionsWithCoins[
                    mathsRng.getInt(positionsWithCoins.size(), "Choose main collector position.")];
            else
            {
                CollectorPosition newPosition = cycle.session.mainCollectorPosition;
                while (newPosition == cycle.session.mainCollectorPosition)
                    newPosition = static_cast<CollectorPosition>(mathsRng.getInt(
                        4, "Choose main collector position."));
                cycle.session.mainCollectorPosition = newPosition;
            }
            cycle.mainCollectorMoved = true;
            cycle.session.deadSpins = 0;
        }
        //cycle.session.mainCollectorPositionSpinRemaining--;
        cycle.grid.coinMultiplierGrid = maths.intGrids["collectorPositions." + collectorPositionNames[cycle.
                session.mainCollectorPosition]
        ];
        cycle.grid.mainCollectorPosition = cycle.session.mainCollectorPosition;
        cycle.grid.collectorMultipliers[cycle.session.mainCollectorPosition] = 1;
    }


    unsigned int doBaseGame(const GameMaths& maths, GameCycle& cycle)
    {
        spinReelsAndSetCoinValues(maths, cycle, "baseGame");

        const Symbol fsTriggerSymbol = maths.symbols["freeSpinsTrigger"];
        cycle.baseNumScatters = cycle.grid.getSymbolCount(fsTriggerSymbol);

        cycle.baseGameWin = analyseAndCascadeIfWin(maths, cycle);
        return cycle.baseGameWin;
    }

    unsigned int doFreeSpins(const GameMaths& maths, GameCycle& cycle)
    {
        const unsigned int fsInitialSpins = maths.ints["freeSpinsInitialSpins"];
        cycle.fsInitialSpins = fsInitialSpins;
        cycle.fsSpinsRemaining = cycle.fsInitialSpins;

        if (cycle.fsCollectorTrigger)
            cycle.recordEvent<FreeSpinsCollectionTriggerEvent>(
                cycle.fsInitialSpins, cycle.fsInitialSecondaryCollectors);
        else cycle.recordEvent<FreeSpinsTriggerEvent>(cycle.fsInitialSpins);


        cycle.fsCollectorsActivated[cycle.session.mainCollectorPosition] = true;
        setMainCollectorMultiplier(maths, cycle);
        activateInitialCollectors(maths, cycle);

        for (cycle.fsSpinsPlayed = 0;
             cycle.fsSpinsPlayed < cycle.fsInitialSpins + cycle.fsRetriggerSpins;
             cycle.fsSpinsPlayed++)
        {
            cycle.freeSpinsWin += doOneFreeSpin(maths, cycle);
            cycle.fsSpinsRemaining--;
        }

        return cycle.freeSpinsWin;
    }

    void activateInitialCollectors(const GameMaths& maths, GameCycle& cycle)
    {
        std::vector<CollectorPosition> candidateCollectors;
        for (const CollectorPosition& position : collectorPositionMap | std::views::values)
            if (cycle.session.mainCollectorPosition != position)
                candidateCollectors.push_back(position);

        for (size_t i = 0; i < cycle.fsInitialSecondaryCollectors; i++)
        {
            const CollectorPosition position = candidateCollectors.size() == 1
                                                   ? candidateCollectors.at(0)
                                                   : candidateCollectors[mathsRng.getInt(
                                                       candidateCollectors.size(),
                                                       "Choose starting collector to activate.")];
            activateCollector(maths, cycle, position);
            std::erase(candidateCollectors, position);
        }
    }

    unsigned int doOneFreeSpin(const GameMaths& maths, GameCycle& cycle)
    {
        cycle.grid.resetCoinValues();
        cycle.recordEvent<NewFreeSpinEvent>();
        spinReelsAndSetCoinValues(maths, cycle, "freeSpins");
        const unsigned int spinWin = analyseAndCascadeIfWin(maths, cycle);
        collectRemainingCoins(maths, cycle);
        cycle.recordEvent<SpinCompleteEvent>(spinWin);
        return spinWin;
    }


    void spinReelsAndSetCoinValues(const GameMaths& maths, GameCycle& cycle,
                                   const std::string& weightTableName)
    {
        if (cycle.session.simForce >= Forces::ForceBaseGameA && weightTableName == "baseGame")
            cycle.grid.applySpin(
                maths.reelSetWeights[weightTableName].reelSetVect[cycle.session.simForce -
                    Forces::ForceBaseGameA]);
        else
            cycle.grid.applySpin(maths.reelSetWeights[weightTableName]);
        setCoinValues(maths, cycle);
        cycle.recordEvent<ReelSpinWithCoinsEvent>(cycle.grid, maths, cycle.session.betMultiplier);
    }

    void collectBonusSymbols(const GameMaths& maths, GameCycle& cycle)
    {
        const Symbol freeSpinsTriggerSymbol = maths.symbols["freeSpinsTrigger"];
        const auto& collectorTriggerChance = maths.boolWeightTables["collectorTriggeredBonusChance"];
        const auto& fsInitialSecondaryCollectorWeights = maths.intWeightTables[
            "fsInitialSecondaryCollectors"];
        for (size_t reel = 0; reel < cycle.grid.maxReels; reel++)
            for (size_t row = 0; row < cycle.grid.maxRows; row++)
                if (cycle.grid.getSymbol({reel, row}) == freeSpinsTriggerSymbol
                    && cycle.grid.coinMultiplierGrid[reel][row] > 0)
                {
                    cycle.recordEvent<BonusSymbolCollectionEvent>(reel, row, cycle.bonusSymbolCollected);
                    cycle.bonusSymbolCollected = true;
                    if (collectorTriggerChance.pick())
                    {
                        cycle.fsTrigger = true;
                        cycle.fsCollectorTrigger = true;
                        cycle.fsInitialSecondaryCollectors = fsInitialSecondaryCollectorWeights.pick();
                        cycle.recordEvent<CollectorAwardedBonusEvent>();
                        return;
                    }
                }
    }


    unsigned int analyseAndCascadeIfWin(const GameMaths& maths, GameCycle& cycle)
    {
        unsigned int totalWin = 0;
        const unsigned int cascadeWin = analysePayAnywhereAndCoinWinnings(maths, cycle);
        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, maths, "base game");
        totalWin += cascadeWin;

        if (cascadeWin > 0)
        {
            const auto& removedSymbols = cycle.grid.cascadeWins(cycle.grid.winVector);
            removeExcessScatters(maths, cycle, removedSymbols);
            setCoinValues(maths, cycle);
            cycle.recordEvent<CascadeWithCoinsEvent>(removedSymbols, cycle.grid, maths,
                                                     cycle.session.betMultiplier);

            totalWin += analyseAndCascadeIfWin(maths, cycle);
        }
        return totalWin;
    }

    void setMainCollectorMultiplier(const GameMaths& maths, const GameCycle& cycle)
    {
        cycle.grid.collectorMultipliers[cycle.session.mainCollectorPosition] = 2;
        for (auto& reel : cycle.grid.coinMultiplierGrid)
            for (auto& row : reel)
                if (row > 0) row = 2;
    }

    void collectRemainingCoins(const GameMaths& maths, GameCycle& cycle)
    {
        bool collectorHasCollected[4] = {};
        for (size_t reel = 0; reel < cycle.grid.maxReels; reel++)
            for (size_t row = 0; row < cycle.grid.maxRows; row++)
                if (cycle.grid.coinValues[reel][row] > 0)
                    collectCoin(maths, cycle, reel, row, collectorHasCollected);

        const bool veryHighChance = cycle.fsSpinsRemaining <= 2 && cycle.fsFinalSecondaryCollectors == 0;
        const bool highChance = cycle.fsSpinsRemaining <= 4 && cycle.fsFinalSecondaryCollectors == 0;
        const auto& collectorActivationChance = maths.boolWeightTables[
            veryHighChance
                ? "collectorActivationChanceVeryHigh"
                : (highChance ? "collectorActivationChanceHigh" : "collectorActivationChance")];
        for (const CollectorPosition position : collectorPositionMap | std::views::values)
            if (collectorHasCollected[position])
                if (collectorActivationChance.pick())
                    activateCollector(maths, cycle, position);
    }

    void setCoinValues(const GameMaths& maths, GameCycle& cycle)
    {
        const Symbol coinSymbol = maths.symbols["coin"];
        const auto& coinValueWeights = maths.intWeightTables["coinValue"];
        for (size_t reel = 0; reel < cycle.grid.maxReels; reel++)
            for (size_t row = 0; row < cycle.grid.maxRows; row++)
                if (cycle.grid.getSymbol({reel, row}) == coinSymbol)
                    if (cycle.grid.coinValues[reel][row] == 0)
                        cycle.grid.coinValues[reel][row] = coinValueWeights.pick();
    }

    unsigned int analysePayAnywhereAndCoinWinnings(const GameMaths& maths, GameCycle& cycle)
    {
        const unsigned int payAnywhereWins = cycle.grid.evaluateWins(cycle.session.betMultiplier);
        if (payAnywhereWins > 0)
        {
            if (cycle.fsTrigger)
            {
                cycle.recordEvent<FreeSpinsPayAnywherePhaseCompleteEvent>(
                    payAnywhereWins, cycle.freeSpinsPayAnywhereRecorded);
                cycle.freeSpinsPayAnywhereRecorded = true;
            }
            else
            {
                cycle.recordEvent<BasePayAnywherePhaseCompleteEvent>(
                    payAnywhereWins, cycle.basePayAnywhereRecorded);
                cycle.basePayAnywhereRecorded = true;
            }
        }
        const auto [totalCoinWin, coinWins] = cycle.grid.evaluateCoinWins(
            cycle.session.betMultiplier);
        if (!cycle.fsTrigger && totalCoinWin > 0)
        {
            cycle.recordEvent<BaseCoinsPhaseCompleteEvent>(totalCoinWin, cycle.baseCoinsRecorded);
            cycle.baseCoinsRecorded = true;
        }

        for (const auto& [coinWin, multiplier] : coinWins)
            for (size_t i = 0; i < multiplier; i++)
            {
                if (!cycle.fsTrigger)
                {
                    cycle.recordEvent<BaseCoinsPayoutEvent>(coinWin, cycle.baseSingleCoinPayoutRecorded);
                    cycle.baseSingleCoinPayoutRecorded = true;
                }
                else
                {
                    cycle.recordEvent<FreeSpinsCoinsPayoutEvent>(
                        coinWin, cycle.freeSpinsSingleCoinPayoutRecorded);
                    cycle.freeSpinsSingleCoinPayoutRecorded = true;
                }
            }

        return payAnywhereWins + totalCoinWin;
    }

    void collectCoin(const GameMaths& maths, GameCycle& cycle, size_t reel, size_t row,
                     bool* collectorHasCollected)
    {
        std::vector<CollectorPosition> candidateInactiveCollectors;
        for (const CollectorPosition& position : collectorPositionMap | std::views::values)
            if (cycle.grid.collectorMultipliers[position] == 0
                && maths.intGrids[
                    "collectorPositions." + collectorPositionNames[position]
                ][reel][row] == 1
            )
                candidateInactiveCollectors.push_back(position);

        if (candidateInactiveCollectors.empty())
            return;

        const CollectorPosition& collector = candidateInactiveCollectors.size() == 1
                                                 ? candidateInactiveCollectors.at(0)
                                                 : candidateInactiveCollectors[mathsRng.getInt(
                                                     candidateInactiveCollectors.size(),
                                                     "Choose inactive collector to collect coin.")];

        cycle.recordEvent<CoinCollectionEvent>(reel, row, collector);
        collectorHasCollected[collector] = true;
    }

    void activateCollector(const GameMaths& maths, GameCycle& cycle, CollectorPosition collectorPosition)
    {
        for (auto& multiplier : cycle.grid.collectorMultipliers)
            if (multiplier > 0) multiplier++;
        cycle.grid.collectorMultipliers[collectorPosition] = 1;
        cycle.fsCollectorsActivated[collectorPosition] = true;

        cycle.recordEvent<CollectorActivationEvent>(collectorPosition, cycle.grid.collectorMultipliers);

        const unsigned int numRetriggerSpins = maths.ints["collectorActivationRetriggerSpins"];
        cycle.fsRetriggerSpins += numRetriggerSpins;
        cycle.fsSpinsRemaining += numRetriggerSpins;
        cycle.recordEvent<FreeSpinsRetriggerEvent>(numRetriggerSpins);

        const auto& collectorControlArea =
            maths.intGrids["collectorPositions." + collectorPositionNames[collectorPosition]];


        for (size_t reel = 0; reel < cycle.grid.maxReels; reel++)
            for (size_t row = 0; row < cycle.grid.maxRows; row++)
            {
                auto& multiplier = cycle.grid.coinMultiplierGrid[reel][row];
                if (multiplier > 0) multiplier++;
                else multiplier = collectorControlArea[reel][row];
            }

        cycle.fsFinalSecondaryCollectors++;
        maximizeMultipliersForEnhancedFreeSpins(maths, cycle);
    }


    void removeExcessScatters(const GameMaths& maths, GameCycle& cycle,
                              const std::vector<std::pair<size_t, size_t>>& removedSymbols)
    {
        const int freeSpinsSymbolsToTrigger = maths.ints["freeSpinsSymbolsToTrigger"];
        const Symbol freeSpinsTriggerSymbol = maths.symbols["freeSpinsTrigger"];

        std::vector<std::pair<size_t, size_t>> excessScatterPositions;

        std::vector<size_t> removedSymbolsPerReel;
        removedSymbolsPerReel.resize(cycle.grid.maxReels);

        for (const auto& reel : removedSymbols | std::views::keys)
            removedSymbolsPerReel[reel]++;

        for (size_t reel = 0; reel < cycle.grid.maxReels; reel++)
            for (size_t row = 0; row < removedSymbolsPerReel[reel]; row++)
                if (cycle.grid.getSymbol({reel, row}) == freeSpinsTriggerSymbol)
                {
                    if (cycle.baseNumScatters == freeSpinsSymbolsToTrigger)
                        excessScatterPositions.push_back({reel, row});
                    else
                        cycle.baseNumScatters++;
                }

        if (excessScatterPositions.size() > 0)
        {
            cycle.grid.cascadeSymbols(excessScatterPositions);
            removeExcessScatters(maths, cycle, excessScatterPositions);
        }
    }

    void recordFreeSpinsCompleteEvents(const GameMaths& maths, GameCycle& cycle)
    {
        cycle.recordEvent<FreeSpinsCompleteEvent>(cycle.freeSpinsWin, cycle.fsInitialSpins,
                                                  cycle.fsSpinsPlayed);
        if (cycle.fsCollectorTrigger)
        {
            cycle.recordEvent<EnhancedFreeSpinsCompleteEvent>(cycle.freeSpinsWin, cycle.fsInitialSpins,
                                                              cycle.fsSpinsPlayed);
            switch (cycle.fsInitialSecondaryCollectors)
            {
            case 1: cycle.recordEvent<EnhancedFreeSpins1CompleteEvent>(
                    cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed);
                break;
            case 2: cycle.recordEvent<EnhancedFreeSpins2CompleteEvent>(
                    cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed);
                break;
            case 3: cycle.recordEvent<EnhancedFreeSpins3CompleteEvent>(
                    cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed);
                break;
            default: break;
            }
        }
        else
            cycle.recordEvent<StandardFreeSpinsCompleteEvent>(cycle.freeSpinsWin, cycle.fsInitialSpins,
                                                              cycle.fsSpinsPlayed,
                                                              cycle.fsFinalSecondaryCollectors);
    }

    void handleFreeSpinsForces(const GameMaths& maths, GameCycle& cycle)
    {
        const auto& simForce = cycle.session.simForce;
        switch (simForce)
        {
        case Forces::ForceStandardFreeSpins:
            cycle.baseNumScatters = 3;
            cycle.fsTrigger = true;
            break;
        case Forces::ForceEnhancedFreeSpins1:
            cycle.fsCollectorTrigger = true;
            cycle.fsInitialSecondaryCollectors = 1;
            cycle.fsTrigger = true;
            break;
        case Forces::ForceEnhancedFreeSpins2:
            cycle.fsCollectorTrigger = true;
            cycle.fsInitialSecondaryCollectors = 2;
            cycle.fsTrigger = true;
            break;
        case Forces::ForceEnhancedFreeSpins3:
            cycle.fsCollectorTrigger = true;
            cycle.fsInitialSecondaryCollectors = 3;
            cycle.fsTrigger = true;
            break;
        case Forces::ForceEnhancedFreeSpinsRandom:
            cycle.fsCollectorTrigger = true;
            cycle.fsInitialSecondaryCollectors = maths.intWeightTables["fsInitialSecondaryCollectors"].pick();
            cycle.fsTrigger = true;
        default:
            break;
        }
    }

    void maximizeMultipliersForEnhancedFreeSpins(const GameMaths& maths, GameCycle& cycle)
    {
        if (!(cycle.fsCollectorTrigger && cycle.fsFinalSecondaryCollectors == 3)) return;

        cycle.multipliersMaximized = true;

        for (auto& multiplier : cycle.grid.collectorMultipliers)
            multiplier = 5;

        for (size_t reel = 0; reel < cycle.grid.maxReels; reel++)
            for (size_t row = 0; row < cycle.grid.maxRows; row++)
                cycle.grid.coinMultiplierGrid[reel][row] = 5;

        cycle.recordEvent<MaxedOutMultipliersEvent>();
    }

    void updateDeadSpinsAndPositionsWithCoins(const GameMaths& maths, GameCycle& cycle)
    {
        if (!cycle.bonusSymbolCollected && !cycle.baseCoinsRecorded && !cycle.fsTrigger)
            cycle.session.deadSpins++;
        else cycle.session.deadSpins = 0;
        if (cycle.session.deadSpins < 2) return;
        cycle.session.positionsWithCoinsOrBonuses.clear();
        for (CollectorPosition position : collectorPositionMap | std::views::values)
        {
            const auto& collectorControlArea = maths.intGrids["collectorPositions." + collectorPositionNames[
                position]];
            std::vector<CollectorPosition> inactiveCollectorPositions;
            bool hasCoins = false;
            for (size_t reel = 0; reel < cycle.grid.maxReels && !hasCoins; reel++)
                for (size_t row = 0; row < cycle.grid.maxRows && !hasCoins; row++)
                    if (
                        collectorControlArea[reel][row] == 1
                        && (cycle.grid.getSymbol({reel, row})
                            == maths.payTables["default"].symbolIndexMap.at("C1")
                            || cycle.grid.getSymbol({reel, row})
                            == maths.payTables["default"].symbolIndexMap.at("BN"))
                    )
                        hasCoins = true;
            if (hasCoins) cycle.session.positionsWithCoinsOrBonuses.push_back(position);
        }
    }
}

