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
    const size_t bet;
    const size_t betMultiplier;
    const std::string rtp;
    const BetType betType;

    GameSession(const int bet, const BetType betType, std::string rtp,
                const GameMathsNew& gameMaths)
        : bet(bet),
          betMultiplier(bet / gameMaths.ints.at("minBet")),
          rtp(std::move(rtp)),
          betType(betType)
    {
    }
};
