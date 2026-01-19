#pragma once
/**
 * @file BaseGameCompleteEvent.h
 * @brief Event emitted when the base game phase is complete.
 */

#include "eventTemplates/PhaseCompleteTemplate.h"

/**
 * @brief Phase completion event for the base game.
 */
class BaseGameCompleteEvent : public PhaseCompleteTemplate<BaseGameCompleteEvent>
{
public:
    /**
     * @param totalWin Total won in simulation.
     */
    explicit BaseGameCompleteEvent(const int totalWin) : PhaseCompleteTemplate(totalWin)
    {
    }

    inline static std::string phaseName = "Base Game";
};
