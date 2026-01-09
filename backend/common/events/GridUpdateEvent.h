#pragma once

#include "BasicEvent.h"
#include "../grids/Grid.h"

static constexpr int GridUpdateMaxReels = 10;
static constexpr int GridUpdateMaxRows = 10;

class GridUpdateEvent : public BasicEvent
{
public:
    explicit GridUpdateEvent(const Grid& grid, std::string message = "Grid updated!");

    static void updateMeters(const Grid& grid[[maybe_unused]],
                             std::string message [[maybe_unused]] = "Grid updated!")
    {
    }

protected :
    std::string_view getEventType() const override
    {
        return "gridUpdate";
    }

    Json::Value getDetailsAsJson() const override;

private :
    std::array<int, GridUpdateMaxReels * GridUpdateMaxRows> symbols{};
    size_t numReels;
    size_t numRows;
    std::string message;
};
