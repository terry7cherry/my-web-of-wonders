#pragma once
#include <cstdint>
#include <string>

namespace Stats
{
    void outputStats(uint64_t simGames, const std::string& betType, int rtp, int bet);

    void outputConsoleStats(uint64_t simGames, int bet);

    void outputFileStats(const std::string& betType, int rtp, int bet);

    void outputGeneralStatsReport(uint64_t simGames, int bet);
    void outputMaxWinReport(uint64_t simGames, int bet);
    void outputSpinsReport(uint64_t simGames, int bet);
}
