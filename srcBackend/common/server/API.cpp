#include "API.h"

#include <cstdlib>
#include <cstring>
#include <random>

#include "../gameMaths/parser/ParSheet.h"
#include "../../thisTitle/state/GameSession.h"
#include "drogon/drogon.h"
#include "../../thisTitle/state/GameCycle.h"
#include "../../thisTitle/GameLogic.h"
#include "../../thisTitle/forces/frontendForces.h"
#include "../rng/MathsRng.h"

#ifdef FRONTEND
void API::startAPIServer()
{
	ParSheet parSheet(GameMaths::getFrontendMathsSheetName());
	const GameMaths& maths = *parSheet.gameMathsVariants["default"][96];
	GameSession session(maths.minBet, maths);
	using namespace drogon;
	app().registerHandler("/api/options",
	                      [&parSheet](const HttpRequestPtr& request [[maybe_unused]],
	                                  std::function<void(const HttpResponsePtr&)>&&
	                                  callback)
	                      {
		                      try
		                      {
			                      handleOptionsRequest(std::move(callback), parSheet);
		                      }
		                      catch (const std::exception& e)
		                      {
			                      callback(getBadRequestError(e.what()));
		                      }
	                      }
	);
	app().registerHandler("/api/maths",
	                      [&parSheet](const HttpRequestPtr& request,
	                                  std::function<void(const HttpResponsePtr&)>&&
	                                  callback)
	                      {
		                      try
		                      {
			                      handleMathsRequest(request, std::move(callback), parSheet);
		                      }
		                      catch (const std::exception& e)
		                      {
			                      callback(getBadRequestError(e.what()));
		                      }
	                      },
	                      {Post}
	);
	app().registerHandler("/api/newSession",

	                      [&parSheet](const HttpRequestPtr& request,
	                                  std::function<void(const HttpResponsePtr&)>&&
	                                  callback)
	                      {
		                      try
		                      {
			                      handleNewSessionRequest(
				                      request, std::move(callback), parSheet);
		                      }
		                      catch (const std::exception& e)
		                      {
			                      callback(getBadRequestError(e.what()));
		                      }
	                      },
	                      {Post}
	);
	app().registerHandler("/api/spin",
	                      [&parSheet](const HttpRequestPtr& request,
	                                  std::function<void(
		                                  const HttpResponsePtr&)>&&
	                                  callback)
	                      {
		                      try
		                      {
			                      handleSpinRequest(request, std::move(callback), parSheet);
		                      }
		                      catch (const std::exception& e)
		                      {
			                      callback(getBadRequestError(e.what()));
		                      }
	                      },
	                      {Post}

	);
	app().registerHandler("/healthz",
	                      [](const HttpRequestPtr& request [[maybe_unused]],
	                         std::function<void(const HttpResponsePtr&)>&& callback)
	                      {
		                      Json::Value result;
		                      result["status"] = "ok";
		                      auto resp = HttpResponse::newHttpJsonResponse(result);
		                      callback(resp);
	                      }
	);
	app().addListener("0.0.0.0", 8080);

#ifndef FRONTEND_DEV
	// Serve static files from "../srcFrontend/dist" in production
	app()
		.setDocumentRoot(FRONTEND_DIST_DIR)
		.setFileTypes({
			"gif", "png", "jpg", "js", "css", "html", "ico", "swf", "xap", "apk",
			"cur", "xml",
			"mp3"

		});
	std::cout << "Starting dev server" << std::endl;
	std::cout << "Go to localhost:8080 in your browser" << std::endl;
#endif

	const char* basePathEnv = std::getenv("BASE_PATH");
	if (basePathEnv && std::strlen(basePathEnv) > 0)
	{
		std::string basePath(basePathEnv);
		if (basePath.front() != '/') basePath = "/" + basePath;
		if (basePath.back() == '/') basePath.pop_back();

		app().registerPreRoutingAdvice(
			[basePath](const HttpRequestPtr& req,
			           AdviceCallback&& acb,
			           AdviceChainCallback&& accb)
			{
				const std::string& path = req->path();
				if (path == basePath)
				{
					auto resp = HttpResponse::newHttpResponse();
					resp->setStatusCode(k301MovedPermanently);
					resp->addHeader("Location", basePath + "/");
					acb(resp);
					return;
				}
				if (path.starts_with(basePath + "/"))
				{
					std::string newPath = path.substr(basePath.length());
					if (newPath.empty()) newPath = "/";
					req->setPath(newPath);
				}
				accb();
			});
		std::cout << "Base path prefix stripping enabled for: " << basePath << std::endl;
	}

	const char* authUser = std::getenv("BASIC_AUTH_USERNAME");
	const char* authPass = std::getenv("BASIC_AUTH_PASSWORD");
	const bool authEnabled = authUser && authPass
		&& std::strlen(authUser) > 0 && std::strlen(authPass) > 0;

	if (authEnabled)
	{
		std::string expectedCredentials = std::string(authUser) + ":" + std::string(authPass);
		std::string expectedEncoded = drogon::utils::base64Encode(expectedCredentials);

		app().registerPreRoutingAdvice(
			[expectedEncoded](const HttpRequestPtr& req,
			                  AdviceCallback&& acb,
			                  AdviceChainCallback&& accb)
			{
				if (req->path() == "/healthz")
				{
					accb();
					return;
				}
				const std::string& authHeader = req->getHeader("authorization");
				if (authHeader.substr(0, 6) == "Basic " &&
					authHeader.substr(6) == expectedEncoded)
				{
					accb();
					return;
				}
				auto resp = HttpResponse::newHttpResponse();
				resp->setStatusCode(k401Unauthorized);
				resp->addHeader("WWW-Authenticate", "Basic realm=\"Game Simulator\"");
				acb(resp);
			});
		std::cout << "Basic auth enabled" << std::endl;
	}

	app()
		.setThreadNum(std::max(2u, std::thread::hardware_concurrency() - 2))
		.run();
}


void API::handleSpinRequest(const drogon::HttpRequestPtr& request,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            const ParSheet& parSheet)
{
	const auto requestJson = getJsonFromRequest(request);
	if (requestJson["session"].isNull()) throw std::runtime_error("session is missing from request.");
	GameSession session(requestJson["session"]);
	const GameMaths& maths = *parSheet.gameMathsVariants.at(session.betType).at(session.rtp);

	GameCycleReused reused(maths);

	Json::Value forceIndexJson = requestJson["forceIndex"];
	Json::Value seedJson = requestJson["seed"];
	Json::Value rnStream = requestJson["rnStream"];

	const auto [cycle, seed] =
		!rnStream.isNull()
			? playRnStreamGame(maths, reused, session, rnStream.asString())
			: !seedJson.isNull()
			? playFixedSeedGame(maths, reused, session, seedJson.asUInt())
			: forceIndexJson.isNull()
			? playNaturalGame(maths, reused, session)
			: playForcedGame(maths, reused, session, forceIndexJson.asInt());
	Json::Value result;
	Json::Value eventsArray(Json::arrayValue);
	for (Json::ArrayIndex i = 0; i < cycle.events.size(); ++i)
	{
		if (!cycle.events[i])
		{
			std::cerr << "[/api/spin] null event pointer at index "
				<< i << std::endl;
			continue;
		}
		Json::Value eventJson = cycle.events[i]->toJson();
		if (!eventJson.isNull())
			eventsArray.append(cycle.events[i]->toJson());
	}
	result["events"] = eventsArray;
	result["session"] = session.toJson();
	result["seed"] = seed;
	result["winnings"] = cycle.totalWin;
	if (!forceIndexJson.isNull())
	{
		int forceIndex = forceIndexJson.asInt();
		result["forceName"] = Forces::frontendForces[forceIndex].name;
	}

	auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
	callback(resp);
}

void API::handleNewSessionRequest(const drogon::HttpRequestPtr& request,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                  const ParSheet& parSheet)
{
	const auto requestJson = getJsonFromRequest(request);
	if (requestJson["bet"].isNull()) throw std::runtime_error("bet is missing from request.");
	if (requestJson["betType"].isNull())
		throw std::runtime_error("betType is missing from request.");
	if (requestJson["rtp"].isNull()) throw std::runtime_error("rtp is missing from request.");

	const GameMaths& maths = *parSheet.gameMathsVariants
	                                  .at(requestJson["betType"].asString())
	                                  .at(requestJson["rtp"].asInt());

	const int bet = requestJson["bet"].asInt();

	const GameSession newSession = requestJson["oldSession"].isNull()
		                               ? GameSession(bet, maths)
		                               : GameSession(bet, maths, requestJson["oldSession"]);

	const Json::Value jsonResponse = newSession.toJson();
	const auto resp = drogon::HttpResponse::newHttpJsonResponse(jsonResponse);
	callback(resp);
}

void API::handleMathsRequest(const drogon::HttpRequestPtr& request,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                             const ParSheet& parSheet)
{
	const auto requestJson = getJsonFromRequest(request);
	if (requestJson["betType"].isNull())
		throw std::runtime_error("betType is missing from request.");
	if (requestJson["rtp"].isNull()) throw std::runtime_error("rtp is missing from request.");

	const GameMaths& maths = *parSheet.gameMathsVariants
	                                  .at(requestJson["betType"].asString())
	                                  .at(requestJson["rtp"].asInt());

	Json::Value jsonResponse;
	jsonResponse["maths"] = maths.toJson();
	jsonResponse["forceOptions"] = getForceOptionsForBetType(maths.betType);
	const auto resp = drogon::HttpResponse::newHttpJsonResponse(jsonResponse);
	callback(resp);
}

Json::Value API::getForceOptionsForBetType(const std::string& betType)
{
	Json::Value forceOptions(Json::arrayValue);
	int forceNum = 0;
	for (const FrontendForce& force : Forces::frontendForces)
	{
		const bool validForBetType =
			force.betTypes.empty()
			|| std::ranges::find(force.betTypes, betType) != force.betTypes.end();
		if (validForBetType)
		{
			Json::Value forceJson;
			forceJson["name"] = force.name;
			forceJson["index"] = forceNum;
			forceOptions.append(forceJson);
		}
		forceNum++;
	}
	return forceOptions;
}


Json::Value API::getJsonFromRequest(const drogon::HttpRequestPtr& request)
{
	const auto jsonPtr = request->getJsonObject();
	if (!jsonPtr)
		throw std::runtime_error("Request is missing JSON.");
	return *jsonPtr;
}

drogon::HttpResponsePtr API::getBadRequestError(const std::string& error)
{
	const auto resp = drogon::HttpResponse::newHttpResponse();
	resp->setStatusCode(drogon::k400BadRequest);
	resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
	Json::Value err;
	err["error"] = error;
	resp->setBody(err.toStyledString());
	return resp;
}

void API::handleOptionsRequest(std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                               const ParSheet& parSheet)
{
	Json::Value optionsJson;
	optionsJson["betTypes"] = Json::Value{};
	for (size_t i = 0; i < parSheet.betTypes.size(); ++i)
		optionsJson["betTypes"][parSheet.betTypes[i]] = std::round(parSheet.betTypeCosts[i] * 100.0) /
			100.0;
	optionsJson["rtps"] = Json::Value(Json::arrayValue);
	for (const int rtp : parSheet.rtps) optionsJson["rtps"].append(rtp);
	optionsJson["gameTitle"] = parSheet.gameTitle;
	optionsJson["minBet"] = parSheet.minBet;

	// Serialize with a writer configured for decimal precision for this endpoint only.
	Json::StreamWriterBuilder builder;
	builder["indentation"] = ""; // compact JSON
	builder["precisionType"] = "decimal"; // digits after decimal point
	builder["precision"] = 2; // e.g. 1.60 (JSON may still display as 1.6 depending on writer behavior)

	const auto resp = drogon::HttpResponse::newHttpResponse();
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
	resp->setBody(Json::writeString(builder, optionsJson));
	callback(resp);
}

std::pair<GameCycle, uint32_t> API::playFixedSeedGame(const GameMaths& maths,
                                                      GameCycleReused& reused,
                                                      GameSession& session,
                                                      const uint32_t seed)
{
	mathsRng.setRng(RngOptions::kiss);
	mathsRng.setSeed(seed);
	GameCycle cycle(session, reused, maths);
	GameLogic::playGame(maths, cycle);
	return {cycle, seed};
}


std::pair<GameCycle, uint32_t> API::playNaturalGame(const GameMaths& maths,
                                                    GameCycleReused& reused,
                                                    GameSession& session)
{
	static std::mt19937 seedGenerator{std::random_device{}()};
	uint32_t seed = seedGenerator();
	mathsRng.setRng(RngOptions::kiss);
	mathsRng.setSeed(seed);
	GameCycle cycle(session, reused, maths);
	GameLogic::playGame(maths, cycle);
	return {cycle, seed};
}

std::pair<GameCycle, uint32_t> API::playRnStreamGame(const GameMaths& maths, GameCycleReused& reused,
                                                     GameSession& session,
                                                     const std::string& rnStreamStr)
{
	mathsRng.setRng(RngOptions::data);
	GameCycle cycle(session, reused, maths);
	auto& rngData = dynamic_cast<RngData&>(*mathsRng.lpRng);
	rngData.setGameCycle(&cycle);
	RngDataGame rngDataGame(rnStreamStr);
	rngData.startGameSequence(&rngDataGame);
	GameLogic::playGame(maths, cycle);
	return {cycle, 0};
}

std::pair<GameCycle, uint32_t> API::playForcedGame(const GameMaths& maths,
                                                   GameCycleReused& reused,
                                                   GameSession& session,
                                                   const int forceIndex)
{
	static std::mt19937 seedGenerator{std::random_device{}()};
	if (forceIndex >= Forces::frontendForces.size())
		throw std::runtime_error(
			"Force " + std::to_string(forceIndex) + " does not exist.");

	const auto& [name, condition, betTypes]
		= Forces::frontendForces[forceIndex];

	if (!betTypes.empty() && std::ranges::find(betTypes, session.betType) == betTypes.end())
		throw std::runtime_error("Selected force is not available for this bet type.");

	uint64_t attempts = 0;
	do
	{
		const uint32_t seed = seedGenerator();
		mathsRng.setRng(RngOptions::kiss);
		mathsRng.setSeed(seed);
		GameCycle cycle(session, reused, maths);
		GameLogic::playGame(maths, cycle);
		if (condition(cycle)) return {cycle, seed};
		attempts++;
	}
	while (attempts < 10'000'000);
	throw std::runtime_error("Force " + std::to_string(forceIndex) + " failed after 10m runs.");
}


#endif
