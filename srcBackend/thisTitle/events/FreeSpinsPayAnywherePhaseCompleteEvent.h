#pragma once

#include "../../common/events/eventTemplates/PhaseCompleteTemplate.h"

class FreeSpinsPayAnywherePhaseCompleteEvent : public PhaseCompleteTemplate<
        FreeSpinsPayAnywherePhaseCompleteEvent>
{
public:
    inline static std::string phaseName = "Free Spins Pay Anywhere Wins";
    inline static bool multipleHitsPerCycle = true;

    explicit FreeSpinsPayAnywherePhaseCompleteEvent(const int totalWins, const bool repeatEvent = false) :
        PhaseCompleteTemplate(totalWins, repeatEvent)
    {
    }
};
