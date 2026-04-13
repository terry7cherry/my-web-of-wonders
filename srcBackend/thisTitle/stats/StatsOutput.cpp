
#include "../../common/events/BaseGameCompleteEvent.h"
#include "../../common/events/GameCycleCompleteEvent.h"
#include "../../common/events/FreeSpinsCompleteEvent.h"
#include "../../common/events/WinAnalysisEvent.h"
#include "../../common/utils/utils.h"
#include <fstream>
#include <sstream>

#include "../events/BaseCoinsPhaseCompleteEvent.h"
#include "../events/BasePayAnywherePhaseCompleteEvent.h"
#include "../events/BonusSymbolCollectionEvent.h"
#include "../events/CoinCollectionEvent.h"
#include "../events/EnhancedFreeSpinsCompleteEvent.h"
#include "../events/EnhancedFreeSpins1CompleteEvent.h"
#include "../events/EnhancedFreeSpins2CompleteEvent.h"
#include "../events/EnhancedFreeSpins3CompleteEvent.h"
#include "../events/FreeSpinsCoinsPayoutEvent.h"
#include "../events/FreeSpinsPayAnywherePhaseCompleteEvent.h"
#include "../events/StandardFreeSpinsCompleteEvent.h"

namespace Stats
{
    void outputStats(const uint64_t simGames, const std::string& betType, const int rtp, const int bet,
                     const std::string& simForce, const int seed,
                     const std::vector<std::string>& symbolNames, const unsigned int currentWinCap)
    {
        ReportBuffer buffer(simGames, betType, rtp, bet, simForce, seed);

        outputDistributionStats(betType, rtp, bet, simGames, buffer.createSubFolder("Distributions"));

        addGeneralStatsReport(buffer, simGames, bet);
        addEndingSecondaryCollectorsReport(buffer);
        addMaxWinReport(buffer, simGames, bet);
        addPayTableReports(buffer, simGames, bet, symbolNames);
        addSpinsReport(buffer, simGames, bet);
        addWinCapReport(buffer, simGames, bet, currentWinCap);

        buffer.printConsole();
        buffer.writeCsv();
    }

    void outputDistributionStats(const std::string& betType, const int rtp, const int bet,
                                 const uint64_t simGames,
                                 const std::filesystem::path& outputFolder)
    {
        GameCycleCompleteEvent::outputDistribution(betType, rtp, bet, false, true, outputFolder);
        BaseGameCompleteEvent::outputDistribution(betType, rtp, bet, false, true, outputFolder);
        FreeSpinsCompleteEvent::outputDistribution(betType, rtp, bet, false, true, outputFolder);
        // You can add other phases (features/bonuses etc) here to output distribution files for them.
        // Ensure you have created an event for your phase derived from the PhaseCompleteEvent class.
        // (Copy from common/events/BaseGameCompleteEvent.h, for example).
        // Make sure you call cycle.recordEvent<YourPhaseCompleteEvent>(phaseWinnings) in your GameLogic.
        // Call outputDistribution on the new event above.
    }

    void addGeneralStatsReport(ReportBuffer& buffer, const uint64_t simGames, const int bet)
    {
        GameCycleCompleteEvent::finalReport(buffer, simGames, bet);
        BaseGameCompleteEvent::finalReport(buffer, simGames, bet);
        FreeSpinsCompleteEvent::finalReport(buffer, simGames, bet);
        StandardFreeSpinsCompleteEvent::finalReport(buffer, simGames, bet);
        BonusSymbolCollectionEvent::finalReport(buffer, simGames, bet);
        EnhancedFreeSpinsCompleteEvent::finalReport(buffer, simGames, bet);
        EnhancedFreeSpins1CompleteEvent::finalReport(buffer, simGames, bet);
        EnhancedFreeSpins2CompleteEvent::finalReport(buffer, simGames, bet);
        EnhancedFreeSpins3CompleteEvent::finalReport(buffer, simGames, bet);
        BasePayAnywherePhaseCompleteEvent::finalReport(buffer, simGames, bet);
        BaseCoinsPhaseCompleteEvent::finalReport(buffer, simGames, bet);
        BaseCoinsPayoutEvent::finalReport(buffer, simGames, bet);
        FreeSpinsPayAnywherePhaseCompleteEvent::finalReport(buffer, simGames, bet);
        FreeSpinsCoinsPayoutEvent::finalReport(buffer, simGames, bet);
        // You can add other phases (features/bonuses etc) to this report.
        // Ensure you have created an event for your phase derived from the PhaseCompleteEvent class.
        // (Copy from common/events/BaseGameCompleteEvent.h, for example).
        // Make sure you call cycle.recordEvent<YourPhaseCompleteEvent>(phaseWinnings) in your GameLogic.
        // Call finalReport on the new event above.
    };

    void addEndingSecondaryCollectorsReport(ReportBuffer& buffer)
    {
        StandardFreeSpinsCompleteEvent::finalSecondaryCollectorsMeter.addReport(
            buffer, "STANDARD FREE SPINS FINAL SECONDARY COLLECTORS", "FinalSecondaryCollectors");
    }

    void addPayTableReports(ReportBuffer& buffer, const uint64_t simGames, const int bet,
                            const std::vector<std::string>& symbolNames)
    {
        WinAnalysisEvent::reportRTP(buffer, simGames, bet, symbolNames);
        WinAnalysisEvent::reportHits(buffer, simGames, symbolNames);
    }

    void addMaxWinReport(ReportBuffer& buffer, const uint64_t simGames, const int bet)
    {
        GameCycleCompleteEvent::maxWinReport(buffer, simGames, bet);
        BaseGameCompleteEvent::maxWinReport(buffer, simGames, bet);
        FreeSpinsCompleteEvent::maxWinReport(buffer, simGames, bet);
        BasePayAnywherePhaseCompleteEvent::maxWinReport(buffer, simGames, bet);
        BaseCoinsPhaseCompleteEvent::maxWinReport(buffer, simGames, bet);
        // Follow instructions in outputGeneralStatsReport to add stats for other phases here.
    }

    void addSpinsReport(ReportBuffer& buffer, const uint64_t simGames, const int bet)
    {
        const uint64_t totalBet = static_cast<uint64_t>(simGames) * static_cast<uint64_t>(bet);
        FreeSpinsCompleteEvent::appendInitialSpinsReport(buffer, totalBet);
        FreeSpinsCompleteEvent::appendFinalSpinsReport(buffer, totalBet);
        // To add other phases here the event must derive from SpinsCompleteTemplate.
        // (Copy from FreeSpinsComplete event.)
        // Then follow instructions in outputGeneralStatsReport.
    }

    void outputWinCappingReport(ReportBuffer& buffer, const uint64_t simGames, const int bet,
                                const unsigned int currentWinCap)
    {
        addWinCapReport(buffer, simGames, bet, currentWinCap);
    }

    void addWinCapReport(ReportBuffer& buffer,
                         const uint64_t simGames,
                         const int bet,
                         const unsigned int currentWinCap)
    {
        const unsigned int winCap[18] = {
            2'000, 3'000, 4'000, 5'000, 6'000, 7'000, 8'000, 9'000, 10'000, 11'000, 12'000, 13'000, 14'000,
            15'000, 20'000, 30'000, 40'000, 50'000
        };
        int scaledWinCap[18] = {};
        const unsigned int currentScaledCap = currentWinCap * bet;
        for (int i = 0; i < 18; ++i) scaledWinCap[i] = winCap[i] * bet;
        uint64_t hitCount[18] = {};
        uint64_t currentCapHitCount = 0;
        uint64_t cappedTotalWin[18] = {};
        uint64_t currentCapTotalWin = 0;
        uint64_t totalWin = 0;
        for (const auto& [win, count] : GameCycleCompleteEvent::eventWinDistribution.getSortedData(true))
        {
            totalWin += static_cast<uint64_t>(win) * count;
            if (currentWinCap > 0)
            {
                if (win >= static_cast<int>(currentScaledCap))
                {
                    currentCapHitCount += count;
                    currentCapTotalWin += static_cast<uint64_t>(currentScaledCap) * count;
                }
                else
                    currentCapTotalWin += static_cast<uint64_t>(win) * count;
            }
            for (int i = 0; i < 18; ++i)
            {
                if (win >= scaledWinCap[i])
                {
                    hitCount[i] += count;
                    cappedTotalWin[i] += static_cast<uint64_t>(scaledWinCap[i]) * count;
                }
                else
                    cappedTotalWin[i] += static_cast<uint64_t>(win) * count;
            }
        }

        // Headers
        std::vector<std::string> headers;
        headers.emplace_back("");
        if (currentWinCap > 0) headers.emplace_back(addThousandsSeparator(currentWinCap));
        for (int i = 0; i < 18; ++i) headers.emplace_back(addThousandsSeparator(winCap[i]));

        // Capped RTP
        {
            std::vector<std::string> row;
            row.emplace_back("Capped RTP");
            if (currentWinCap > 0)
                row.emplace_back(buffer.toPercent(
                    static_cast<long double>(currentCapTotalWin) / (simGames * static_cast<long
                        double>(bet)), 2));
            for (int i = 0; i < 18; ++i)
                row.emplace_back(buffer.toPercent(
                    static_cast<long double>(cappedTotalWin[i]) / (simGames * static_cast<long
                        double>(bet)), 2));
            buffer.addRow("WIN CAP REPORT", headers, row);
        }
        // RTP lost
        {
            std::vector<std::string> row;
            row.emplace_back("RTP lost");
            if (currentWinCap > 0)
                row.emplace_back(buffer.toPercent(
                    static_cast<long double>(totalWin - currentCapTotalWin) / (simGames * static_cast
                        <long double>(bet))));
            for (int i = 0; i < 18; ++i)
                row.emplace_back(buffer.toPercent(
                    static_cast<long double>(totalWin - cappedTotalWin[i]) / (simGames * static_cast<
                        long double>(bet))));
            buffer.addRow("WIN CAP REPORT", headers, row);
        }
        // Capped 1 in
        {
            std::vector<std::string> row;
            row.emplace_back("Capped 1 in");
            if (currentWinCap > 0)
                row.emplace_back(currentCapHitCount
                                     ? addThousandsSeparator(simGames / currentCapHitCount)
                                     : std::string("Inf"));
            for (int i = 0; i < 18; ++i)
                row.emplace_back(hitCount[i]
                                     ? addThousandsSeparator(simGames / hitCount[i])
                                     : std::string("Inf"));
            buffer.addRow("WIN CAP REPORT", headers, row);
        }
    }
}
