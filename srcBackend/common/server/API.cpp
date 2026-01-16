#include "API.h"
#include "../gameMaths/parser/ParSheet.h"
#include "../../thisTitle/state/GameSession.h"
#include "drogon/drogon.h"
#include "../../thisTitle/state/GameCycle.h"
#include "../../thisTitle/GameLogic.h"

#ifdef FRONTEND
void API::startAPIServer()
{
	ParSheet parSheet("test.xlsx");
	const GameMaths& gameMaths = *parSheet.gameMathsVariants["default"][96];
	GameSession gameSession(static_cast<int>(gameMaths.ints.at("minBet") * gameMaths.betTypeCost), gameMaths);
	using namespace drogon;
	// Register REST endpoint
	app().registerHandler("/api/spin",
	                      [&gameSession, &gameMaths](const HttpRequestPtr&,
	                                                 std::function<void(const HttpResponsePtr&)>&&
	                                                 callback)
	                      {
		                      GameCycleReused reused(gameMaths);
		                      GameCycle gameCycle(reused);


		                      GameLogic::playGame(gameMaths, gameSession, gameCycle);;

		                      Json::Value result;
		                      Json::Value eventsArray(Json::arrayValue);
		                      for (Json::ArrayIndex i = 0; i < gameCycle.events.size(); ++i)
		                      {
			                      if (!gameCycle.events[i])
			                      {
				                      std::cerr << "[/api/spin] null event pointer at index "
					                      << i << std::endl;
				                      continue;
			                      }
			                      Json::Value eventJson = gameCycle.events[i]->toJson();
			                      if (!eventJson.isNull())
				                      eventsArray.append(gameCycle.events[i]->toJson());
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
		.setFileTypes(


			{
				"gif", "png", "jpg", "js", "css", "html", "ico", "swf", "xap", "apk", "cur", "xml",
				"mp3"
			});
#endif

	std::cout << "Starting dev server" << std::endl;
	std::cout << "Go to localhost:8080 in your browser" << std::endl;
	app().run();
}
#endif
