#include "StatsOutput.h"

#include "../../common/events/BaseGameCompleteEvent.h"
#include "../../common/events/GameCycleCompleteEvent.h"
#include "../../common/events/FreeSpinsCompleteEvent.h"
#include "../../common/events/WinAnalysisEvent.h"

namespace Stats
{
    void outputStats(const uint64_t simGames, const std::string& betType, const int rtp, const int bet,
                     const std::vector<std::string>& symbolNames)
    {
        outputConsoleStats(simGames, bet, symbolNames);
        outputFileStats(betType, rtp, bet, simGames);
    }

    void outputConsoleStats(const uint64_t simGames, const int bet,
                            const std::vector<std::string>& symbolNames)
    {
        outputGeneralStatsReport(simGames, bet);
        outputPayTableRTPReport(simGames, bet, symbolNames);
        outputMaxWinReport(simGames, bet);
        outputSpinsReport(simGames, bet);
    }

    void outputFileStats(const std::string& betType, const int rtp, const int bet, const uint64_t simGames)
    {
        const std::filesystem::path outputFolder = createOutputFolder(betType, rtp, bet, simGames);
        GameCycleCompleteEvent::outputDistribution(betType, rtp, bet, false, true, outputFolder);
        BaseGameCompleteEvent::outputDistribution(betType, rtp, bet, false, true, outputFolder);
        FreeSpinsCompleteEvent::outputDistribution(betType, rtp, bet, false, true, outputFolder);
        // You can add other phases (features/bonuses etc) here to output distribution files for them.
        // Ensure you have created an event for your phase derived from the PhaseCompleteEvent class.
        // (Copy from common/events/BaseGameCompleteEvent.h, for example).
        // Make sure you call cycle.recordEvent<YourPhaseCompleteEvent>(phaseWinnings) in your GameLogic.
        // Call outputDistribution on the new event above.
    }

    void outputGeneralStatsReport(const uint64_t simGames, const int bet)
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

    void outputPayTableRTPReport(const uint64_t simGames, const int bet,
                                 const std::vector<std::string>& symbolNames)
    {
        WinAnalysisEvent::consoleReportRTP(simGames, bet, symbolNames);
    }

    void outputMaxWinReport(const uint64_t simGames, const int bet)
    {
        printf("\n\nMAX WIN REPORT\n");
        std::printf("%20s,%12s,%12s,%12s\n",
                    "Phase", "MaxWin (xBet)", "HitRate", "EventRate");
        GameCycleCompleteEvent::maxWinReport(simGames, bet);
        BaseGameCompleteEvent::maxWinReport(simGames, bet);
        FreeSpinsCompleteEvent::maxWinReport(simGames, bet);
        // Follow instructions in outputGeneralStatsReport to add stats for other phases here.
    }

    void outputSpinsReport(const uint64_t simGames, const int bet)
    {
        printf("\n\nSPINS REPORT\n");
        std::printf("\nPhase: Free Spins\n");
        FreeSpinsCompleteEvent::outputInitialSpinsReport(simGames * bet);
        FreeSpinsCompleteEvent::outputFinalSpinsReport(simGames * bet);
        // To add other phases here the event must derive from SpinsCompleteTemplate.
        // (Copy from FreeSpinsComplete event.)
        // Then follow instructions in outputGeneralStatsReport.
    }

    std::filesystem::path createOutputFolder(const std::string& betType, const int rtp, const int bet,
                                             const uint64_t simGames)
    {
        namespace fs = std::filesystem;

        const fs::path distributionsRoot = fs::path(GAME_SOURCE_DIR) / "Simulations";
        fs::create_directories(distributionsRoot);

        const auto now = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H.%M");

        if (simGames > BILLION)
            oss << "_" << simGames / BILLION << "b";
        else
            oss << "_" << simGames / MILLION << "m";

        oss << "_RTP" << rtp << "_bet" << bet;

        if (betType != "default") oss << "_" << betType;

        const fs::path outputFolder = distributionsRoot / oss.str() / "Distributions";
        fs::create_directories(outputFolder);
        return outputFolder;
    }
}
