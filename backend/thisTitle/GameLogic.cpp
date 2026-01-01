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
    int playGame(const GameMathsNew& gm, GameSession& gs, GameCycle& gc, const int force)
    {
        if (force == 1)
        {
            gc.baseNumScatters = 3;
            gc.fsTrigger = true;
        }
        else
            gc.totalWin += doBaseGame(gm, gs, gc);

        if (gc.fsTrigger)
            gc.totalWin += doFreeSpins(gm, gs, gc);

        gc.recordEvent<GameCycleCompleteEvent>(gc.totalWin);
        return gc.totalWin;
    }

    int doBaseGame(const GameMathsNew& gm, GameSession& gs, GameCycle& gc)
    {
        gc.grid.applySpin(gm.reelSetWeights.at("baseGame"));

        gc.recordEvent<ReelSpinEvent>(gc.grid);

        const int spinWin = gc.grid.evaluateWins(gs.betMultiplier, false);
        gc.baseGameWin += spinWin;

        gc.recordEvent<WinAnalysisEvent>(gc.grid, "base game");

        //FIX
        gc.baseNumScatters = gc.grid.getSymbolCount(9);
        if (gc.baseNumScatters >= 3) gc.fsTrigger = true;

        return gc.baseGameWin;
    }

    int doFreeSpins(const GameMathsNew& gm, GameSession& gs, GameCycle& gc)
    {
        static auto& fsStartSpins = gm.intThresholdTables.at("fsStartSpins");
        gc.fsInitialSpins = fsStartSpins.at(gc.baseNumScatters);
        gc.recordEvent<FreeSpinsTriggerEvent>(gc.fsInitialSpins);

        for (gc.fsTotalSpinsPlayed = 0;
             gc.fsTotalSpinsPlayed < gc.fsInitialSpins + gc.fsRetriggerSpins;
             gc.fsTotalSpinsPlayed++
        )
            gc.freeSpinsWin += doOneFreeSpin(gm, gs, gc);

        return gc.freeSpinsWin;
    }

    int doOneFreeSpin(const GameMathsNew& gm, GameSession& gs, GameCycle& gc)
    {
        gc.grid.gridMultiplier = gm.intWeightTables.at("fsMultiplierWeights").pick();
        gc.recordEvent<NewFreeSpinEvent>(gc.grid.gridMultiplier);

        gc.grid.applySpin(gm.reelSetWeights.at("freeSpins"));
        gc.recordEvent<ReelSpinEvent>(gc.grid);

        doExpandingWilds(gm, gc);

        int spinWin = gc.grid.evaluateWins(gs.betMultiplier, true);
        if (spinWin)
        {
            if (gc.grid.gridMultiplier > 1)
                spinWin = gc.grid.applyGridMultiplier(MultipliersCombineType::add);
        }
        gc.recordEvent<WinAnalysisEvent>(gc.grid, "free spin");

        //FIX
        const int scatterCount = gc.grid.getSymbolCount(9);
        //FIX

        if (scatterCount >= 2)
        {
            static auto& retriggerSpinTable = gm.intThresholdTables.at("fsRetriggerSpins");
            const int retriggerSpins = retriggerSpinTable.at(scatterCount);
            gc.fsRetriggerSpins += retriggerSpins;
            if (retriggerSpins) gc.recordEvent<FreeSpinsRetriggerEvent>(retriggerSpins);
        }

        return spinWin;
    }

    void doExpandingWilds(const GameMathsNew& gm, GameCycle& gc)
    {
        //FIX
        constexpr int wildSymbol = 0;

        size_t numWildReelsExpanded = 0;
        for (size_t reelNum = 0; reelNum < gc.grid.maxReels; ++reelNum)
            if (gc.grid.getReelSymbolCount(reelNum, wildSymbol))
            {
                numWildReelsExpanded++;
                gc.grid.setReelSymbols(reelNum, wildSymbol);
            }

        if (numWildReelsExpanded)
            gc.recordEvent<GridUpdateEvent>(
                gc.grid, numWildReelsExpanded > 1 ? "Wilds expanded!" : "Wild expanded!");
    }
}
