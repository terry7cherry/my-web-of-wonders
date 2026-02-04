#pragma once
/**
 * @file PhaseCompleteTemplate.h
 * @brief Template for phase-completion events with shared meters and reporting.
 */

#include <string>
#include <inttypes.h>
#include <filesystem>

#include "BasicEvent.h"
#include "../threadSafeMeters/IntSumMeter.h"
#include "../threadSafeMeters/DistributionMeter.h"
#include "../threadSafeMeters/IntMaxMeter.h"

/**
 * @brief Base for Events signalling end of phase (base game, bonus, free spins, whole game).
 * @attention When extending this class, you MUST pass the child class name as a template parameter to get separate stats. You must also set phaseName (use GameCycleCompleteEvent as a guide).
 * @tparam T Derived event type providing `phaseName`.
 */
template <typename T>
class PhaseCompleteTemplate : public BasicEvent
{
public:
    /**
     * @param phaseWin Win for the completed phase.
     */
    explicit PhaseCompleteTemplate(const int phaseWin) : totalWin(phaseWin)
    {
    }

    /**
     * @brief Update aggregate meters for the phase.
     * @param phaseWin Win for the completed phase.
     */
    static void updateMeters(const int phaseWin)
    {
        ++eventTotalHits;
        //eventWinDistribution.add(totalWin);
        eventWinDistribution[phaseWin]++;
        if (phaseWin)
        {
            eventTotalWin += phaseWin;
            ++eventTotalWinningHits;
            maxWinMeter.update(phaseWin);
        }
    }

    inline static DistributionMeter eventWinDistribution{};
    inline static IntSumMeter eventTotalWin{};
    inline static IntSumMeter eventTotalHits{};
    inline static IntSumMeter eventTotalWinningHits{};
    inline static IntMaxMeter maxWinMeter{};

    /**
     * @brief Write the phase win distribution to a CSV file.
     * @param betType Bet type (for filename).
     * @param rtp RTP variant (for filename).
     * @param bet Bet amount (for filename).
     * @param descendingOrder Sort order for distribution output.
     * @param doConsoleProgress Print progress messages to console?
     * @param outputFolder Folder to store file.
     */
    static void outputDistribution(const std::string& betType,
                                   const int rtp,
                                   const int bet,
                                   const bool descendingOrder,
                                   const bool doConsoleProgress,
                                   const std::filesystem::path& outputFolder
    )
    {
        // remove spaces from phaseName
        std::string fileName = T::phaseName;
        std::erase(fileName, ' ');
        // add bet type to file name if not "default"
        if (betType != "default") fileName += "_" + betType;
        // add RTP and bet to file name.
        fileName += "_" + std::to_string(rtp) + "_" + std::to_string(bet) + ".csv";

        std::ofstream outfile;
        const std::filesystem::path filePath = outputFolder / fileName;

        outfile.open(filePath);
        if (!outfile.is_open())
        {
            reportError("Distribution.fileOutput", "Can't open file: " + fileName + ". Output aborted",
                        "Warning");
            return;
        }

        if (doConsoleProgress)
            std::cout << "Writing Distribution file: " << fileName << ". . .";

        const auto sortedData = eventWinDistribution.getSortedData(descendingOrder);

        for (const auto& [win, count] : sortedData)
        {
            outfile << std::to_string(win) << ", " << std::to_string(count) << std::endl;
        }

        outfile.close();

        if (doConsoleProgress)
            std::cout << "Done\r\n";
    }

    /**
     * @brief Print a final summary report of RTP and hit/win statistics.
     * @param plays Number of plays simulated.
     * @param betAmount Bet amount per play.
     */
    static void finalReport(const uint64_t plays, const int betAmount)
    {
        const uint64_t hitCount = eventTotalHits.global();
        const uint64_t winCount = eventTotalWinningHits.global();
        const uint64_t totalWin = eventTotalWin.global();
        const uint64_t totalBet = plays * betAmount;

        std::printf("%20s,", T::phaseName.c_str()); // Event
        std::printf("%11.2f%%,", static_cast<long double>(totalWin) * 100.0 / totalBet); // RTP
        std::printf("%12.2f,", static_cast<long double>(plays) / (hitCount ? hitCount : 1)); // HitRate
        std::printf("%12.2f,", hitCount ? static_cast<long double>(totalWin) / hitCount / betAmount : 0);
        // AvgPay
        std::printf("%12.2f,", winCount ? static_cast<long double>(totalWin) / winCount / betAmount : 0);
        // AvgWin
        std::printf("%11.2f%%,", hitCount ? static_cast<long double>(winCount) * 100.0 / hitCount : 0);
        // WinChance
        std::printf("%12.2f\n", winCount ? static_cast<long double>(hitCount) / winCount : 0); // WinRate
    }

    /**
     * @brief Print the maximum win and frequencies.
     * @param plays Number of plays simulated.
     * @param betAmount Bet amount per play.
     */
    static void maxWinReport(const uint64_t plays, const int betAmount)
    {
        const uint64_t hitCount = eventTotalHits.global();
        const uint64_t maxWin = maxWinMeter.globalMax;
        const uint64_t maxWinCount = maxWinMeter.globalCount;
        std::printf("%20s,", T::phaseName.c_str()); // Event
        std::printf("%12.2f,", static_cast<long double>(maxWin) / betAmount); // MaxWin
        std::printf("%12" PRIu64 ",",
                    maxWinCount ? static_cast<uint64_t>(plays / maxWinCount) : static_cast<uint64_t>(0));
        // HitRate
        std::printf("%12" PRIu64 "\n",
                    maxWinCount ? static_cast<uint64_t>(hitCount / maxWinCount) : static_cast<uint64_t>(0));
        // EventRate
    }

protected :
    /** @brief Name used for output to console and file. */
    inline static std::string phaseName = "Template";
    const int totalWin;
};
