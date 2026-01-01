#pragma once

#include "BasicEvent.h"

class GameCycleCompleteEvent : public BasicEvent
{
public:
    explicit GameCycleCompleteEvent(const int totalWin) : totalWin(totalWin)
    {
    }

    static void updateMeters(const int totalWin)
    {
        eventTotalWin += totalWin;
    }

    inline static int64_t eventTotalWin = 0;

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
