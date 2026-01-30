#pragma once

#include <vector>
#include "../../common/server/FrontendForce.h"

namespace Forces
{
    const std::vector frontendForces = {
        FrontendForce(
            "Free Spins",
            [](const GameCycle& cycle)
            {
                return cycle.fsTrigger;
            }
        ),
        FrontendForce(
            "50 Free Spins",
            [](const GameCycle& cycle)
            {
                return cycle.fsInitialSpins == 50;
            }
        ),
        FrontendForce(
            "Big Win (x1000)",
            [](const GameCycle& cycle)
            {
                return cycle.totalWin >= 25000;
            },
            {"bonusBuy"}
        )

    };
}
