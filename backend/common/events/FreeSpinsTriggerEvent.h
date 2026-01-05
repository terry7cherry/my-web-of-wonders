#pragma once

#include "BasicEvent.h"

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
	static constexpr std::string_view EventType = "freeSpinsTrigger";

	std::string_view getEventType() const override
	{
		return EventType;
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
