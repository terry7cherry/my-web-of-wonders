#pragma once

#include "GameLogic.h"
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
    unsigned int playGame(const GameMaths& maths, GameCycle& cycle)
    {
        doBaseGame(maths, cycle);
        doFreeSpinsIfTriggered(maths, cycle);

        cycle.recordEvent<GameCycleCompleteEvent>(cycle.totalWin);
        return cycle.totalWin;
    }

    void doBaseGame(const GameMaths& maths, GameCycle& cycle)
    {
        if (cycle.session.simForce == Forces::ForceFreeSpins)
            skipBaseGameAndForceFreeSpins(maths, cycle);
        else
            doRandomBaseGameAndCheckFreeSpinsTrigger(maths, cycle);

        cycle.totalWin += cycle.baseGameWin;
        cycle.recordEvent<BaseGameCompleteEvent>(cycle.baseGameWin);
    }


    void doFreeSpinsIfTriggered(const GameMaths& maths, GameCycle& cycle)
    {
        if (!cycle.fsTrigger) return;

        cycle.recordEvent<FreeSpinsTriggerEvent>(cycle.fsInitialSpins);
        cycle.totalWin += doFreeSpins(maths, cycle);
        cycle.recordEvent<FreeSpinsCompleteEvent>(cycle);
    }

    unsigned int skipBaseGameAndForceFreeSpins(const GameMaths& maths, GameCycle& cycle)
    {
        cycle.baseNumScatters = 3;
        cycle.fsInitialSpins = 10;
        cycle.fsTrigger = true;
        return 0;
    }

    unsigned int doRandomBaseGameAndCheckFreeSpinsTrigger(const GameMaths& maths, GameCycle& cycle)
    {
        spinReels(maths, cycle, "baseGame");

        cycle.baseGameWin += cycle.grid.evaluateWins(cycle.session.betMultiplier, false);
        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, "base game");

        checkForFreeSpinsTrigger(maths, cycle);
        return cycle.baseGameWin;
    }

    unsigned int doFreeSpins(const GameMaths& maths, GameCycle& cycle)
    {
        for (cycle.fsSpinsPlayed = 0;
             cycle.fsSpinsPlayed < cycle.fsInitialSpins + cycle.fsRetriggerSpins;
             cycle.fsSpinsPlayed++
        )
            cycle.freeSpinsWin += doOneFreeSpin(maths, cycle);

        return cycle.freeSpinsWin;
    }

    void spinReels(const GameMaths& maths, GameCycle& cycle, const std::string& weightTableName)
    {
        cycle.grid.applySpin(maths.reelSetWeights[weightTableName]);
        cycle.recordEvent<ReelSpinEvent>(cycle.grid);
    }

    void checkForFreeSpinsTrigger(const GameMaths& maths, GameCycle& cycle)
    {
        cycle.baseNumScatters = countBaseScatters(maths, cycle);
        cycle.fsInitialSpins = maths.awards["fsStartSpins"][cycle.baseNumScatters];
        if (cycle.fsInitialSpins) cycle.fsTrigger = true;
    }

    unsigned int doOneFreeSpin(const GameMaths& maths, GameCycle& cycle)
    {
        setMultiplierAndRecordNewSpinEvent(maths, cycle);

        spinReels(maths, cycle, "freeSpins");
        //expandWilds(maths, cycle);

        const unsigned int spinWin = evaluateWinsWithMultiplier(maths, cycle);
        awardRetriggers(maths, cycle);

        return spinWin;
    }

    unsigned int countBaseScatters(const GameMaths& maths, const GameCycle& cycle)
    {
        static Symbol fsTriggerSymbol = maths.symbols["fsTriggerSymbol"];
        return cycle.grid.getSymbolCount(fsTriggerSymbol);
    }

    void setMultiplierAndRecordNewSpinEvent(const GameMaths& maths, GameCycle& cycle)
    {
        cycle.grid.gridMultiplier = maths.weightTables["fsMultiplierWeights"].pick();
        cycle.recordEvent<NewFreeSpinEvent>(cycle.grid.gridMultiplier);
    }

    void expandWilds(const GameMaths& maths, GameCycle& cycle)
    {
        const int numWildReelsExpanded = cycle.grid.expandSymbol(maths.symbols["wild"]);

        if (numWildReelsExpanded > 0)
            cycle.recordEvent<GridUpdateEvent>(
                cycle.grid, numWildReelsExpanded > 1 ? "Wilds expanded!" : "Wild expanded!");
    }

    unsigned int evaluateWinsWithMultiplier(const GameMaths& maths, GameCycle& cycle)
    {
        unsigned int spinWin = cycle.grid.evaluateWins(cycle.session.betMultiplier, true);

        if (spinWin && cycle.grid.gridMultiplier > 1)
            spinWin = cycle.grid.applyGridMultiplier(MultipliersCombineType::add);

        cycle.recordEvent<WinAnalysisEvent>(cycle.grid, "free spin");
        return spinWin;
    }

    void awardRetriggers(const GameMaths& maths, GameCycle& cycle)
    {
        const int scatterCount = cycle.grid.getSymbolCount(maths.symbols["fsTriggerSymbol"]);

        const int retriggerSpins = maths.awards["fsRetriggerSpins"][scatterCount];
        cycle.fsRetriggerSpins += retriggerSpins;

        if (retriggerSpins) cycle.recordEvent<FreeSpinsRetriggerEvent>(retriggerSpins);
    }
}
