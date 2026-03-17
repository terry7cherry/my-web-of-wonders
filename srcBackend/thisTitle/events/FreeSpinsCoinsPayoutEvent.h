#pragma once

#include "../../common/events/eventTemplates/PhaseCompleteTemplate.h"

class FreeSpinsCoinsPayoutEvent : public PhaseCompleteTemplate<FreeSpinsCoinsPayoutEvent>
{
public:
    inline static std::string phaseName = "Single Free Spins Coin Payout";
    inline static bool multipleHitsPerCycle = true;

    explicit FreeSpinsCoinsPayoutEvent(const int totalWins, const bool repeatEvent = false) :
        PhaseCompleteTemplate(
            totalWins, repeatEvent)
    {
    }
};
