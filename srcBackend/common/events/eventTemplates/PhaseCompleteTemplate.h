#pragma once

#include <string>
#include "BasicEvent.h"
#include "../threadSafeMeters/IntSumMeter.h"
#include "../threadSafeMeters/DistributionMeter.h"
#include "../threadSafeMeters/IntMaxMeter.h"

template <typename T>
class PhaseCompleteTemplate : public BasicEvent
{
public:
    explicit PhaseCompleteTemplate(const int totalWin) : totalWin(totalWin)
    {
    }

    static void updateMeters(const int totalWin)
    {
        ++eventTotalHits;
        eventWinDistribution.add(totalWin);
        if (totalWin)
        {
            eventTotalWin += totalWin;
            ++eventTotalWinningHits;
            maxWinMeter.update(totalWin);
        }
    }

    inline static DistributionMeter eventWinDistribution{};
    inline static IntSumMeter eventTotalWin{};
    inline static IntSumMeter eventTotalHits{};
    inline static IntSumMeter eventTotalWinningHits{};
    inline static IntMaxMeter maxWinMeter{};

    static void outputDistribution(const std::string& betType,
                                   const int rtp,
                                   const int bet,
                                   const bool descendingOrder,
                                   const bool doConsoleProgress)
    {
        std::ofstream outfile;
        // remove spaces from phaseName
        std::string fileName = T::phaseName;
        std::erase(fileName, ' ');
        if (betType != "default") fileName += "_" + betType;
        fileName += "_" + std::to_string(rtp) + "_" + std::to_string(bet) + ".csv";
        const std::filesystem::path filePath = std::filesystem::path(GAME_SOURCE_DIR) / "Distributions" /
            fileName;

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

    static void finalReport(const uint64_t plays, const int betAmount)
    {
        const uint64_t hitCount = eventTotalHits.global();
        const uint64_t winCount = eventTotalWinningHits.global();
        const uint64_t totalWin = eventTotalWin.global();
        const uint64_t totalBet = plays * betAmount;

        std::printf("%20s,", T::phaseName.c_str()); // Event
        std::printf("%11.2f%%,", static_cast<double>(totalWin) * 100.0 / totalBet); // RTP
        std::printf("%12.2f,", static_cast<double>(plays) / (hitCount ? hitCount : 1)); // HitRate
        std::printf("%12.2f,", hitCount ? static_cast<double>(totalWin) / hitCount / betAmount : 0); // AvgPay
        std::printf("%12.2f,", winCount ? static_cast<double>(totalWin) / winCount / betAmount : 0); // AvgWin
        std::printf("%11.2f%%,", hitCount ? static_cast<double>(winCount) * 100.0 / hitCount : 0);
        // WinChance
        std::printf("%12.2f\n", winCount ? static_cast<double>(hitCount) / winCount : 0); // WinRate
    }

    static void maxWinReport(const uint64_t plays, const int betAmount)
    {
        const uint64_t hitCount = eventTotalHits.global();
        const uint64_t maxWin = maxWinMeter.globalMax;
        const uint64_t maxWinCount = maxWinMeter.globalCount;
        std::printf("%20s,", T::phaseName.c_str()); // Event
        std::printf("%12.2f,", static_cast<long double>(maxWin) / betAmount); // MaxWin
        std::printf("%12llu,", maxWinCount ? static_cast<unsigned long long>(plays / maxWinCount) : 0ULL);
        // HitRate
        std::printf("%12llu\n",
                    maxWinCount ? static_cast<unsigned long long>(hitCount / maxWinCount) : 0ULL);
        // EventRate
    }

protected :
    inline static std::string phaseName = "Template";

private :
    const int totalWin;
};
