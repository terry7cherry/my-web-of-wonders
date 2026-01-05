#pragma once

#include "BasicEvent.h"

class FreeSpinsEndEvent : public BasicEvent
{
public:
	explicit FreeSpinsEndEvent(const int totalWin) : totalWin(totalWin)
	{
	}

	static void updateMeters(const int totalWin [[maybe_unused]])
	{
	}

protected:
	static constexpr std::string_view EventType = "freeSpinsEnd";

	std::string_view getEventType() const override
	{
		return EventType;
	}

	Json::Value getDetailsAsJson() const override
	{
		Json::Value eventDetails;
		eventDetails["totalWin"] = totalWin;
		return eventDetails;
	}

private:
	const int totalWin;
};
