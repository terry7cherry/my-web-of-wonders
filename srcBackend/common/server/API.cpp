#include "API.h"
#include "../gameMaths/parser/ParSheet.h"
#include "../../thisTitle/state/GameSession.h"
#include "drogon/drogon.h"
#include "../../thisTitle/state/GameCycle.h"
#include "../../thisTitle/GameLogic.h"
#include "../../thisTitle/forces/frontendForces.h"

#ifdef FRONTEND
void API::startAPIServer()
{
	ParSheet parSheet("test.xlsx");
	const GameMaths& maths = *parSheet.gameMathsVariants["default"][96];
	GameSession session(maths.ints["minBet"], maths);
	using namespace drogon;
	// Register REST endpoint
	app().registerHandler("/api/spin",
	                      [&session, &maths](const HttpRequestPtr& request,
	                                         std::function<void(const HttpResponsePtr&)>&&
	                                         callback)
	                      {
		                      GameCycleReused reused(maths);
		                      const std::string force = request->getParameter("force");

		                      const GameCycle cycle = force.empty()
			                                              ? playNaturalGame(maths, reused, session)
			                                              : playForcedGame(maths, reused, session, force);
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

		                      auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
		                      callback(resp);
	                      }

	);
	app().addListener("0.0.0.0", 8080);

#ifndef FRONTEND_DEV
	// Serve static files from "../srcFrontend/dist" in production
	app()
		.setDocumentRoot(FRONTEND_DIST_DIR)
		.setFileTypes({
			"gif", "png", "jpg", "js", "css", "html", "ico", "swf", "xap", "apk", "cur", "xml",
			"mp3"

		});
	std::cout << "Starting dev server" << std::endl;
	std::cout << "Go to localhost:8080 in your browser" << std::endl;
#endif

	app().run();
}

GameCycle API::playNaturalGame(const GameMaths& maths,
                               GameCycleReused& reused,
                               GameSession& session)
{
	GameCycle cycle(session, reused, maths);
	GameLogic::playGame(maths, cycle);
	return cycle;
}

GameCycle API::playForcedGame(const GameMaths& maths,
                              GameCycleReused& reused,
                              GameSession& session,
                              const std::string& forceString)
{
	for (const char& c : forceString)
		if (!isdigit(c)) throw std::runtime_error("Invalid force. Must be an integer.");
	const int forceInt = std::stoi(forceString);
	if (forceInt >= Forces::frontendForces.size())
		throw std::runtime_error(
			"Force " + std::to_string(forceInt) + " does not exist.");

	const auto& [name, condition] = Forces::frontendForces[forceInt];
	uint64_t attempts = 0;
	do
	{
		GameCycle cycle(session, reused, maths);
		GameLogic::playGame(maths, cycle);
		if (condition(cycle)) return cycle;
		attempts++;
	}
	while (attempts < 100'000'000);
	throw std::runtime_error("Force " + std::to_string(forceInt) + " failed after 1b runs.");
}


#endif
