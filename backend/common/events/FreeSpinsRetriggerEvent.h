#pragma once

#include "BasicEvent.h"

class FreeSpinsRetriggerEvent : public BasicEvent {
public: 
	explicit FreeSpinsRetriggerEvent(int spins, int multiplier = 1) 
		: spins(spins), multiplier(multiplier) { }

protected:
	static constexpr std::string_view EventType = "freeSpinsRetrigger";

	std::string_view getEventType() const override {
		return EventType;
	}

	Json::Value getDetailsAsJson() const override {
		Json::Value eventDetails;
		eventDetails["spins"] = spins;
		eventDetails["multiplier"] = multiplier;
		return eventDetails;
	}


private:
	const int spins;
	const float multiplier;
};
