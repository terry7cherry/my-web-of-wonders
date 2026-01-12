#pragma once

#include "eventTemplates/BasicEvent.h"

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
