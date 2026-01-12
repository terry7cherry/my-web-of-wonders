#include "StatsOutput.h"

#include "../common/events/BaseGameCompleteEvent.h"
#include "../common/events/GameCycleCompleteEvent.h"
#include "../common/events/FreeSpinsCompleteEvent.h"

void Stats::outputStats(const uint64_t simGames, const std::string& betType, const int rtp, const int bet)
{
    outputConsoleStats(simGames, bet);
    outputFileStats(betType, rtp, bet);
}

void Stats::outputConsoleStats(const uint64_t simGames, const int bet)
{
    outputGeneralStatsReport(simGames, bet);
    outputMaxWinReport(simGames, bet);
    outputSpinsReport(simGames, bet);
}

void Stats::outputFileStats(const std::string& betType, const int rtp, const int bet)
{
    GameCycleCompleteEvent::outputDistribution(betType, rtp, bet, false, true);
    BaseGameCompleteEvent::outputDistribution(betType, rtp, bet, false, true);
    FreeSpinsCompleteEvent::outputDistribution(betType, rtp, bet, false, true);
}

void Stats::outputGeneralStatsReport(const uint64_t simGames, const int bet)
{
    printf("\n\nGENERAL STATS REPORT\n");
    std::printf("%20s,%12s,%12s,%12s,%12s,%12s,%12s\n",
                "Phase", "RTP", "HitRate", "AvgPay", "AvgWin", "WinChance", "WinRate");
    GameCycleCompleteEvent::finalReport(simGames, bet);
    BaseGameCompleteEvent::finalReport(simGames, bet);
    FreeSpinsCompleteEvent::finalReport(simGames, bet);
    // You can add other phases (features/bonuses etc) to this report.
    // Ensure you have created an event for your phase derived from the PhaseCompleteEvent class.
    // (Copy from common/events/BaseGameCompleteEvent.h, for example).
    // Make sure you call cycle.recordEvent<YourPhaseCompleteEvent>(phaseWinnings) in your GameLogic.
    // Call finalReport on the new event above.
};


void Stats::outputMaxWinReport(const uint64_t simGames, const int bet)
{
    printf("\n\nMAX WIN REPORT\n");
    std::printf("%20s,%12s,%12s,%12s\n",
                "Phase", "MaxWin (xBet)", "HitRate", "EventRate");
    GameCycleCompleteEvent::maxWinReport(simGames, bet);
    BaseGameCompleteEvent::maxWinReport(simGames, bet);
    FreeSpinsCompleteEvent::maxWinReport(simGames, bet);
    // Follow instructions in outputGeneralStatsReport to add stats for other phases here.
}

void Stats::outputSpinsReport(const uint64_t simGames, const int bet)
{
    printf("\n\nSPINS REPORT\n");
    std::printf("\nPhase: Free Spins\n");
    FreeSpinsCompleteEvent::outputInitialSpinsReport(simGames * bet);
    FreeSpinsCompleteEvent::outputFinalSpinsReport(simGames * bet);
    // To add other phases here the event must derive from SpinsCompleteTemplate.
    // (Copy from FreeSpinsComplete event.)
    // Then follow instructions in outputGeneralStatsReport.
}
