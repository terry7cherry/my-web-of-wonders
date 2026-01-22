#pragma once

#include "../common/gameMaths/GameMaths.h"
#include "state/GameCycle.h"

namespace GameLogic
{
    unsigned int playGame(const GameMaths& maths, GameCycle& cycle);

    void doBaseGame(const GameMaths& maths, GameCycle& cycle);

    void doFreeSpinsIfTriggered(const GameMaths& maths, GameCycle& cycle);

    unsigned int skipBaseGameAndForceFreeSpins(const GameMaths& maths, GameCycle& cycle);

    unsigned int doRandomBaseGameAndCheckFreeSpinsTrigger(const GameMaths& maths, GameCycle& cycle);

    unsigned int doFreeSpins(const GameMaths& maths, GameCycle& cycle);

    void spinReels(const GameMaths& maths, GameCycle& cycle, const std::string& weightTableName);

    void checkForFreeSpinsTrigger(const GameMaths& maths, GameCycle& cycle);

    unsigned int doOneFreeSpin(const GameMaths& maths, GameCycle& cycle);

    unsigned int countBaseScatters(const GameMaths& maths, const GameCycle& cycle);

    void setMultiplierAndRecordNewSpinEvent(const GameMaths& maths, GameCycle& cycle);

    void expandWilds(const GameMaths& maths, GameCycle& cycle);

    unsigned int evaluateWinsWithMultiplier(const GameMaths& maths, GameCycle& cycle);

    void awardRetriggers(const GameMaths& maths, GameCycle& cycle);
}
