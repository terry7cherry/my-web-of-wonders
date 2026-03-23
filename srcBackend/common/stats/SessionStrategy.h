#pragma once

#include <functional>
#include <string>
#include <cstdint>

// Forward declare to avoid heavy includes in common header
struct GameSession;
struct GameCycle;

/**
 * SessionStrategy describes how a batch-play session should start and when it should end.
 * - name: human-readable name for file names / UI selection
 * - initialBalanceMult: starting bankroll as an integer multiple of bet
 * - shouldStop: predicate evaluated at the END of each completed spin; returns true to end session
 */
struct SessionStrategy
{
    std::string name;
    int initialBalanceMult{0};
    // Evaluated at the END of each completed spin; may use GameCycle to check flags like fsTrigger, fsSpinsPlayed
    std::function<bool(long long currentBalance, uint64_t spinsPlayed, const GameSession& session, const GameCycle& cycle)> shouldStop;
};
