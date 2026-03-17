#pragma once
/**
 * @file StandardFreeSpinsCompleteEvent.h
 * @brief Event emitted when the free spins phase completes.
 */

#include "../../common/events/eventTemplates/SpinsCompleteTemplate.h"
#include "../state/GameCycle.h"

/**
 * @brief Phase completion event for Free Spins.
 * @details Handled automatically on frontend.
 */
class EnhancedFreeSpins1CompleteEvent : public SpinsCompleteTemplate<EnhancedFreeSpins1CompleteEvent>
{
public:
	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Enhanced Free Spins 1";

	/**
	 * @param cycle Game cycle event including totalWin, fsInitialSpins, and fsSpinsPlayed properties.
	 */
	explicit EnhancedFreeSpins1CompleteEvent(const GameCycle& cycle)
		: EnhancedFreeSpins1CompleteEvent(cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed)
	{
	}

	/**
	 * @param totalWin Win from free spins.
	 * @param initialSpins Free spins awarded from base game.
	 * @param finalSpins Number of spins played including retriggers.
	 */
	EnhancedFreeSpins1CompleteEvent(
		const unsigned int totalWin,
		const unsigned int initialSpins,
		const unsigned int finalSpins
	) : SpinsCompleteTemplate(totalWin, initialSpins, finalSpins)
	{
	}

	static void updateMeters(const GameCycle& cycle)
	{
		SpinsCompleteTemplate::updateMeters(
			cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed);
	}

	static void updateMeters(const unsigned int win, const unsigned int initialSpins,
	                         const unsigned int finalSpins)
	{
		SpinsCompleteTemplate::updateMeters(win, initialSpins, finalSpins);
	}

protected :
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "freeSpinsComplete";
		eventDetails["totalWin"] = totalWin;
		return eventDetails;
	}
};
