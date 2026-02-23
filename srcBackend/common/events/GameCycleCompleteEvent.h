#pragma once
/**
 * @file GameCycleCompleteEvent.h
 * @brief Event emitted when a full game cycle completes.
 */

#include "eventTemplates/PhaseCompleteTemplate.h"
#include "../gameMaths/GameMaths.h"

/**
 * @brief Phase completion event for the entire game cycle.
 */
class GameCycleCompleteEvent : public PhaseCompleteTemplate<GameCycleCompleteEvent>
{
public:
	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Full Game";
	inline static bool countsAsSpinForRnStream = false;

	inline static IntSumMeter cappedTotalWin{};
	inline static IntSumMeter capHitCount{};

	/**
	 * @param totalWin Total won in simulation.
	 */
	explicit GameCycleCompleteEvent(const int totalWin) : PhaseCompleteTemplate(totalWin)
	{
	}

	/**
	 * @param totalWin Total won in simulation.
	 * @param maths GameMaths object (for win cap).
	 * @param bet Bet level for session.
	 */
	explicit GameCycleCompleteEvent(const int totalWin,
	                                const GameMaths& maths [[maybe_unused]],
	                                const unsigned int bet [[maybe_unused]]) : PhaseCompleteTemplate(totalWin)
	{
	}

	static void updateMeters(const int phaseWin)
	{
		PhaseCompleteTemplate<GameCycleCompleteEvent>::updateMeters(phaseWin);
		cappedTotalWin.increment(phaseWin);
	}

	static void updateMeters(const unsigned int phaseWin, const GameMaths& maths, const unsigned int bet)
	{
		PhaseCompleteTemplate<GameCycleCompleteEvent>::updateMeters(phaseWin);
		if (!maths.ints.contains("winCap"))
		{
			cappedTotalWin.increment(phaseWin);
			return;
		}
		const unsigned int scaledCap = maths.ints["winCap"] * bet;
		if (phaseWin >= scaledCap)
		{
			cappedTotalWin.increment(scaledCap);
			++capHitCount;
		}
		else
			cappedTotalWin.increment(phaseWin);
	}
};
