#pragma once

#include "../common/gameMaths/GameMaths.h"
#include "GameSession.h"
#include "GameCycle.h"

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
    int playGame(const GameMathsNew& maths, GameSession& session, GameCycle& cycle, int force = 0);

    int doBaseGame(const GameMathsNew& maths, GameSession& session, GameCycle& cycle);

    int doFreeSpins(const GameMathsNew& maths, GameSession& session, GameCycle& cycle);

    int doOneFreeSpin(const GameMathsNew& maths, GameSession& session, GameCycle& cycle);

    void forceSpinToAwardFreeSpins(const GameMathsNew& gameMaths, GameCycle& gameCycle);

    void doExpandingWilds(const GameMathsNew& maths, GameCycle& cycle);
}
