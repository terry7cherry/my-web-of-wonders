#pragma once

#include <vector>
#include "../../common/server/FrontendForce.h"

namespace Forces
{
    const std::vector frontendForces = {
        FrontendForce(
            "Eggwakening",
            [](const GameCycle& cycle)
            {
                return cycle.fsTrigger;
            }
        ),
        FrontendForce(
            "Super Eggwakening",
            [](const GameCycle& cycle)
            {
                return cycle.fsCollectorTrigger;
            }
        ),
        FrontendForce(
            "Coin Win",
            [](const GameCycle& cycle)
            {
                return cycle.baseCoinsRecorded;
            }
        ),
        FrontendForce(
            "Bonus Collection",
            [](const GameCycle& cycle)
            {
                return cycle.bonusSymbolCollected;
            }
        ),
        FrontendForce(
            "Multipliers Maxed",
            [](const GameCycle& cycle)
            {
                return cycle.multipliersMaximized;
            }),
        FrontendForce(
            "Big Win (x1000)",
            [](const GameCycle& cycle)
            {
                return cycle.totalWin >= 25000;
            }
        ),
        FrontendForce(
            "Win",
            [](const GameCycle& cycle)
            {
                return cycle.totalWin > 0;
            }
        ),
    };
}
