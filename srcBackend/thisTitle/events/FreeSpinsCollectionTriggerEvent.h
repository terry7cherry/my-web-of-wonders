#pragma once

#include "../../common/events/FreeSpinsTriggerEvent.h"

class FreeSpinsCollectionTriggerEvent : public FreeSpinsTriggerEvent
{
public:
    unsigned int initialSecondaryCollectors;

    FreeSpinsCollectionTriggerEvent(const int spins, const unsigned int initialSecondaryCollectors) :
        FreeSpinsTriggerEvent(spins),
        initialSecondaryCollectors(initialSecondaryCollectors)
    {
    }

    static void updateMeters(const int spins [[maybe_unused]],
                             const unsigned int initialSecondaryCollectors [[maybe_unused]])
    {
        FreeSpinsTriggerEvent::updateMeters(spins);
    }

    Json::Value toJson() const override
    {
        Json::Value eventDetails = FreeSpinsTriggerEvent::toJson();
        eventDetails["eventType"] = "freeSpinsCollectionTrigger";
        eventDetails["initialSecondaryCollectors"] = initialSecondaryCollectors;
        return eventDetails;
    }
};
