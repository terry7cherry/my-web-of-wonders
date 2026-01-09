#pragma once

#include "PhaseCompleteEvent.h"

class FreeSpinsCompleteEvent : public PhaseCompleteEvent<FreeSpinsCompleteEvent>
{
public:
	explicit FreeSpinsCompleteEvent(const int totalWin) : PhaseCompleteEvent(totalWin)
	{
	}

	inline static std::string phaseName = "Free Spins";

protected:
	std::string_view getEventType() const override { return "freeSpinsEnd"; }
};
