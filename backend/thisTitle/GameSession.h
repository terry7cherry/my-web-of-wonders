#pragma once

#include <utility>

#include "../common/gameMaths/GameMaths.h"

enum BetType
{
    BetTypeDefault,
    BetTypeBonusBuy
};

struct GameSession
{
    const unsigned int bet;
    const unsigned int betMultiplier;
    const std::string rtp;
    const BetType betType;

    GameSession(const int bet, const BetType betType, std::string rtp,
                const GameMaths& gameMaths)
        : bet(bet),
          betMultiplier(bet / gameMaths.ints.at("minBet")),
          rtp(std::move(rtp)),
          betType(betType)
    {
    }
};
