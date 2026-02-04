#pragma once
/**
 * @file API.h
 * @brief Backend API server entry points, compiled only for FRONTEND builds.
 */

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include "../../thisTitle/state/GameCycle.h"
#include "../../thisTitle/state/GameSession.h"
#include "../gameMaths/parser/ParSheet.h"

namespace API
{
#ifdef FRONTEND
    /**
     * @brief Start the HTTP API server for the frontend to communicate with.
     */
    void startAPIServer();

    void handleSpinRequest(const drogon::HttpRequestPtr& request,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const ParSheet& parSheet);

    void handleNewSessionRequest(const drogon::HttpRequestPtr& request,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 const ParSheet& parSheet);

    void handleMathsRequest(const drogon::HttpRequestPtr& request,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            const ParSheet& parSheet);

    void handleOptionsRequest(std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                              const ParSheet& parSheet);

    std::pair<GameCycle, uint32_t> playFixedSeedGame(const GameMaths& maths,
                                                     GameCycleReused& reused,
                                                     GameSession& session,
                                                     uint32_t seed);

    std::pair<GameCycle, uint32_t> playNaturalGame(const GameMaths& maths,
                                                   GameCycleReused& reused,
                                                   GameSession& session);

    std::pair<GameCycle, uint32_t> playForcedGame(const GameMaths& maths,
                                                  GameCycleReused& reused,
                                                  GameSession& session,
                                                  int forceIndex);

    Json::Value getJsonFromRequest(const drogon::HttpRequestPtr& request);

    Json::Value getForceOptionsForBetType(const std::string& betType);

    drogon::HttpResponsePtr getBadRequestError(const std::string& error);
#endif
}
