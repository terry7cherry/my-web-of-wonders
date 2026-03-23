#pragma once

#include "../../common/state/GameSessionBase.h"
#include "../../common/stats/SessionStrategy.h"
#include "../state/GameSession.h"
#include "../state/GameCycle.h"

namespace SessionStats
{
    const std::vector sessionStrategies = {
        SessionStrategy(
            "100 Spins",
            100,
            [](
            const long long currentBalance [[maybe_unused]],
            const uint64_t spinsPlayed [[maybe_unused]],
            const GameSession& session [[maybe_unused]],
            const GameCycle& cycle [[maybe_unused]])
            {
                return spinsPlayed >= 100;
            }),
        SessionStrategy(
            "500 Spins",
            500,
            [](
            const long long currentBalance [[maybe_unused]],
            const uint64_t spinsPlayed [[maybe_unused]],
            const GameSession& session [[maybe_unused]],
            const GameCycle& cycle [[maybe_unused]])
            {
                return spinsPlayed >= 500;
            }),
        SessionStrategy(
            "Until First Free Spins Or 1000xBet spent",
            1000,
            [](
            const long long currentBalance [[maybe_unused]],
            const uint64_t spinsPlayed [[maybe_unused]],
            const GameSession& session [[maybe_unused]],
            const GameCycle& cycle [[maybe_unused]])
            {
                return cycle.fsTrigger || currentBalance < session.bet;
            }),
        SessionStrategy(
            "Spend 100xBet",
            100,
            [](
            const long long currentBalance [[maybe_unused]],
            const uint64_t spinsPlayed [[maybe_unused]],
            const GameSession& session [[maybe_unused]],
            const GameCycle& cycle [[maybe_unused]])
            {
                return currentBalance < session.bet;
            }),
    };
}
