#pragma once

#include "GameMaths.h"

enum BetType
{
    BetTypeDefault,
    BetTypeBonusBuy
};

struct GameSession
{
    const int bet;
    const int betMultiplier;
    const std::string rtp;
    const BetType betType;

    GameSession(const int bet, const BetType betType, const std::string& rtp,
                const GameMaths& gameMaths)
        : bet(bet),
          betMultiplier(bet / gameMaths.minBet),
          rtp(rtp),
          betType(betType)
    {
    }

    void rtpReport(const uint64 plays, uint64 totalBet, bool newLine)
    {
        printf("%s%10I64d spins. RTP ", newLine ? "\n" : "\r", plays);

        //for (auto& event : eventVect)
        //    event.rtpReport(totalBet);
    }
};
