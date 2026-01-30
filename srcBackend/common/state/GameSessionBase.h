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
    const int rtp; // RTP variant (e.g., 92, 94, 96)
    const std::string betType; // e.g., "default" or product-defined type
    const Forces::SimForce simForce;
    const int frontendForceIndex = -1;
    const unsigned int betMultiplier; // computed from bet, minBet, and betTypeCost

    /**
     * @brief Construct a session using a bet and maths configuration.
     * @param bet Wager amount in credits.
     * @param maths Selected maths variant.
     * @param simForce The simulator force to be applied.
     * @throws std::runtime_error if bet is not a valid multiple of minBet*betTypeCost.
     */
    GameSessionBase(int bet,
                    const GameMaths& maths,
                    Forces::SimForce simForce = Forces::ForceNone);

    /**
     * @brief Construct a session from a JSON sent by the client.
     * @param jsonSession Json representation of session.
     * @throws std::runtime_error if a required element is not present.
     */
    GameSessionBase(const Json::Value& jsonSession);

    /**
     * @brief Compute bet multiplier validating divisibility by minBet*betTypeCost.
     * @throws std::runtime_error when bet is not a valid multiple.
     */
    static int getBetMultiplier(int bet, const GameMaths& maths);

    /**
     * @brief Convert session to JSON to be sent to client.
     * @return Json representation of session.
     */
    Json::Value toJson() const;

    static unsigned int getBetFromJson(const Json::Value& jsonSession);
    static int getRtpFromJson(const Json::Value& jsonSession);
    static std::string getBetTypeFromJson(const Json::Value& jsonSession);
    static unsigned int getBetMultiplierFromJson(const Json::Value& jsonSession);
    static Forces::SimForce getSimForceFromJson(const Json::Value& jsonSession);
    static int getFrontendForceIndexFromJson(const Json::Value& jsonSession);
};
