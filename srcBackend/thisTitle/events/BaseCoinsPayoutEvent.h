#pragma once

#include "../../common/events/eventTemplates/PhaseCompleteTemplate.h"

class BaseCoinsPayoutEvent : public PhaseCompleteTemplate<BaseCoinsPayoutEvent>
{
public:
    inline static std::string phaseName = "Single Base Coin Payout";
    inline static bool multipleHitsPerCycle = true;

    explicit BaseCoinsPayoutEvent(const int totalWins, const bool repeatEvent = false) :
        PhaseCompleteTemplate(
            totalWins, repeatEvent)
    {
    }
};
