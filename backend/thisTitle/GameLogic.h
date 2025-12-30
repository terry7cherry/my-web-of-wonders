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
    int playGame(const GameMathsNew& gm, GameSession& gs, GameCycle& gc, int force = 0);

    int doBaseGame(const GameMathsNew& gm, GameSession& gs, GameCycle& gc);

    int doFreeSpins(const GameMathsNew& gm, GameSession& gs, GameCycle& gc);

    int doOneFreeSpin(const GameMathsNew& gm, GameSession& gs, GameCycle& gc);

    void forceSpinToAwardFreeSpins(const GameMathsNew& gameMaths, GameCycle& gameCycle);

    void doExpandingWilds(const GameMathsNew& gm, GameCycle& gc);
}
