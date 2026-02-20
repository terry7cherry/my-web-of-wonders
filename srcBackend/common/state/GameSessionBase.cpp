#include "GameSessionBase.h"

#include "../../thisTitle/forces/frontendForces.h"

GameSessionBase::GameSessionBase(const int bet,
                                 const GameMaths& maths,
                                 const Forces::SimForce simForce)
    : bet(bet),
      rtp(maths.rtp),
      betType(maths.betType),
      simForce(simForce),
      betMultiplier(getBetMultiplier(bet, maths))
{
}


GameSessionBase::GameSessionBase(const Json::Value& jsonSession)
    : bet(getBetFromJson(jsonSession)),
      rtp(getRtpFromJson(jsonSession)),
      betType(getBetTypeFromJson(jsonSession)),
      simForce(Forces::ForceNone),
      betMultiplier(getBetMultiplierFromJson(jsonSession))
{
}


int GameSessionBase::getBetMultiplier(const int bet, const GameMaths& maths)
{
    const double multiplier = bet / static_cast<int>(maths.minBet * maths.betTypeCost + 0.5);
    double intPart;
    if (std::modf(multiplier, &intPart) == 0.0) return static_cast<int>(multiplier);
    throw std::runtime_error(
        "Bet is not valid for this betType. Bet must be an integer multiple of "
        + std::to_string(static_cast<int>(maths.minBet * maths.betTypeCost))
        + ". (minBet of " + std::to_string(maths.minBet)
        + " multiplied by betType cost of " + std::to_string(maths.betTypeCost) + ".)"
    );
}

Json::Value GameSessionBase::toJson() const
{
    Json::Value json;
    json["bet"] = bet;
    json["betMultiplier"] = betMultiplier;
    json["rtp"] = rtp;
    json["betType"] = betType;
    json["simForce"] = simForce;
    return json;
}

unsigned int GameSessionBase::getBetFromJson(const Json::Value& jsonSession)
{
    const Json::Value bet = jsonSession["bet"];
    if (bet.isNull()) throw std::runtime_error("bet is missing from session.");
    return bet.asUInt();
}

unsigned int GameSessionBase::getBetMultiplierFromJson(const Json::Value& jsonSession)
{
    const Json::Value betMultiplier = jsonSession["betMultiplier"];
    if (betMultiplier.isNull()) throw std::runtime_error("betMultiplier is missing from session.");
    return betMultiplier.asUInt();
}

int GameSessionBase::getRtpFromJson(const Json::Value& jsonSession)
{
    const Json::Value rtp = jsonSession["rtp"];
    if (rtp.isNull()) throw std::runtime_error("rtp is missing from session.");
    return rtp.asInt();
}

std::string GameSessionBase::getBetTypeFromJson(const Json::Value& jsonSession)
{
    const Json::Value betType = jsonSession["betType"];
    if (betType.isNull()) throw std::runtime_error("betType is missing from session.");
    return betType.asString();
}

Forces::SimForce GameSessionBase::getSimForceFromJson(const Json::Value& jsonSession)
{
    const Json::Value simForce = jsonSession["simForce"];
    if (simForce.isNull()) return Forces::ForceNone;
    if (simForce.asInt() < 0 || simForce.asInt() >= Forces::NumForces)
        throw std::runtime_error(
            "Invalid simForce value.");
    return static_cast<Forces::SimForce>(simForce.asInt());
}
