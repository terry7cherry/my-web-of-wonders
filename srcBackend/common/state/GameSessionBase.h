#pragma once
/**
 * @file GameSessionBase.h
 * @brief Immutable snapshot of session parameters derived from `GameMaths`.
 */

#include "../gameMaths/GameMaths.h"
#include "../../thisTitle/forces/simForces.h"

/**
 * @brief Holds the bet, computed bet multiplier, RTP, and bet type for a session.
 */
struct GameSessionBase
{
    const unsigned int bet; // wager amount in credits
    const unsigned int betMultiplier; // computed from bet, minBet, and betTypeCost
    const int rtp; // RTP variant (e.g., 92, 94, 96)
    const std::string betType; // e.g., "default" or product-defined type
    const Forces::SimForce simForce;

    /**
     * @brief Construct a session using a wager and maths configuration.
     * @param bet Wager amount in credits.
     * @param gameMaths Selected maths variant.
     * @param simForce The simulator force to be applied.
     * @throws std::runtime_error if bet is not a valid multiple of minBet*betTypeCost.
     */
    GameSessionBase(const int bet,
                    const GameMaths& gameMaths,
                    const Forces::SimForce simForce = Forces::ForceNone)
        : bet(bet),
          betMultiplier(getBetMultiplier(bet, gameMaths)),
          rtp(gameMaths.rtp),
          betType(gameMaths.betType),
          simForce(simForce)
    {
    }

    /**
     * @brief Compute bet multiplier validating divisibility by minBet*betTypeCost.
     * @throws std::runtime_error when bet is not a valid multiple.
     */
    static int getBetMultiplier(const int bet, const GameMaths& gameMaths)
    {
        const double multiplier = bet / gameMaths.ints.at("minBet") / gameMaths.betTypeCost;
        double intPart;
        if (std::modf(multiplier, &intPart) == 0.0) return static_cast<int>(multiplier);
        throw std::runtime_error(
            "Bet is not valid for this betType. Bet must be an integer multiple of "
            + std::to_string(static_cast<int>(gameMaths.ints.at("minBet") * gameMaths.betTypeCost))
            + ". (minBet of " + std::to_string(gameMaths.ints.at("minBet"))
            + " multiplied by betType cost of " + std::to_string(gameMaths.betTypeCost) + ".)"
        );
    }
};
