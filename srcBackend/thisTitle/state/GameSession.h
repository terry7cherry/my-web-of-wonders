/**
 * \file GameSession.h
 * \brief Defines the title-specific session state persisted between spins.
 */
#pragma once

#include "../../common/gameMaths/GameMaths.h"
#include "../../common/state/GameSessionBase.h"
#include "../forces/simForces.h"

/**
 * \brief Title-specific game session state.
 *
 * Extends \c GameSessionBase with any persistent variables that should survive
 * across spins for this title. Values here are typically initialized from
 * \c GameMaths, may be updated by game logic, and are serialized via \c toJson().
 */
struct GameSession : GameSessionBase
{
    // int jackpotValue;
    // int consecutiveLosingSpins;

    /**
     * \brief Construct a new session for a given bet and maths configuration.
     *
     * Initializes all persistent values from the provided \c GameMaths and
     * optional simulation force.
     *
     * \param bet The current wager/bet value (bet-locked base state).
     * \param maths Reference to the game maths configuration for initialization.
     * \param simForce Optional simulation force used for deterministic testing.
     */
    GameSession(const int bet,
                const GameMaths& maths,
                const Forces::SimForce simForce = Forces::ForceNone
    )
        : GameSessionBase(bet, maths, simForce)
    // Initialise persistent values (typically from gameMaths). e.g.
    // jackpotValue(maths.ints["initialJackpotValue"]),
    // consecutiveLosingSpins(0)
    {
    }

    /**
     * \brief Construct a new session for a different bet using an existing session.
     *
     * Creates a new session (e.g., after a bet change) while copying only the
     * persistent state that should NOT be bet-locked from \p oldSession.
     *
     * \param bet The new wager/bet value.
     * \param maths Game maths for the new session.
     * \param oldSession The previous session to copy non bet-locked values from.
     */
    GameSession(const int bet,
                const GameMaths& maths,
                const GameSession oldSession)
        : GameSession(bet, maths, Forces::ForceNone)
    {
        // Copy any persistent data that SHOULD NOT BE bet locked from the oldSession. E.g.
        // consecutiveLosingSpins = oldSession.consecutiveLosingSpins;
    }

    /**
     * \brief Reconstruct a session from its JSON representation.
     *
     * \param jsonSession JSON object produced by \c toJson().
     */
    GameSession(const Json::Value& jsonSession)
        : GameSessionBase(jsonSession)
    // Initialise all persistent values from the json here. E.g.
    // jackpotValue(jsonSession["jackpotValue"].asInt()),
    // consecutiveLosingSpins(jsonSession["consecutiveLosingSpins"].asInt())
    {
    }

    /**
     * \brief Serialize the session to JSON.
     *
     * Includes base-session fields and all title-specific persistent values.
     *
     * \return JSON object representing the full session state.
     */
    Json::Value toJson() const
    {
        Json::Value jsonSession = GameSessionBase::toJson();
        // Add any persistent data to the JSON session here. E.g.
        // jsonSession["jackpotValue"] = jackpotValue;
        // jsonSession["consecutiveLosingSpins"] = consecutiveLosingSpins;
        return jsonSession;
    }
};
