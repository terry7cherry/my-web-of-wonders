#pragma once

#include "../../common/events/eventTemplates/PhaseCompleteTemplate.h"

class BonusSymbolCollectionEvent : public PhaseCompleteTemplate<BonusSymbolCollectionEvent>
{
public:
    size_t reel;
    size_t row;
    inline static bool multipleHitsPerCycle = true;
    inline static bool countsAsSpinForRnStream = false;
    inline static std::string phaseName = "Bonus Symbol Collection";

    BonusSymbolCollectionEvent(const size_t reel, const size_t row, const bool repeatEvent = false) :
        PhaseCompleteTemplate(0, repeatEvent),
        reel(reel), row(row)
    {
    }

    static void updateMeters(const size_t reel [[maybe_unused]], const size_t row [[maybe_unused]],
                             const bool repeatEvent = false)
    {
        PhaseCompleteTemplate::updateMeters(0, repeatEvent);
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
