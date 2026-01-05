#pragma once

#include "BasicEvent.h"

class NewFreeSpinEvent : public BasicEvent
{
public:
	explicit NewFreeSpinEvent(const int multiplier = 1) : multiplier(multiplier)
	{
	}

	static void updateMeters(const int multiplier [[maybe_unused]] = 1)
	{
	}

protected:
	static constexpr std::string_view EventType = "newFreeSpin";

	std::string_view getEventType() const override
	{
		return EventType;
	}

	Json::Value getDetailsAsJson() const override
	{
		Json::Value eventDetails;
		eventDetails["multiplier"] = multiplier;
		return eventDetails;
	}

private:
	const int multiplier;
};
