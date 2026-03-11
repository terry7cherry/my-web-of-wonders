#pragma once

#include "../../common/events/eventTemplates/BasicEvent.h"

class CollectorAwardedBonusEvent : public BasicEvent
{
public:
    Json::Value toJson() const override
    {
        Json::Value eventDetails = BasicEvent::toJson();
        eventDetails["eventType"] = "collectorAwardedBonus";
        return eventDetails;
    }
};
