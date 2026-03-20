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
	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Free Spins";
    
    // Reset all meters for a fresh session aggregation
    static void reset()
    {
        // Reset phase template meters
        SpinsCompleteTemplate<FreeSpinsCompleteEvent>::initialSpinsMeter = DistributionMeter();
        SpinsCompleteTemplate<FreeSpinsCompleteEvent>::finalSpinsMeter = DistributionMeter();
        SpinsCompleteTemplate<FreeSpinsCompleteEvent>::initialSpinsWinsMeter = DistributionMeter();
        SpinsCompleteTemplate<FreeSpinsCompleteEvent>::finalSpinsWinsMeter = DistributionMeter();
        // Reset base phase meters
        PhaseCompleteTemplate<FreeSpinsCompleteEvent>::eventWinDistribution = DistributionMeter();
        PhaseCompleteTemplate<FreeSpinsCompleteEvent>::eventTotalWin = IntSumMeter();
        PhaseCompleteTemplate<FreeSpinsCompleteEvent>::eventTotalHits = IntSumMeter();
        PhaseCompleteTemplate<FreeSpinsCompleteEvent>::eventTotalCycleHits = IntSumMeter();
        PhaseCompleteTemplate<FreeSpinsCompleteEvent>::eventTotalWinningHits = IntSumMeter();
        PhaseCompleteTemplate<FreeSpinsCompleteEvent>::maxWinMeter = IntMaxMeter();
    }

	/**
	 * @param cycle Game cycle event including totalWin, fsInitialSpins, and fsSpinsPlayed properties.
	 */
	explicit FreeSpinsCompleteEvent(const GameCycle& cycle)
		: FreeSpinsCompleteEvent(cycle.freeSpinsWin, cycle.fsInitialSpins, cycle.fsSpinsPlayed)
	{
	}

	/**
	 * @param totalWin Win from free spins.
	 * @param initialSpins Free spins awarded from base game.
	 * @param finalSpins Number of spins played including retriggers.
	 */
	FreeSpinsCompleteEvent(
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
