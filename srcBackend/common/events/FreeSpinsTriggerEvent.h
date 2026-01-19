#pragma once
/**
 * @file FreeSpinsTriggerEvent.h
 * @brief Event emitted when free spins are initially triggered. For frontend.
 */

#include "eventTemplates/BasicEvent.h"

/**
 * @brief Captures the number of awarded free spins and multiplier on trigger.
 * @details Handled automatically on the frontend.
 */
class FreeSpinsTriggerEvent : public BasicEvent
{
public:
	/**
	 * @param spins Num spins awarded.
	 * @param multiplier Optional win multiplier for awarded spins.
	 */
	explicit FreeSpinsTriggerEvent(const int spins, const int multiplier = 1)
		: spins(spins), multiplier(multiplier)
	{
	}

	/**
	 * @brief Optional metrics hook.
	 * @details Note this is required because updateMeters arguments must match constructor.
	 * @param spins Num spins awarded.
	 * @param multiplier Optional win multiplier for awarded spins.
	 */
	static void updateMeters(const int spins [[maybe_unused]], const int multiplier [[maybe_unused]] = 1)
	{
	}

protected:
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "freeSpinsTrigger";
		eventDetails["spins"] = spins;
		eventDetails["multiplier"] = multiplier;
		return eventDetails;
	}

private:
	const int spins;
	const int multiplier;
};
