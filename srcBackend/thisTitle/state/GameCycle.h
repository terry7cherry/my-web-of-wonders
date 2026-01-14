#pragma once

#include "../../common/grids/LineGrid.h"
#include "../../common/gameMaths/GameMaths.h"
#include "../../common/state/GameCycleFunctionalBase.h"

struct GameCycleReused
{
    LineGrid grid;

    explicit GameCycleReused(const GameMaths& gameMaths) :
        grid(gameMaths)
    {
    }

    void reset()
    {
        grid.reset();
    }
};

struct GameCycle : GameCycleFunctionalBase
{
    LineGrid& grid;
    unsigned int totalWin = 0;
    unsigned int baseGameWin = 0;
    unsigned int freeSpinsWin = 0;
    unsigned int fsInitialSpins = 0;
    unsigned int fsRetriggerSpins = 0;
    unsigned int fsTotalSpinsPlayed = 0;
    unsigned int baseNumScatters = 0;
    bool fsTrigger = false;

    explicit GameCycle(GameCycleReused& reused)
        : grid(reused.grid)
    {
        reused.reset();
    }
};

