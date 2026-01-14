#pragma once

#include <utility>

#include "../../common/gameMaths/GameMaths.h"
#include "../../common/state/GameSessionBase.h"

struct GameSession : GameSessionBase
{
    // Add any persistent variables that need to be tracked here,
    // e.g., progressive jackpot data.

    GameSession(const int bet,
                const GameMaths& gameMaths)
        : GameSessionBase(bet, gameMaths)
    {
    }
};
