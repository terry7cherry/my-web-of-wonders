#pragma once

#include "../CollectorPositions.h"
#include "../../common/events/eventTemplates/BasicEvent.h"
#include "../CollectorPositions.h"

class CoinCollectionEvent : public BasicEvent
{
public:
    size_t reel;
    size_t row;
    CollectorPosition collectorPosition;

    CoinCollectionEvent(const size_t reel, const size_t row, const CollectorPosition position)
        : reel(reel),
          row(row),
          collectorPosition(position)
    {
    }

    static void updateMeters(const size_t reel [[maybe_unused]],
                             const size_t row [[maybe_unused]],
                             const CollectorPosition position [[maybe_unused]])
    {
    }

    Json::Value toJson() const override
    {
        Json::Value eventDetails = BasicEvent::toJson();
        eventDetails["eventType"] = "coinCollection";
        eventDetails["reel"] = reel;
        eventDetails["row"] = row;
        eventDetails["collectorPosition"] = collectorPositionNames[collectorPosition];
        return eventDetails;
    }
};
