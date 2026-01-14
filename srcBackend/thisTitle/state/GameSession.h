#pragma once

#include <utility>

#include "../../common/gameMaths/GameMaths.h"

struct GameSession
{
    const unsigned int bet;
    const unsigned int betMultiplier;
    const int rtp;
    const std::string betType;

    GameSession(const int bet,
                const GameMaths& gameMaths)
        : bet(bet),
          betMultiplier(getBetMultiplier(bet, gameMaths)),
          rtp(gameMaths.rtp),
          betType(gameMaths.betType)
    {
    }

    static int getBetMultiplier(const int bet, const GameMaths& gameMaths)
    {
        const double multiplier = bet / gameMaths.ints.at("minBet") / gameMaths.betTypeCost;
        double intPart;
        if (std::modf(multiplier, &intPart) == 0.0) return static_cast<int>(multiplier);
        throw std::runtime_error(
            "Bet is not valid for this betType. Bet must be a multiple of "
            + std::to_string(gameMaths.ints.at("minBet") * gameMaths.betTypeCost)
            + "(minBet of " + std::to_string(gameMaths.ints.at("minBet"))
            + "multiplied by betType cost of " + std::to_string(gameMaths.betTypeCost) + ".)");
    }
};
