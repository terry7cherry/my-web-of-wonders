#pragma once
#include "./eventTemplates/BasicEvent.h"
#include "../rng/MathsRng.h"

class SpinCompleteEvent : public BasicEvent
{
public:
    explicit SpinCompleteEvent(const int spinWin)
    {
        mathsRng.endSpinSequence(spinWin);
    }

    static void updateMeters(const int spinWin [[maybe_unused]])
    {
    }
};
