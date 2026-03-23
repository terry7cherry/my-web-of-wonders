#include "SessionStats.h"
#include "../../common/events/FreeSpinsCompleteEvent.h"
#include "../../common/events/WinAnalysisEvent.h"
#include "../../common/events/GameCycleCompleteEvent.h"
#include "../../common/events/threadSafeMeters/MeterUpdate.h"
#include "../state/GameCycle.h"

namespace SessionStats
{
    struct Accum
    {
        long long startingBalance = 0;
        uint64_t spins = 0;
        long long totalBet = 0;
        long long totalPayout = 0;
        uint64_t winSpins = 0;
        uint64_t zeroWinSpins = 0;
        uint64_t fsTriggers = 0;
        uint64_t fsSpins = 0;
        unsigned int maxSingleWin = 0;
    };

    static thread_local Accum g;
    static std::vector<std::string> gHeaders = {
        "SessionIndex",
        "StartingBalance",
        "Spins",
        "TotalBet",
        "TotalPayout",
        "RTP",
        "WinSpins",
        "ZeroWinSpins",
        "FS_Triggers",
        "FS_Spins",
        "MaxSingleWin",
        "EndBalance",
        "NetProfit",
        "HP Wins",
        "MP Wins",
        "LP Wins",
    };

    const std::vector<std::string>& headers()
    {
        return gHeaders;
    }

    void beginSession(const long long startingBalance)
    {
        g = Accum{};
        g.startingBalance = startingBalance;
    }

    void storeSessionStatsForSpin(const GameCycle& cycle, const int bet)
    {
        g.spins += 1;
        g.totalBet += bet;
        g.totalPayout += cycle.totalWin;
        if (cycle.totalWin > 0) g.winSpins += 1;
        else g.zeroWinSpins += 1;
        if (cycle.fsTrigger)
        {
            g.fsTriggers += 1;
            g.fsSpins += cycle.fsSpinsPlayed;
        }
        if (cycle.totalWin > g.maxSingleWin) g.maxSingleWin = cycle.totalWin;
    }

    std::vector<float> storeSessionStatsForSession(const long long endingBalance,
                                                   const std::vector<std::string>& symbolNames)
    {
        std::vector<float> row;
        // Caller is expected to prepend SessionIndex externally when writing CSV
        row.push_back(static_cast<float>(g.startingBalance));
        row.push_back(static_cast<float>(g.spins));
        row.push_back(static_cast<float>(g.totalBet));
        row.push_back(static_cast<float>(g.totalPayout));
        const float rtp = g.totalBet > 0
                              ? static_cast<float>(static_cast<long double>(g.totalPayout) / static_cast<long
                                  double>(g.totalBet))
                              : 0.0f;
        row.push_back(rtp);
        row.push_back(static_cast<float>(g.winSpins));
        row.push_back(static_cast<float>(g.zeroWinSpins));
        row.push_back(static_cast<float>(g.fsTriggers));
        row.push_back(static_cast<float>(g.fsSpins));
        row.push_back(static_cast<float>(g.maxSingleWin));
        row.push_back(static_cast<float>(endingBalance));
        row.push_back(static_cast<float>(endingBalance - g.startingBalance));
        const auto& [hp, mp, lp] = getWins(symbolNames);
        row.push_back(static_cast<float>(hp));
        row.push_back(static_cast<float>(mp));
        row.push_back(static_cast<float>(lp));
        // Ensure thread-local meters are flushed if the caller wants to read any (example only)
        MeterUpdate::triggerUpdate();
        return row;
    }

    std::tuple<uint64_t, uint64_t, uint64_t> getWins(const std::vector<std::string>& symbolNames)
    {
        Symbol symbol = 0;
        uint64_t hpWins = 0;
        uint64_t mpWins = 0;
        uint64_t lpWins = 0;
        auto& payTableMeter = WinAnalysisEvent::payTableHits;
        const auto& localArray = payTableMeter.getLocalArray();
        for (const auto& symbolHits : localArray)
        {
            uint64_t total = 0;
            for (const uint64_t hit : symbolHits)
                total += hit;
            if (total > 0)
            {
                const std::string& symbolName = symbolNames.at(symbol);
                if (symbolName.starts_with("H"))
                    hpWins += total;
                else if (symbolName.starts_with("M"))
                    mpWins += total;
                else if (symbolName.starts_with("L"))
                    lpWins += total;
            }
            else
            {
                if (symbol > symbolNames.size()) break;
            }
            symbol++;
        }

        return {hpWins, mpWins, lpWins};
    }
}
