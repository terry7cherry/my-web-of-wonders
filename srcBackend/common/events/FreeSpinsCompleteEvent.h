#pragma once

#include "./eventTemplates/SpinsCompleteTemplate.h"

class FreeSpinsCompleteEvent : public SpinsCompleteTemplate<FreeSpinsCompleteEvent>
{
public:
	FreeSpinsCompleteEvent(
		const int totalWin,
		const int initialSpins,
		const int finalSpins
	) : SpinsCompleteTemplate(totalWin, initialSpins, finalSpins)
	{
	}

	inline static std::string phaseName = "Free Spins";

protected :
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "freeSpinsComplete";
		return eventDetails;
	}
};
