#pragma once

#include "eventTemplates/PhaseCompleteTemplate.h"

class BaseGameCompleteEvent : public PhaseCompleteTemplate<BaseGameCompleteEvent>
{
public:
    explicit BaseGameCompleteEvent(const int totalWin) : PhaseCompleteTemplate(totalWin)
    {
    }

    inline static std::string phaseName = "Base Game";

protected :
    std::string_view getEventType() const override { return "baseGameComplete"; }
};
