#pragma once
/**
 * @file FreeSpinsRetriggerEvent.h
 * @brief Event emitted when free spins are retriggered.
 * @details Currently for front end only.
 * @details Some retrigger stats are output by FreeSpinsCompleteEvent.
 */

#include "eventTemplates/BasicEvent.h"

/**
 * @brief Captures additional spins and multiplier awarded during free spins.
 * @details Handled automatically on frontend.
 */
class FreeSpinsRetriggerEvent : public BasicEvent
{
public:
	const int spins;
	const int multiplier;

	/**
	 * @param spins Retrigger spins awarded
	 * @param multiplier Optional multiplier for retrigger spins awarded.
	 */
	explicit FreeSpinsRetriggerEvent(const int spins, const int multiplier = 1)
		: spins(spins), multiplier(multiplier)
	{
	}

	/**
	 * @brief Empty metrics hook.
	 * @details Note this is required because updateMeters arguments must match constructor.
	 * @param spins Retrigger spins awarded
	 * @param multiplier Optional multiplier for retrigger spins awarded.
	 */
	static void updateMeters(const int spins [[maybe_unused]], const int multiplier [[maybe_unused]] = 1)
	{
	}

protected:
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "freeSpinsRetrigger";
		eventDetails["spins"] = spins;
		eventDetails["multiplier"] = multiplier;
		return eventDetails;
	}
};
