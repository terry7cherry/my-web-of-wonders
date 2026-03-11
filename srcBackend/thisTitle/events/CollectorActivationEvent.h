#pragma once

#include "../../common/events/eventTemplates/BasicEvent.h"
#include "../CollectorPositions.h"

class CollectorActivationEvent : public BasicEvent
{
public:
    CollectorPosition position;
    unsigned int collectorMultipliers[4];

    CollectorActivationEvent(const CollectorPosition position, const unsigned int collectorMultipliers[4])
        : position(position),
          collectorMultipliers{
              collectorMultipliers[0], collectorMultipliers[1], collectorMultipliers[2],
              collectorMultipliers[3]
          }
    {
    }

    static void updateMeters(const CollectorPosition position [[maybe_unused]],
                             const unsigned int collectorMultipliers[4])
    {
    }

    Json::Value toJson() const override
    {
        Json::Value eventDetails = BasicEvent::toJson();
        eventDetails["eventType"] = "collectorActivation";
        eventDetails["position"] = collectorPositionNames[position];
        eventDetails["collectorMultipliers"] = Json::Value();
        for (size_t i = 0; i < 4; i++)
        {
            eventDetails["collectorMultipliers"][collectorPositionNames[i]] = collectorMultipliers[i];
        }
        return eventDetails;
    }
};
