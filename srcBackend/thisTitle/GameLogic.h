#pragma once

#include "../common/gameMaths/GameMaths.h"
#include "state/GameSession.h"
#include "state/GameCycle.h"

enum EventID
{
    EventIDGame,
    EventIDBaseGame,
    EventIDFreeSpins,
    EventIDBaseGameStd,
    EventIDFreeSpinsStd,
    EventIDCount
};

namespace GameLogic
{
    unsigned int playGame(const GameMaths& maths, GameSession& session, GameCycle& cycle, int force = 0);

    unsigned int doBaseGame(const GameMaths& maths, GameSession& session, GameCycle& cycle);

    unsigned int doFreeSpins(const GameMaths& maths, GameSession& session, GameCycle& cycle);

    unsigned int doOneFreeSpin(const GameMaths& maths, GameSession& session, GameCycle& cycle);

    void forceSpinToAwardFreeSpins(const GameMaths& gameMaths, GameCycle& gameCycle);

    void doExpandingWilds(const GameMaths& maths, GameCycle& cycle);
}
