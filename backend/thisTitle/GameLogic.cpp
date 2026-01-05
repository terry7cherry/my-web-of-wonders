#pragma once

#include "GameLogic.h"
#include "GameMaths.h"
#include "GameSession.h"
#include "GameCycle.h"
#include "../common/spin/SpinResult.h"
#include "../common/events/FreeSpinsRetriggerEvent.h"
#include "../common/events/FreeSpinsTriggerEvent.h"
#include "../common/events/GameCycleCompleteEvent.h"
#include "../common/events/NewFreeSpinEvent.h"
#include "../common/events/ReelSpinEvent.h"
#include "../common/events/WinAnalysisEvent.h"
#include "../common/events/GridUpdateEvent.h"

namespace GameLogic
{
    int playGame(const GameMathsNew& maths, GameSession& session, GameCycle& cycle, const int force)
    {
        if (force == 1)
        {
            cycle.baseNumScatters = 3;
            cycle.fsTrigger = true;
        }
        else
            cycle.totalWin += doBaseGame(maths, session, cycle);

        if (cycle.fsTrigger)
            cycle.totalWin += doFreeSpins(maths, session, cycle);

        cycle.recordEvent<GameCycleCompleteEvent>(cycle.totalWin);
        return cycle.totalWin;
    }

    int doBaseGame(const GameMathsNew& maths, GameSession& session, GameCycle& cycle)
    {
        cycle.grid.applySpin(maths.reelSetWeights.at("baseGame"));

        cycle.recordEvent<ReelSpinEvent>(cycle.grid);

        const size_t spinWin = cycle.grid.evaluateWins(session.betMultiplier, false);
        cycle.baseGameWin += spinWin;

        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, "base game");

        //FIX
        cycle.baseNumScatters = cycle.grid.getSymbolCount(9);
        if (cycle.baseNumScatters >= 3) cycle.fsTrigger = true;

        return cycle.baseGameWin;
    }

    int doFreeSpins(const GameMathsNew& maths, GameSession& session, GameCycle& cycle)
    {
        static auto& fsStartSpins = maths.intThresholdTables.at("fsStartSpins");
        cycle.fsInitialSpins = fsStartSpins.at(cycle.baseNumScatters);
        cycle.recordEvent<FreeSpinsTriggerEvent>(cycle.fsInitialSpins);

        for (cycle.fsTotalSpinsPlayed = 0;
             cycle.fsTotalSpinsPlayed < cycle.fsInitialSpins + cycle.fsRetriggerSpins;
             cycle.fsTotalSpinsPlayed++
        )
            cycle.freeSpinsWin += doOneFreeSpin(maths, session, cycle);

        return cycle.freeSpinsWin;
    }

    int doOneFreeSpin(const GameMathsNew& maths, GameSession& session, GameCycle& cycle)
    {
        cycle.grid.gridMultiplier = maths.intWeightTables.at("fsMultiplierWeights").pick();
        cycle.recordEvent<NewFreeSpinEvent>(cycle.grid.gridMultiplier);

        cycle.grid.applySpin(maths.reelSetWeights.at("freeSpins"));
        cycle.recordEvent<ReelSpinEvent>(cycle.grid);

        //doExpandingWilds(maths, cycle);

        size_t spinWin = cycle.grid.evaluateWins(session.betMultiplier, true);
        if (spinWin)
        {
            if (cycle.grid.gridMultiplier > 1)
                spinWin = cycle.grid.applyGridMultiplier(MultipliersCombineType::add);
        }
        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, "free spin");

        //FIX
        const int scatterCount = cycle.grid.getSymbolCount(9);
        //FIX

        if (scatterCount >= 2)
        {
            static auto& retriggerSpinTable = maths.intThresholdTables.at("fsRetriggerSpins");
            const int retriggerSpins = retriggerSpinTable.at(scatterCount);
            cycle.fsRetriggerSpins += retriggerSpins;
            if (retriggerSpins) cycle.recordEvent<FreeSpinsRetriggerEvent>(retriggerSpins);
        }

        return spinWin;
    }

    void doExpandingWilds(const GameMathsNew& maths, GameCycle& cycle)
    {
        //FIX
        constexpr int wildSymbol = 0;

        size_t numWildReelsExpanded = 0;
        for (size_t reelNum = 0; reelNum < cycle.grid.maxReels; ++reelNum)
            if (cycle.grid.getReelSymbolCount(reelNum, wildSymbol))
            {
                numWildReelsExpanded++;
                cycle.grid.setReelSymbols(reelNum, wildSymbol);
            }

        if (numWildReelsExpanded)
            cycle.recordEvent<GridUpdateEvent>(
                cycle.grid, numWildReelsExpanded > 1 ? "Wilds expanded!" : "Wild expanded!");
    }
}
