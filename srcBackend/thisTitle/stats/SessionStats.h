#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct GameCycle;
struct GameSession;

namespace SessionStats
{
    // Returns headers for the per-session CSV
    const std::vector<std::string>& headers();

    // Reset per-session accumulators and set starting balance
    void beginSession(long long startingBalance);

    // Called after each completed spin
    void storeSessionStatsForSpin(const GameCycle& cycle, int bet);

    // Called at the end of a session to produce one CSV row. This may also read thread-local meters
    // from events if needed (e.g., FreeSpinsCompleteEvent) after MeterUpdate::triggerUpdate() is called.
    std::vector<float> storeSessionStatsForSession(long long endingBalance,
                                                   const std::vector<std::string>& symbolNames);

    std::tuple<uint64_t, uint64_t, uint64_t> getWins(const std::vector<std::string>& symbolNames);
}
