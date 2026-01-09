#pragma once

#include "BasicEvent.h"
#include "threadSafeMeters/IntMeter.h"
#include "threadSafeMeters/DistributionMeter.h"
#include "PhaseCompleteEvent.h"

class GameCycleCompleteEvent : public PhaseCompleteEvent<GameCycleCompleteEvent>
{
public:
    explicit GameCycleCompleteEvent(const int totalWin) : PhaseCompleteEvent(totalWin)
    {
    }

    inline static std::string phaseName = "Game";

protected :
    std::string_view getEventType() const override { return "gameCycleComplete"; }
};
