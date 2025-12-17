#pragma once

#include <utility>
#include <concepts>
#include <memory_resource>

#include "../common/grids/LineGrid.h"
#include "GameMaths.h"
#include "GameSession.h"
#include "../common/events/BasicEvent.h"
#include "../common/events/EventsArena.h"

struct GameCycleReused
{
    std::vector<BasicEvent*> events;
    EventsArena eventsArena{128 * 1024};
    LineGrid grid;

    explicit GameCycleReused(const GameMaths& gameMaths) :
        grid(gameMaths.numReels, gameMaths.numRows)
    {
        events.reserve(1000);
        grid.setPayTable(gameMaths.payTable);
        grid.setPayLines(gameMaths.payLines);
        grid.paySymVect = {1, 2, 3, 4, 5, 6, 7, 8};
        grid.scatterSymVect = {9};
        grid.wildSymVect = {0};
        grid.symbolsInit(11);
    }
};

struct GameCycle
{
    LineGrid& grid;

    std::vector<BasicEvent*>& events;
    EventsArena& eventsArena;
    unsigned int totalWin = 0;
    unsigned int baseGameWin = 0;
    unsigned int freeSpinsWin = 0;
    unsigned int fsInitialSpins = 0;
    unsigned int fsRetriggerSpins = 0;
    unsigned int fsTotalSpinsPlayed = 0;
    unsigned int baseNumScatters = 0;
    bool fsTrigger = false;

    explicit GameCycle(const GameMaths& gameMaths, GameCycleReused& reused)
        : grid(reused.grid),
          events(reused.events),
          eventsArena(reused.eventsArena)
    {
        events.clear();
        eventsArena.reset();
        grid.reset();
    }

    template <typename T, typename... Args>
    void recordEvent(Args&&... args)
        requires std::derived_from<T, BasicEvent> && std::constructible_from<T, Args...>
    {
        void* mem = eventsArena.allocate(sizeof(T), alignof(T));
        T* e = new(mem) T(std::forward<Args>(args)...);
        events.push_back(e);
    }
};

