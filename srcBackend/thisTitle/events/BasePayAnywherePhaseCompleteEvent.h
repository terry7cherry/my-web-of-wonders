#pragma once

#include "../../common/events/eventTemplates/PhaseCompleteTemplate.h"

class BasePayAnywherePhaseCompleteEvent : public PhaseCompleteTemplate<BasePayAnywherePhaseCompleteEvent>
{
public:
    inline static std::string phaseName = "Base Pay Anywhere Wins";
    inline static bool multipleHitsPerCycle = true;

    explicit BasePayAnywherePhaseCompleteEvent(const int totalWins, const bool repeatEvent = false) :
        PhaseCompleteTemplate(totalWins, repeatEvent)
    {
    }
};
