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
	inline static DistributionMeter mainCollectorPositionMeter{};
	inline static DistributionMeter mainCollectorPositionWinningsMeter{};

	/**
	 * @param cycle Game cycle event including totalWin, fsInitialSpins, and fsSpinsPlayed properties.
	 */
	explicit StandardFreeSpinsCompleteEvent(const GameCycle& cycle)
		: StandardFreeSpinsCompleteEvent(cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed,
		                                 cycle.fsFinalSecondaryCollectors,
		                                 cycle.session.mainCollectorPosition)
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
		const unsigned int finalSecondaryCollectors,
		const CollectorPosition mainCollectorPosition
	) : SpinsCompleteTemplate(totalWin, initialSpins, finalSpins)
	{
	}

	static void updateMeters(const GameCycle& cycle)
	{
		SpinsCompleteTemplate::updateMeters(cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed);
		finalSecondaryCollectorsMeter.add(cycle.fsFinalSecondaryCollectors);
		mainCollectorPositionMeter.add(cycle.session.mainCollectorPosition);
		mainCollectorPositionWinningsMeter.add(cycle.session.mainCollectorPosition, cycle.freeSpinsWin);
	}

	static void updateMeters(const unsigned int win, const unsigned int initialSpins,
	                         const unsigned int finalSpins, const unsigned int finalSecondaryCollectors,
	                         const CollectorPosition mainCollectorPosition)
	{
		SpinsCompleteTemplate::updateMeters(win, initialSpins, finalSpins);
		finalSecondaryCollectorsMeter.add(finalSecondaryCollectors);
		mainCollectorPositionMeter.add(mainCollectorPosition);
		mainCollectorPositionWinningsMeter.add(mainCollectorPosition, win);
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
