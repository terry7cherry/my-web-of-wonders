#pragma once
#include "eventTemplates/BasicEvent.h"

class RNStreamProblemEvent : public BasicEvent
{
public:
    std::string shortMessage;
    std::string longMessage;

    explicit RNStreamProblemEvent(const std::string& shortMessage, const std::string& longMessage) :
        shortMessage(shortMessage),
        longMessage(longMessage)
    {
    }

    static void updateMeters(const std::string& shortMessage [[maybe_unused]],
                             const std::string& longMessage [[maybe_unused]])
    {
    }

    Json::Value toJson() const override
    {
        Json::Value eventDetails;
        eventDetails["eventType"] = "rnStreamProblem";
        eventDetails["shortMessage"] = shortMessage;
        eventDetails["longMessage"] = longMessage;
        return eventDetails;
    }
};
