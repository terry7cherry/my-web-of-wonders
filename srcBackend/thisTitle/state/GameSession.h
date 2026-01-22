#pragma once

#include "../../common/gameMaths/GameMaths.h"
#include "../../common/state/GameSessionBase.h"
#include "../forces/simForces.h"

struct GameSession : GameSessionBase
{
    // Add any persistent variables that need to be tracked here,
    // e.g., progressive jackpot data.

    GameSession(const int bet,
                const GameMaths& gameMaths,
                const Forces::SimForce simForce = Forces::ForceNone)
        : GameSessionBase(bet, gameMaths, simForce)
    {
    }
};
