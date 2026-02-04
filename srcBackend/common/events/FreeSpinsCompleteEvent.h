#pragma once
/**
 * @file FreeSpinsCompleteEvent.h
 * @brief Event emitted when the free spins phase completes.
 */

#include "./eventTemplates/SpinsCompleteTemplate.h"
#include "../../thisTitle/state/GameCycle.h"

/**
 * @brief Phase completion event for Free Spins.
 * @details Handled automatically on frontend.
 */
class FreeSpinsCompleteEvent : public SpinsCompleteTemplate<FreeSpinsCompleteEvent>
{
public:
	/**
	 * @param cycle Game cycle event including totalWin, fsInitialSpins, and fsSpinsPlayed properties.
	 */
	explicit FreeSpinsCompleteEvent(
		const GameCycle& cycle)
		: FreeSpinsCompleteEvent(cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed)
	{
	}

	/**
	 * @param totalWin Win from free spins.
	 * @param initialSpins Free spins awarded from base game.
	 * @param finalSpins Number of spins played including retriggers.
	 */
	FreeSpinsCompleteEvent(
		const int totalWin,
		const int initialSpins,
		const int finalSpins
	) : SpinsCompleteTemplate(totalWin, initialSpins, finalSpins)
	{
	}

	static void updateMeters(const GameCycle& cycle)
	{
		SpinsCompleteTemplate::updateMeters(
			cycle.totalWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed);
	}

	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Free Spins";

protected :
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "freeSpinsComplete";
		eventDetails["totalWin"] = totalWin;
		return eventDetails;
	}
};
