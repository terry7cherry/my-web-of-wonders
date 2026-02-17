#pragma once
/**
 * @file GameCycleBase.h
 * @brief Minimal base for a game cycle, providing event recording or metering.
 *
 * In FRONTEND builds, events are instantiated and stored for later serialization to the UI.
 * In non-FRONTEND (simulation) builds, no objects are created; instead, event types update
 * their aggregate meters via static hooks.
 */

#include <utility>
#include <concepts>
#include <memory_resource>

#include "../events/eventTemplates/BasicEvent.h"
#include "../../thisTitle/state/GameSession.h"

struct GameCycleReused;

/**
 * @brief Base struct exposing `recordEvent` with dual behavior for FRONTEND vs simulation.
 */
struct GameCycleBase
{
    GameSession& session;
#if defined(FRONTEND) || defined(RN_STREAM)
    std::vector<BasicEvent*> events;
#endif

    /**
     * @param session GameSession to store a reference to.
     */
    explicit GameCycleBase(GameSession& session)
        : session(session)
    {
    }

    /**
     * @brief Record an event or update meters depending on build.
     *
     * FRONTEND: constructs the concrete event `T` in heap storage and appends to `events`.
     * Non-FRONTEND: calls `T::updateMeters(args...)` without constructing an object.
     *
     * @tparam T Event type derived from `BasicEvent`.
     * @tparam Args Argument types forwarded to the event constructor or `updateMeters`.
     * @param args Constructor arguments for `T` (FRONTEND) or params for `updateMeters` (sim).
     */
    template <typename T, typename... Args>
    void recordEvent(Args&&... args)
        requires std::derived_from<T, BasicEvent> && std::constructible_from<T, Args...>
    {
#if defined(FRONTEND) || defined(RN_STREAM)
        T* e = new T(std::forward<Args>(args)...);
        events.push_back(e);
#else
        T::updateMeters(std::forward<Args>(args)...);
#endif
    }
};

