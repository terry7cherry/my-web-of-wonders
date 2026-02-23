#include "StatsOutput.h"

#include "../../common/events/BaseGameCompleteEvent.h"
#include "../../common/events/GameCycleCompleteEvent.h"
#include "../../common/events/FreeSpinsCompleteEvent.h"
#include "../../common/events/WinAnalysisEvent.h"
#include "../../common/utils/utils.h"

namespace Stats
{
    void outputStats(const uint64_t simGames, const std::string& betType, const int rtp, const int bet,
                     const std::vector<std::string>& symbolNames, const unsigned int currentWinCap)
    {
        outputConsoleStats(simGames, bet, symbolNames, currentWinCap);
        outputFileStats(betType, rtp, bet, simGames);
    }

    void outputConsoleStats(const uint64_t simGames, const int bet,
                            const std::vector<std::string>& symbolNames, const unsigned int currentWinCap)
    {
        outputGeneralStatsReport(simGames, bet);
        outputPayTableRTPReport(simGames, bet, symbolNames);
        outputMaxWinReport(simGames, bet);
        outputSpinsReport(simGames, bet);
        outputWinCappingReport(simGames, bet, currentWinCap);
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

    void outputWinCappingReport(const uint64_t simGames, const int bet, const unsigned int currentWinCap)
    {
        const unsigned int winCap[18] = {
            2'000, 3'000, 4'000, 5'000, 6'000, 7'000, 8'000, 9'000, 10'000, 11'000, 12'000, 13'000, 14'000,
            15'000, 20'000, 30'000, 40'000, 50'000
        };
        int scaledWinCap[18] = {};
        const unsigned int currentScaledCap = currentWinCap * bet;
        for (size_t i = 0; i < 18; i++)
            scaledWinCap[i] = winCap[i] * bet;
        uint64_t hitCount[18] = {};
        uint64_t currentCapHitCount = 0;
        uint64_t cappedTotalWin[18] = {};
        uint64_t currentCapTotalWin = 0;
        uint64_t totalWin = 0;
        for (const auto& [win, count] : GameCycleCompleteEvent::eventWinDistribution.getSortedData(true))
        {
            totalWin += win * count;
            if (currentWinCap > 0)
            {
                if (win >= currentScaledCap)
                {
                    currentCapHitCount += count;
                    currentCapTotalWin += currentScaledCap * count;
                }
                else
                    currentCapTotalWin += win * count;
            }
            for (int i = 0; i < 18; i++)
            {
                if (win >= scaledWinCap[i])
                {
                    hitCount[i] += count;
                    cappedTotalWin[i] += scaledWinCap[i] * count;
                }
                else
                    cappedTotalWin[i] += win * count;
            }
        }


        std::printf("\n\nWIN CAP REPORT\n\n");
        std::printf("%14s", "WinCap (xBet)");
        if (currentWinCap > 0)
            std::printf("%14s |", addThousandsSeparator(currentWinCap).c_str());
        for (int i = 0; i < 18; i++)
            std::printf("%14s", addThousandsSeparator(winCap[i]).c_str());
        std::printf("\n");
        std::printf("%14s", "Capped RTP");
        if (currentWinCap > 0)
            std::printf("%13.2f%% |", 100.0 * currentCapTotalWin / simGames / bet);
        for (int i = 0; i < 18; i++)
            std::printf("%13.2f%%", 100.0 * cappedTotalWin[i] / simGames / bet);
        std::printf("\n");
        std::printf("%14s", "RTP lost");
        if (currentWinCap > 0)
            std::printf("%13.3f%% |", 100.0 * (totalWin - currentCapTotalWin) / simGames / bet);
        for (int i = 0; i < 18; i++)
            std::printf("%13.3f%%", 100.0 * (totalWin - cappedTotalWin[i]) / simGames / bet);
        std::printf("\n");
        std::printf("%14s", "Capped 1 in");
        if (currentWinCap > 0)
            std::printf("%14s |", currentCapHitCount
                                      ? addThousandsSeparator(simGames / currentCapHitCount).c_str()
                                      : "Inf");
        for (int i = 0; i < 18; i++)
            std::printf("%14s", hitCount[i] ? addThousandsSeparator(simGames / hitCount[i]).c_str() : "Inf");
        std::printf("\n");
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
