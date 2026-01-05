#pragma once
//#define SIMPLE
//#define SINGLETHREAD

#include "BasicEvent.h"
#include "threadSafeMeters/IntMeter.h"
#include "threadSafeMeters/DistributionMeter.h"

class GameCycleCompleteEvent : public BasicEvent
{
public:
    explicit GameCycleCompleteEvent(const int totalWin) : totalWin(totalWin)
    {
    }

    static void updateMeters(const int totalWin)
    {
#ifdef SIMPLE
#ifdef SINGLETHREAD
        eventTotalCycles++;
        if (totalWin) eventTotalWin += totalWin;
#else
        eventTotalCycles.fetch_add(1);
        if (totalWin) eventTotalWin.fetch_add(totalWin);
#endif
#else
        ++eventTotalCycles;
        if (totalWin)
            eventTotalWin += totalWin;
#endif
        if (totalWin)
            eventWinDistribution.add(totalWin);
    }

    inline static DistributionMeter eventWinDistribution{};
#ifdef SIMPLE
#ifdef SINGLETHREAD
    inline static uint64_t eventTotalWin = 0;
    inline static uint64_t eventTotalCycles = 0;
#else
    inline static std::atomic<uint64_t> eventTotalWin{0};
    inline static std::atomic<uint64_t> eventTotalCycles{0};
#endif
#else
    inline static IntMeter eventTotalWin{};
    inline static IntMeter eventTotalCycles{};
#endif

    static void outputDistribution(const bool descendingOrder, const bool doConsoleProgress)
    {
        eventWinDistribution.fileOutput(eventTotalCycles.global(), descendingOrder, doConsoleProgress);
    }

protected:
    static constexpr std::string_view EventType = "gameCycleComplete";
    std::string_view getEventType() const override { return EventType; }

    Json::Value getDetailsAsJson() const override
    {
        Json::Value eventDetails;
        eventDetails["totalWin"] = totalWin;
        return eventDetails;
    }

private:
    const int totalWin;
};
