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

#include <unordered_map>
#include <mutex>
#include <filesystem>
#include <random>
#include <fstream>

namespace {
// Session id -> uploaded ParSheet mapping
std::unordered_map<std::string, std::shared_ptr<ParSheet>> g_sessionParSheets;
std::mutex g_sessionParSheetsMutex;

std::string generateSid()
{
    static std::mt19937_64 rng{std::random_device{}()};
    static const char* hex = "0123456789abcdef";
    uint64_t a = rng();
    uint64_t b = rng();
    std::string out;
    out.reserve(32);
    for (int i = 0; i < 16; ++i)
    {
        uint8_t byte = (i < 8 ? (a >> (i * 8)) : (b >> ((i - 8) * 8)));
        out.push_back(hex[(byte >> 4) & 0xF]);
        out.push_back(hex[byte & 0xF]);
    }
    return out;
}

// Extract sid from request (JSON body or query/form parameter)
std::string getSidFromRequest(const drogon::HttpRequestPtr& request)
{
    // Prefer JSON body if present
    const auto jsonPtr = request->getJsonObject();
    if (jsonPtr && (*jsonPtr)["sid"].isString())
        return (*jsonPtr)["sid"].asString();

    // Fallback to form/query parameter
    const auto sidParam = request->getParameter("sid");
    if (!sidParam.empty()) return sidParam;
    return {};
}

// Resolve which ParSheet to use for this request
const ParSheet& resolveParSheetForRequest(const drogon::HttpRequestPtr& request,
                                          const ParSheet& defaultParSheet)
{
    const std::string sid = getSidFromRequest(request);
    if (sid.empty()) return defaultParSheet;
    std::scoped_lock lock(g_sessionParSheetsMutex);
    const auto it = g_sessionParSheets.find(sid);
    if (it != g_sessionParSheets.end() && it->second)
        return *(it->second);
    return defaultParSheet;
}

void setSessionParSheet(const std::string& sid, std::shared_ptr<ParSheet> ps)
{
    std::scoped_lock lock(g_sessionParSheetsMutex);
    g_sessionParSheets[sid] = std::move(ps);
}

// Handle base64 JSON upload of a par sheet for a session
void handleUploadParSheet(const drogon::HttpRequestPtr& request,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                          const ParSheet& defaultParSheet)
{
    using namespace std;
    using namespace std::filesystem;

    const auto jsonPtr = request->getJsonObject();
    if (!jsonPtr) throw std::runtime_error("Request is missing JSON.");
    const Json::Value& body = *jsonPtr;

    const string sid = body.isMember("sid") ? body["sid"].asString() : string{};
    if (sid.empty()) throw std::runtime_error("sid is missing from request.");

    const string fileName = body.isMember("fileName") ? body["fileName"].asString() : string{"uploaded.xlsx"};
    const string dataBase64 = body.isMember("dataBase64") ? body["dataBase64"].asString() : string{};
    if (dataBase64.empty()) throw std::runtime_error("dataBase64 is missing from request.");

    // Decode base64
    const auto data = drogon::utils::base64Decode(dataBase64);

    const path destDir = path(GAME_SOURCE_DIR) / "srcBackend" / "thisTitle" / "parSheets";
    std::error_code ec;
    create_directories(destDir, ec);

    // Build unique filename: sid + original extension if any
    string ext;
    const auto dotPos = fileName.find_last_of('.');
    if (dotPos != string::npos) ext = fileName.substr(dotPos);
    if (ext.empty()) ext = ".xlsx";
    const string uniqueName = sid + ext;
    const path destPath = destDir / uniqueName;

    // Write file
    std::ofstream ofs(destPath, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    ofs.close();

    // Parse into a new ParSheet (by filename only; ParSheet loads from parSheets dir)
    auto ps = std::make_shared<ParSheet>(uniqueName);
    setSessionParSheet(sid, ps);

    // Return updated options (for this session's par sheet)
    const ParSheet& effective = *ps;
    Json::Value optionsJson;
    optionsJson["betTypes"] = Json::Value{};
    for (size_t i = 0; i < effective.betTypes.size(); ++i)
        optionsJson["betTypes"][effective.betTypes[i]] = std::round(effective.betTypeCosts[i] * 100.0) / 100.0;
    optionsJson["rtps"] = Json::Value(Json::arrayValue);
    for (const int rtp : effective.rtps) optionsJson["rtps"].append(rtp);
    optionsJson["gameTitle"] = effective.gameTitle;
    optionsJson["minBet"] = effective.minBet;
    optionsJson["sid"] = sid;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(optionsJson);
    callback(resp);
}
} // namespace

#ifdef FRONTEND
void API::startAPIServer()
{
	ParSheet parSheet(GameMaths::getFrontendMathsSheetName());
	const GameMaths& maths = *parSheet.gameMathsVariants["default"][96];
	GameSession session(maths.minBet, maths);
	using namespace drogon;
	// Allow larger uploads for base64-encoded spreadsheets (default may be too small)
	// 20 MB should be sufficient for typical Excel files encoded in base64
	app().setClientMaxBodySize(20 * 1024 * 1024);
 app().registerHandler("/api/options",
                       [&parSheet](const HttpRequestPtr& request,
                                   std::function<void(const HttpResponsePtr&)>&&
                                   callback)
                       {
                           try
                           {
                               handleOptionsRequest(request, std::move(callback), parSheet);
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

 // Upload a new maths spreadsheet (session-scoped)
 app().registerHandler("/api/uploadParSheet",
                       [&parSheet](const HttpRequestPtr& request,
                                   std::function<void(const HttpResponsePtr&)>&& callback)
                       {
                           try
                           {
                               handleUploadParSheet(request, std::move(callback), parSheet);
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
#ifdef FRONTEND_DEV
	app().addListener("0.0.0.0", 5172);
#else
	app().addListener("0.0.0.0", 8080);
#endif

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
    const ParSheet& effectiveParSheet = resolveParSheetForRequest(request, parSheet);
    const GameMaths& maths = *effectiveParSheet.gameMathsVariants.at(session.betType).at(session.rtp);

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

    const ParSheet& effectiveParSheet = resolveParSheetForRequest(request, parSheet);
    const GameMaths& maths = *effectiveParSheet.gameMathsVariants
                                          .at(requestJson["betType"].asString())
                                          .at(requestJson["rtp"].asInt());

	const int bet = requestJson["bet"].asInt();

	const GameSession newSession = requestJson["oldSession"].isNull()
		                               ? GameSession(bet, maths)
		                               : GameSession(bet, maths, requestJson["oldSession"]);

    Json::Value jsonResponse = newSession.toJson();
    // Ensure a sid is present/returned so the client can upload a custom sheet for this session
    std::string sid = requestJson.isMember("sid") && requestJson["sid"].isString()
                          ? requestJson["sid"].asString()
                          : getSidFromRequest(request);
    if (sid.empty()) sid = generateSid();
    jsonResponse["sid"] = sid;
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

    const ParSheet& effectiveParSheet = resolveParSheetForRequest(request, parSheet);
    const GameMaths& maths = *effectiveParSheet.gameMathsVariants
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

void API::handleOptionsRequest(const drogon::HttpRequestPtr& request,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                               const ParSheet& parSheet)
{
    const ParSheet& effective = resolveParSheetForRequest(request, parSheet);
    Json::Value optionsJson;
    optionsJson["betTypes"] = Json::Value{};
    for (size_t i = 0; i < effective.betTypes.size(); ++i)
        optionsJson["betTypes"][effective.betTypes[i]] = std::round(effective.betTypeCosts[i] * 100.0) /
            100.0;
    optionsJson["rtps"] = Json::Value(Json::arrayValue);
    for (const int rtp : effective.rtps) optionsJson["rtps"].append(rtp);
    optionsJson["gameTitle"] = effective.gameTitle;
    optionsJson["minBet"] = effective.minBet;

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
