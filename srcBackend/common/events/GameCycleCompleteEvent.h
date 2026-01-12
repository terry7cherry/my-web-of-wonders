#pragma once

#include "eventTemplates/PhaseCompleteTemplate.h"

class GameCycleCompleteEvent : public PhaseCompleteTemplate<GameCycleCompleteEvent>
{
public:
    explicit GameCycleCompleteEvent(const int totalWin) : PhaseCompleteTemplate(totalWin)
    {
    }

    inline static std::string phaseName = "Full Game";
};
