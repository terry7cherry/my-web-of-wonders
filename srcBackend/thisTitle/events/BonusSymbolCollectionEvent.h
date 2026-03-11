#pragma once
#include "../../common/events/eventTemplates/BasicEvent.h"

class BonusSymbolCollectionEvent : public BasicEvent
{
public:
    size_t reel;
    size_t row;

    BonusSymbolCollectionEvent(const size_t reel, const size_t row)
        : reel(reel),
          row(row)
    {
    }

    static void updateMeters(const size_t reel [[maybe_unused]],
                             const size_t row [[maybe_unused]])
    {
    }

    Json::Value toJson() const override
    {
        Json::Value eventDetails = BasicEvent::toJson();
        eventDetails["eventType"] = "bonusSymbolCollection";
        eventDetails["reel"] = reel;
        eventDetails["row"] = row;
        return eventDetails;
    }
};
