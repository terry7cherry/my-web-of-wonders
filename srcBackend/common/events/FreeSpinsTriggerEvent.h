#pragma once

#include "eventTemplates/BasicEvent.h"

class FreeSpinsTriggerEvent : public BasicEvent
{
public:
	explicit FreeSpinsTriggerEvent(const int spins, const int multiplier = 1)
		: spins(spins), multiplier(multiplier)
	{
	}

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
