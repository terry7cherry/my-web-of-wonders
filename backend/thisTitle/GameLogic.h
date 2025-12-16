#pragma once

#include "GameMaths.h"
#include "GameSession.h"
#include "GameCycle.h"

enum EventID {
    EventIDGame,
    EventIDBaseGame,
    EventIDFreeSpins,
    EventIDBaseGameStd,
    EventIDFreeSpinsStd,
    EventIDCount
};

namespace GameLogic {
    int playGame(const GameMaths &gm, GameSession &gs, GameCycle &gc, int force = 0);

    int doBaseGame(const GameMaths &gm, GameSession &gs, GameCycle &gc);

    int doFreeSpins(const GameMaths &gm, GameSession &gs, GameCycle &gc);

    int doOneFreeSpin(const GameMaths &gm, GameSession &gs, GameCycle &gc);

    void forceSpinToAwardFreeSpins(const GameMaths &gameMaths, GameCycle &gameCycle);

    void doExpandingWilds(const GameMaths &gm, GameCycle &gc);
}
