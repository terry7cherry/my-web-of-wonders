#pragma once
#include "../../common/events/eventTemplates/BasicEvent.h"

class MaxedOutMultipliersEvent : public BasicEvent
{
public:
    MaxedOutMultipliersEvent() : BasicEvent()
    {
    }

    static void updateMeters()
    {
    }

    Json::Value toJson() const override
    {
        Json::Value eventDetails = BasicEvent::toJson();
        eventDetails["eventType"] = "maxedOutMultipliers";
        return eventDetails;
    }
};
