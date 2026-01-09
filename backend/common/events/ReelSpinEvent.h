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
    std::string_view getEventType() const override
    {
        return "reelSpin";
    }

    Json::Value getDetailsAsJson() const override;

private:
    std::array<int, ReelSpinMaxReels * ReelSpinMaxRows> symbols{};
    size_t numReels;
    size_t numRows;
};
