#pragma once

#include <utility>
#include <concepts>
#include <memory_resource>

#include "../common/grids/LineGrid.h"
#include "../common/gameMaths/GameMaths.h"
#include "../common/events/BasicEvent.h"
#include "../common/events/EventsArena.h"

struct GameCycleReused
{
    LineGrid grid;

    explicit GameCycleReused(const GameMaths& gameMaths) :
        grid(gameMaths.ints.at("numReels"), gameMaths.ints.at("numRows"), gameMaths.payTables.at("default"))
    {
        grid.setPayLines(gameMaths.payLines.at("default"));
    }

    void reset()
    {
        grid.reset();
    }
};

struct GameCycle
{
    LineGrid& grid;


#ifdef FRONTEND
    std::vector<BasicEvent*> events;
#endif
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

    template <typename T, typename... Args>
    void recordEvent(Args&&... args)
        requires std::derived_from<T, BasicEvent> && std::constructible_from<T, Args...>
    {
#ifdef FRONTEND
        T* e = new T(std::forward<Args>(args)...);
        events.push_back(e);
#else
        T::updateMeters(std::forward<Args>(args)...);
#endif
    }
};

