#pragma once

#include <string>
#include "BasicEvent.h"
#include "threadSafeMeters/IntMeter.h"
#include "threadSafeMeters/DistributionMeter.h"

template <typename T>
class PhaseCompleteEvent : public BasicEvent
{
public:
    explicit PhaseCompleteEvent(const int totalWin) : totalWin(totalWin)
    {
    }

    static void updateMeters(const int totalWin)
    {
        ++eventTotalHits;
        if (totalWin)
        {
            eventTotalWin += totalWin;
            ++eventTotalWinningHits;
        }
        eventWinDistribution.add(totalWin);
    }

    inline static DistributionMeter eventWinDistribution{};
    inline static IntMeter eventTotalWin{};
    inline static IntMeter eventTotalHits{};
    inline static IntMeter eventTotalWinningHits{};

    static void outputDistribution(const bool descendingOrder, const bool doConsoleProgress)
    {
        eventWinDistribution.fileOutput(descendingOrder, doConsoleProgress);
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

protected :
    std::string_view getEventType() const override { return "gameSectionComplete"; }

    inline static std::string phaseName = "Template";

    Json::Value getDetailsAsJson() const override
    {
        Json::Value eventDetails;
        eventDetails["totalWin"] = totalWin;
        return eventDetails;
    }

private :
    const int totalWin;
};
