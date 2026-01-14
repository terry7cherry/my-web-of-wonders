#pragma once

#include "GameLogic.h"
#include "state/GameSession.h"
#include "state/GameCycle.h"
#include "forces/simForces.h"
#include "../common/spin/SpinResult.h"
#include "../common/events/BaseGameCompleteEvent.h"
#include "../common/events/FreeSpinsCompleteEvent.h"
#include "../common/events/FreeSpinsRetriggerEvent.h"
#include "../common/events/FreeSpinsTriggerEvent.h"
#include "../common/events/GameCycleCompleteEvent.h"
#include "../common/events/NewFreeSpinEvent.h"
#include "../common/events/ReelSpinEvent.h"
#include "../common/events/WinAnalysisEvent.h"
#include "../common/events/GridUpdateEvent.h"

namespace GameLogic
{
    unsigned int playGame(const GameMaths& maths, GameSession& session, GameCycle& cycle, const int force)
    {
        if (force == SimForces::ForceFreeSpins)
        {
            cycle.fsInitialSpins = 10;
            cycle.fsTrigger = true;
        }
        else
            cycle.totalWin += doBaseGame(maths, session, cycle);
        cycle.recordEvent<BaseGameCompleteEvent>(cycle.baseGameWin);

        if (cycle.fsTrigger)
        {
            cycle.totalWin += doFreeSpins(maths, session, cycle);
            cycle.recordEvent<FreeSpinsCompleteEvent>(cycle.freeSpinsWin,
                                                      cycle.fsInitialSpins,
                                                      cycle.fsTotalSpinsPlayed);
        }

        cycle.recordEvent<GameCycleCompleteEvent>(cycle.totalWin);
        return cycle.totalWin;
    }

    unsigned int doBaseGame(const GameMaths& maths, GameSession& session, GameCycle& cycle)
    {
        cycle.grid.applySpin(maths.reelSetWeights["baseGame"]);

        cycle.recordEvent<ReelSpinEvent>(cycle.grid);

        const unsigned int spinWin = cycle.grid.evaluateWins(session.betMultiplier, false);
        cycle.baseGameWin += spinWin;

        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, "base game");

        static Symbol fsTriggerSymbol = maths.symbols["fsTriggerSymbol"];
        cycle.baseNumScatters = cycle.grid.getSymbolCount(fsTriggerSymbol);
        auto& fsStartSpins = maths.awards["fsStartSpins"];
        if ((cycle.fsInitialSpins = fsStartSpins.at(cycle.baseNumScatters)))
            cycle.fsTrigger = true;

        return cycle.baseGameWin;
    }

    unsigned int doFreeSpins(const GameMaths& maths, GameSession& session, GameCycle& cycle)
    {
        cycle.recordEvent<FreeSpinsTriggerEvent>(cycle.fsInitialSpins);

        for (cycle.fsTotalSpinsPlayed = 0;
             cycle.fsTotalSpinsPlayed < cycle.fsInitialSpins + cycle.fsRetriggerSpins;
             cycle.fsTotalSpinsPlayed++
        )
            cycle.freeSpinsWin += doOneFreeSpin(maths, session, cycle);

        return cycle.freeSpinsWin;
    }

    unsigned int doOneFreeSpin(const GameMaths& maths, GameSession& session, GameCycle& cycle)
    {
        cycle.grid.gridMultiplier = maths.weightTables["fsMultiplierWeights"].pick();
        cycle.recordEvent<NewFreeSpinEvent>(cycle.grid.gridMultiplier);

        cycle.grid.applySpin(maths.reelSetWeights["freeSpins"]);
        cycle.recordEvent<ReelSpinEvent>(cycle.grid);

        //doExpandingWilds(maths, cycle);

        unsigned int spinWin = cycle.grid.evaluateWins(session.betMultiplier, true);
        if (spinWin)
        {
            if (cycle.grid.gridMultiplier > 1)
                spinWin = cycle.grid.applyGridMultiplier(MultipliersCombineType::add);
        }
        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, "free spin");

        static Symbol fsTriggerSymbol = maths.symbols["fsTriggerSymbol"];
        const int scatterCount = cycle.grid.getSymbolCount(fsTriggerSymbol);

        if (scatterCount >= 2)
        {
            auto& retriggerSpinTable = maths.awards["fsRetriggerSpins"];
            const int retriggerSpins = retriggerSpinTable.at(scatterCount);
            cycle.fsRetriggerSpins += retriggerSpins;
            if (retriggerSpins) cycle.recordEvent<FreeSpinsRetriggerEvent>(retriggerSpins);
        }

        return spinWin;
    }

    void doExpandingWilds(const GameMaths& maths [[maybe_unused]], GameCycle& cycle)
    {
        //FIX
        constexpr int wildSymbol = 0;

        unsigned int numWildReelsExpanded = 0;
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
