#pragma once

#include "BasicEvent.h"

#include "../grids/Grid.h"

static constexpr int ReelSpinMaxReels = 10;
static constexpr int ReelSpinMaxRows = 10;

class ReelSpinEvent : public BasicEvent
{
public:
    explicit ReelSpinEvent(const Grid& grid);

    static void updateMeters(const Grid& grid [[maybe_unused]])
    {
    }

protected:
    static constexpr std::string_view EventType = "reelSpin";

    std::string_view getEventType() const override
    {
        return EventType;
    }

    Json::Value getDetailsAsJson() const override;

private:
    std::array<int, ReelSpinMaxReels * ReelSpinMaxRows> symbols{};
    size_t numReels;
    size_t numRows;
};
