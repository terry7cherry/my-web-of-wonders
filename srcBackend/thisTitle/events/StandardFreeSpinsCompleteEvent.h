#pragma once
/**
 * @file StandardFreeSpinsCompleteEvent.h
 * @brief Event emitted when the free spins phase completes.
 */

#include "../../common/events/eventTemplates/SpinsCompleteTemplate.h"
#include "../../common/events/threadSafeMeters/DistributionMeter.h"
#include "../state/GameCycle.h"

/**
 * @brief Phase completion event for Free Spins.
 * @details Handled automatically on frontend.
 */
class StandardFreeSpinsCompleteEvent : public SpinsCompleteTemplate<StandardFreeSpinsCompleteEvent>
{
public:
	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Standard Free Spins";

	inline static DistributionMeter finalSecondaryCollectorsMeter{};

	/**
	 * @param cycle Game cycle event including totalWin, fsInitialSpins, and fsSpinsPlayed properties.
	 */
	explicit StandardFreeSpinsCompleteEvent(const GameCycle& cycle)
		: StandardFreeSpinsCompleteEvent(cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed,
		                                 cycle.fsFinalSecondaryCollectors)
	{
	}

	/**
	 * @param totalWin Win from free spins.
	 * @param initialSpins Free spins awarded from base game.
	 * @param finalSpins Number of spins played including retriggers.
	 * @param finalSecondaryCollectors Number of secondary collectors activated by end of free spins.
	 */
	StandardFreeSpinsCompleteEvent(
		const unsigned int totalWin,
		const unsigned int initialSpins,
		const unsigned int finalSpins,
		const unsigned int finalSecondaryCollectors
	) : SpinsCompleteTemplate(totalWin, initialSpins, finalSpins)
	{
	}

	static void updateMeters(const GameCycle& cycle)
	{
		SpinsCompleteTemplate::updateMeters(
			cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed, cycle.fsFinalSecondaryCollectors);
	}

	static void updateMeters(const unsigned int win, const unsigned int initialSpins,
	                         const unsigned int finalSpins, const unsigned int finalSecondaryCollectors)
	{
		SpinsCompleteTemplate::updateMeters(win, initialSpins, finalSpins);
		finalSecondaryCollectorsMeter.add(finalSecondaryCollectors);
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
