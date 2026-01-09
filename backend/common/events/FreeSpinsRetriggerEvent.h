#pragma once

#include "BasicEvent.h"

class FreeSpinsRetriggerEvent : public BasicEvent
{
public:
	explicit FreeSpinsRetriggerEvent(const int spins, const int multiplier = 1)
		: spins(spins), multiplier(multiplier)
	{
	}

	static void updateMeters(const int spins [[maybe_unused]], const int multiplier [[maybe_unused]] = 1)
	{
	}

protected:
	std::string_view getEventType() const override
	{
		return "freeSpinsRetrigger";
	}

	Json::Value getDetailsAsJson() const override
	{
		Json::Value eventDetails;
		eventDetails["spins"] = spins;
		eventDetails["multiplier"] = multiplier;
		return eventDetails;
	}

private:
	const int spins;
	const int multiplier;
};
