#pragma once
/**
 * @file FreeSpinsCompleteEvent.h
 * @brief Event emitted when the free spins phase completes.
 */

#include "./eventTemplates/SpinsCompleteTemplate.h"

/**
 * @brief Phase completion event for Free Spins.
 * @details Handled automatically on frontend.
 */
class FreeSpinsCompleteEvent : public SpinsCompleteTemplate<FreeSpinsCompleteEvent>
{
public:
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

	/** @brief Name used for stats output. **/
	inline static std::string phaseName = "Free Spins";

protected :
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "freeSpinsComplete";
		return eventDetails;
	}
};
