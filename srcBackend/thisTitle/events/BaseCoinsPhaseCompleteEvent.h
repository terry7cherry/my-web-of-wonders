#pragma once

#include "../../common/events/eventTemplates/PhaseCompleteTemplate.h"

class BaseCoinsPhaseCompleteEvent : public PhaseCompleteTemplate<BaseCoinsPhaseCompleteEvent>
{
public:
    inline static std::string phaseName = "Base Coin Wins";
    inline static bool multipleHitsPerCycle = true;

    explicit BaseCoinsPhaseCompleteEvent(const int totalWins, const bool repeatEvent = false) :
        PhaseCompleteTemplate(
            totalWins, repeatEvent)
    {
    }
};
