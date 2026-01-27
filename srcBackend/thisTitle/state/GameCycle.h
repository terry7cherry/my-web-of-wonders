#pragma once

#include "../../common/grids/LineGrid.h"
#include "../../common/gameMaths/GameMaths.h"
#include "../../common/state/GameCycleBase.h"
#include "./GameSession.h"

struct GameCycleReused
{
    LineGrid grid; // Use a different Grid class if necessary, e.g., WaysGrid.

    // Add other members here that are very costly to initialise.
    // Initialise them once either in the GameCycleReused initializer list or in the constructor body.
    // Reset them in the reset() method (called before each game cycle).
    // Add a reference to them as a member in the GameCycle struct and add to the initializer list.
    // (Follow how this is done with the grid member.)

    explicit GameCycleReused(const GameMaths& gameMaths) :
        grid(gameMaths)
    {
    }

    void reset()
    {
        grid.reset();
    }
};

struct GameCycle : GameCycleBase
{
    LineGrid& grid;
    unsigned int totalWin = 0;
    unsigned int baseGameWin = 0;
    unsigned int freeSpinsWin = 0;
    unsigned int fsInitialSpins = 0;
    unsigned int fsRetriggerSpins = 0;
    unsigned int fsSpinsPlayed = 0;
    unsigned int baseNumScatters = 0;
    bool fsTrigger = false;

    explicit GameCycle(GameSession& session, GameCycleReused& reused, const GameMaths& maths [[maybe_unused]])
        : GameCycleBase(session),
          grid(reused.grid)
    {
        reused.reset();
        // Use maths to initialise member variables if necessary.
    }
};

