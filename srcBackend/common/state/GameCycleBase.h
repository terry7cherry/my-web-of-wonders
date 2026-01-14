#pragma once

#include <utility>
#include <concepts>
#include <memory_resource>

#include "../events/eventTemplates/BasicEvent.h"

struct GameCycleReused;

struct GameCycleBase
{
#ifdef FRONTEND
    std::vector<BasicEvent*> events;
#endif
    explicit GameCycleBase()
    {
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

