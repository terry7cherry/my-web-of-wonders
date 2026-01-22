#pragma once
/**
 * @file NewFreeSpinEvent.h
 * @brief Event emitted for each awarded free spin instance.
 */

#include "eventTemplates/BasicEvent.h"

/**
 * @brief New free spin event autohandled in front end.
 */
class NewFreeSpinEvent : public BasicEvent
{
public:
	/**
	 * @param multiplier Optional win multiplier for spin. Displayed in frontend ticker.
	 */
	explicit NewFreeSpinEvent(const int multiplier = 1) : multiplier(multiplier)
	{
	}

	/**
	 * @brief Optional metrics hook.
	 * @param multiplier Optional win multiplier for spin. Displayed in frontend ticker.
	 * @details Note this is required because updateMeters arguments must match constructor.
	 */
	static void updateMeters(const int multiplier [[maybe_unused]] = 1)
	{
	}

protected:
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "newFreeSpin";
		eventDetails["multiplier"] = multiplier;
		return eventDetails;
	}

private:
	const int multiplier;
};
