#pragma once
/**
 * @file GameCycleCompleteEvent.h
 * @brief Event emitted when a full game cycle completes.
 */

#include "eventTemplates/PhaseCompleteTemplate.h"

/**
 * @brief Phase completion event for the entire game cycle.
 */
class GameCycleCompleteEvent : public PhaseCompleteTemplate<GameCycleCompleteEvent>
{
public:
	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Full Game";
	inline static bool countsAsSpinForRnStream = false;

	/**
	 * @param totalWin Total won in simulation.
	 */
	explicit GameCycleCompleteEvent(const int totalWin) : PhaseCompleteTemplate(totalWin)
	{
	}
};
