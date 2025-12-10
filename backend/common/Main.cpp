#include <iostream>
#include <cstdio>
#include <algorithm>
#include <string>
#include <thread>
#include <cstdlib>
#include <cstdint>
#include <drogon/drogon.h>
// MathLibrary includes
#include "headers/cpp-math-library.h"
using namespace MathLibrary;

// cpp-dev-library includes
#include "console/Console.h"

// Game includes
#include "../thisTitle/GameMaths.h"
#include "../thisTitle/simConfig.h"
#include "events/BasicEvent.h"

;

// global variables
Console gameConsole;
std::string gameTitle;

GameMaths gameMaths;
bool showGame = false;

enum class simFunction {
	unknown, stdMontecarlo, bbMontecarlo, reelTest
};

simFunction simMode = simFunction::unknown;

/*********************************************************/
// Simulation Meters
/*********************************************************/
int64_t n_hits, n_base_hit, n_fg_hits;


/*********************************************************/
// Simulation Configuration
/*********************************************************/
int maxPayLines = 20;			// data for cycle eval

int bet = 0;

int rtpOption = -1;				// the user selected rtp [90,91,92...] or 0 if all rtps wanted in the sim
int forceOption = 0;

int reportAtCount = MILLION / 10;
int reportCountLimit = 100 * MILLION;
int reportCount = reportAtCount;

void setHeader(std::string modeText = "", bool show = true)
{
	gameTitle = gameMaths.getGameName();

	if (rtpOption != -1)
	{
		gameTitle += " RTP";
		gameTitle += (rtpOption ? gameMaths.getSzRtp() : "Cycle");
	}

	if (RN_STREAM_TEST)
		gameTitle += " - RN_STREAM_TEST";
	else
	{
		if (PLAY_MODE)
			gameTitle += " - PLAYABLE";
		else if (OUTPUT_REELS)
			gameTitle += " - DATA OUTPUT";
		else if (simMode == simFunction::reelTest)
			gameTitle += " - REEL TEST";
		else
			gameTitle += " - MONTE_CARLO";

		if (FIXED_SEED)
			gameTitle += "(FIXED SEED)";

		if (forceOption == 1)
			gameTitle += " - FORCE FS";

	}
	if (modeText.size())
		gameTitle += (" - " + modeText);

	gameConsole.setHeader(gameTitle, show);
}

void outputData()
{
	if (OUTPUT_REELS)
	{			// output all reel in their own files
		for (auto& reelSet : gameMaths.reelsetVect)
			reelSet.fileOutput(gameMaths.symbolTextMap);
	}
}


// parameter rtp is the value: 96, 91, 90 etc
void apply_cycle_evaluation(int rtp, int maxPayLines)
{
}

void doMonteCarloSimBB(int bbMode, uint64_t simGames)
{
}

void doMonteCarloStd(int gameBet, uint64_t simGames)
{
	gameConsole.clearScreen();
	setHeader("Std");
	gameConsole.setCursorPos(2);

	gameMaths.reset(gameBet);
	reportCount = reportAtCount = MILLION / 10;
	Timer simTimer("Monte Carlo");

	// Start the simulation loop
	uint64_t gameCount = 0;
	uint64_t sessionPlays = 0;

	for (; gameCount < simGames; gameCount++)
	{
		gameMaths.playGame(-1);

#if SESSION_LIMIT
		if (++sessionPlays == SESSION_LIMIT)
		{
			sessionPlays = 0;
		}
#endif
		// Print progress every reportCount plays
		if (--reportCount == 0)
		{
			int64_t gamesPlayed = gameCount + 1;
			gameMaths.rtpReport(gamesPlayed, bet * gamesPlayed, SHOW_RTP_REPORT_HISTORY);
			if ((reportAtCount != reportCountLimit) && (gamesPlayed == (10 * reportAtCount)))
				reportAtCount *= 10;
			reportCount = reportAtCount;
		}
	}

	//*****************************
	// Montecarlo over
	//*****************************
	gameMaths.finalReport(gameCount, bet * gameCount);
	gameMaths.distributionsSave();

	printf("\n");
	simTimer.outputTime();
}

void doRnStreamTest(int gameBet)
{
	gameConsole.clearScreen();
	setHeader();
	gameConsole.setCursorPos(2);

	// Start the stream loop
	for (int64_t gameCount = 0; !rnStream.rnStreamDone; gameCount++)
	{
		showGame = false;
		int breakGame = RN_TEST_GAME;				// RN stream broken game
		if (rnStream.startGameSequence(breakGame) == breakGame)
			showGame = true;		    // show what happens this game
		gameMaths.playGame();
	}
}

void doReelTest(int gameBet, uint64_t simGames, int reelsetId)
{
	gameConsole.clearScreen();
	setHeader(gameMaths.reelsetVect[reelsetId].mSzName);
	gameConsole.setCursorPos(2);

	gameMaths.reset(gameBet);
	reportCount = reportAtCount = MILLION / 10;
	Timer simTimer("Monte Carlo");

	// Start the simulation loop
	uint64_t gameCount = 0;
	uint64_t sessionPlays = 0;

	for (; gameCount < simGames; gameCount++)
	{
		gameMaths.reelTest(reelsetId);

		// Print progress every reportCount plays
		if (--reportCount == 0)
		{
			int64_t gamesPlayed = gameCount + 1;
			gameMaths.rtpReport(gamesPlayed, bet * gamesPlayed, SHOW_RTP_REPORT_HISTORY);
			if ((reportAtCount != reportCountLimit) && (gamesPlayed == (10 * reportAtCount)))
				reportAtCount *= 10;
			reportCount = reportAtCount;
		}
	}

	//*****************************
	// Reel Test over
	//*****************************
	gameMaths.finalReport(gameCount, bet * gameCount);
	printf("\n");
	simTimer.outputTime();
}

void doPlayMode(void)
{
	gameMaths.setConsole(&gameConsole);
	setHeader();
	gameConsole.clearBuffer();
	int totBet = 0, totalWin = 0, balance = 0;
	for (int game = 0; true; game++, totBet += bet)
	{
		gameConsole.setCursorPos(2);
		printf("Game:%5d\t Bet: %4dc\t Balance $%8.2f\tRTP: %5.2f\n", game, bet, (float)balance / 100,
			totBet ? ((float)totalWin * 100 / totBet) : 0);

		gameConsole.waitAnyKeyText(game == 0 ? "TO PLAY" : "TO PLAY AGAIN", LINE_WINS_COLUMN, gameMaths.getPressToPlayLine());
		balance -= bet;

		gameConsole.showHeader(gameMaths.clearScreen());
		gameConsole.setCursorPos(2);
		printf("Game:%5d\t Bet: %4dc\t Balance $%8.2f\tRTP: %5.2f\n", game + 1, bet, (float)balance / 100,
			totBet ? ((float)totalWin * 100 / totBet) : 0);


		int win = gameMaths.playGame(forceOption);
		totalWin += win;
		balance += win;
	}
}

// return index + 1 from gameMaths options
// return 0 to sim all availble options 
int getUsersRtpChoice(void)
{
	bool giveAllRtpOption = !PLAY_MODE && !RN_STREAM_TEST;
	int lowOption = giveAllRtpOption ? 0 : 1;
	int highOption = (int)gameMaths.rtpOptionVect.size();

	gameConsole.setCursorPos(2);
	gameConsole.coutStyle("\tSELLECT RTP", CONSOLE_INVERT);
	std::cout << std::endl;

	if (giveAllRtpOption)
		printf("\t\t0) All\n");
	int pick = 1;
	for (int i = 0; i < (int)gameMaths.rtpOptionVect.size(); i++, pick++)
		printf("\t\t%d) %s\n", pick, gameMaths.rtpOptionVect[i].c_str());

	do {
		std::cout << "Enter a valid Option >";
		std::cin >> pick;
	} while (pick < lowOption || pick > highOption);
	return pick;
}

// return index + 1 from gameMaths options
// return 0 to sim all availble options 
simFunction getUserSimModeChoice(void)
{				// get user configuration
	int newMode = 0;

	gameConsole.setCursorPos(2);
	gameConsole.coutStyle("\tSET SIM MODE", CONSOLE_INVERT);
	std::cout << std::endl;
	//printf("\t0) Cycle Evaluation\n");
	printf("\t\t1) Std-Game Monte-Carlo\n");
	printf("\t\t2) Reel Test\n");
#if 1
	do {
		std::cout << "\tEnter a valid Option >";
		std::cin >> newMode;
	} while (newMode < 1 || newMode >2);
#else
	printf("\t\t3) BB-R3 Monte-Carlo\n");
	printf("\t\t4) BB-R4 Monte-Carlo\n");
	printf("\t\t5) BB-R5 Monte-Carlo\n");
	if (!RN_STREAM_TEST)
		printf("\t\t6) All BB Options Monte-Carlo\n");
	do {
		std::cout << "\tEnter a valid Option >";
		std::cin >> newMode;
	} while (newMode < 1 || newMode >(RN_STREAM_TEST ? 5 : 6));
#endif
	return (newMode == 1) ? simFunction::stdMontecarlo : simFunction::reelTest;
}

// Prompt user to select play zone
int getUserForceSelection(void)
{
	int force = 0;
	if (DO_FREE_SPINS == false)
		return force;


	gameConsole.setCursorPos(2);
	gameConsole.coutStyle("\tSET GAME FORCE\t", CONSOLE_INVERT);
	std::cout << std::endl;
	std::cout << "\t\t0   None - Random play\n";
	std::cout << "\t\t1   Free Spins\n";
	do {
		std::cout << "\tEnter a valid Option >";
		std::cin >> force;
	} while (force < 0 || force >1);

	if (force == 1 && DO_FREE_SPINS == false)
		force = 0;
	return force;
}

int getUsersPlayModeChoice(void)
{				// get user configuration
	int newMode = 0;
	gameConsole.setCursorPos(2);
	gameConsole.coutStyle("\tSET PLAY MODE\t", CONSOLE_INVERT);
	std::cout << std::endl;
	printf("\t\t1) All games\n");
	printf("\t\t2) Special Games\n");
	do {
		std::cout << "\tEnter a valid Option >";
		std::cin >> newMode;
	} while (newMode < 1 || newMode >2);

	return newMode;
}

#ifdef FRONTEND
void startBackEndDevServer()
{
	gameMaths.initialise();
	gameConsole.initialise(PLAY_MODE);
	using namespace drogon;
	// Register REST endpoint
	app().registerHandler("/api/spin",
	[](const HttpRequestPtr&,
	   std::function<void(const HttpResponsePtr&)>&& callback) {

		using namespace std;

		try {

			gameMaths.reset(gameMaths.minBet);

			gameMaths.playGame(0);

			Json::Value result;
			Json::Value eventsArray(Json::arrayValue);
			for (Json::ArrayIndex i = 0; i < gameMaths.events.size(); ++i) {
				if (!gameMaths.events[i]) {
					cerr << "[/api/spin] null event pointer at index "
						 << i << endl;
					continue;
				}
				eventsArray[i] = gameMaths.events[i]->toJson();
			}
			result["events"] = eventsArray;

			auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
			callback(resp);
		}
		catch (const std::exception& e) {
			cerr << "[/api/spin] exception: " << e.what() << endl;
			auto resp = drogon::HttpResponse::newHttpResponse();
			resp->setStatusCode(drogon::k500InternalServerError);
			resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
			resp->setBody("Internal server error");
			callback(resp);
		}
		catch (...) {
			cerr << "[/api/spin] unknown exception" << endl;
			auto resp = drogon::HttpResponse::newHttpResponse();
			resp->setStatusCode(drogon::k500InternalServerError);
			resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
			resp->setBody("Internal server error");
			callback(resp);
		}
	});
	//app().registerHandler("/api/spin", [](const HttpRequestPtr&,
		//std::function<void(const HttpResponsePtr&)>&& callback) {
			//gameMaths.reset(gameMaths.minBet);
			//gameMaths.playGame(0);
//
			//Json::Value result;
			//Json::Value eventsArray(Json::arrayValue);
			//for (Json::ArrayIndex i = 0; i < gameMaths.events.size(); ++i) {
				//eventsArray[i] = gameMaths.events[i]->toJson();
			//}
			//result["events"] = eventsArray;
			//auto resp = HttpResponse::newHttpJsonResponse(result);
			//callback(resp);
		//});
	app().addListener("0.0.0.0", 8080);

#ifndef FRONTEND_DEV
	// Serve static files from ../frontend/dist in production
	app()
		.setDocumentRoot(FRONTEND_DIST_DIR)
		.setFileTypes({ "gif","png","jpg","js","css","html","ico","swf","xap","apk","cur","xml","mp3" });
#endif

	std::cout << "Starting dev server" << std::endl;
	app().run();
}
#endif


#ifdef FRONTEND_DEV
void startNodeDevServer() {
	std::string cmd = "npm --prefix \"" + std::string(FRONTEND_DIR) + "\" run dev";
	std::system(cmd.c_str());
}
#endif

int main(void)
{
#ifdef FRONTEND
#ifdef FRONTEND_DEV
	std::thread nodeThread(startNodeDevServer);
	nodeThread.detach();
#endif
	startBackEndDevServer();
	return 0;
#else
	uint64_t simSize = 0;
	gameMaths.initialise();
	gameConsole.initialise(PLAY_MODE);

	if (OUTPUT_REELS)
	{
		setHeader();
		outputData();
		return 0;
	}

	bet = gameMaths.minBet;
	gameMaths.reset(bet);
	if (RN_STREAM_TEST)
	{
		mathRng.setRng(RngOptions::data);
		bet = rnStream.openDataFile(RN_STREAM_FILE, bet);
	}
	else if (FIXED_SEED)
		mathRng.setSeed(0);
	else
		mathRng.setSeed();
	setHeader();

	if (RN_STREAM_TEST)
	{
		rtpOption = gameMaths.rtpSetOption(0);
		setHeader();
		doRnStreamTest(bet);
	}
	else if (PLAY_MODE)
	{
		forceOption = getUserForceSelection();
		setHeader();
		gameMaths.playableMode = gameMaths.PlayableOptions::allGames;
		doPlayMode();
	}
	else
	{
		simMode = getUserSimModeChoice();
		setHeader();

		printf("\nEnter number of Simulation runs in Millions: ");
		std::cin >> simSize;

		if (simMode == simFunction::reelTest)
		{				// reel test
			int reelset = -1;
			auto& reelNameVect = gameMaths.getReelList();
			gameConsole.setCursorPos(2);
			gameConsole.coutStyle("\tSELECT TEST REELSET\t", CONSOLE_INVERT);
			std::cout << std::endl;
			for (size_t i = 0; i < reelNameVect.size(); i++)
				printf("\t\t%2zd %s\n", i, reelNameVect[i].c_str());
			do {
				std::cout << "\tEnter a valid Option >";
				std::cin >> reelset;
			} while (reelset < 0 || reelset >= reelNameVect.size());

			doReelTest(bet, simSize * MILLION, reelset);
		}
		else
		{				// Monte-Carlo options
			int rtpChoice = getUsersRtpChoice();
			rtpOption = gameMaths.rtpSetOption(rtpChoice - 1);
			setHeader();
			forceOption = getUserForceSelection();
			setHeader();
			if (simMode == simFunction::stdMontecarlo)
			{				// Std-Game Monte-Carlo
				if (rtpOption == 0)
				{			// all RTP options
					for (int rtpIndex = 0; rtpIndex < gameMaths.rtpOptionVect.size(); rtpIndex++)
					{
						if (gameMaths.rtpSetOption(rtpIndex))
							doMonteCarloStd(bet, simSize * MILLION);
					}
				}
				else
				{			// valid single RTP option
					doMonteCarloStd(bet, simSize * MILLION);
				}
			}
			else if (simMode == simFunction::bbMontecarlo)
			{				// all BB Monte-Carlo
				if (rtpOption == 0)
				{			// all RTP options
					for (int bbMode = 0; bbMode < 4; bbMode++)
					{
						for (int rtpIndex = 0; rtpIndex < gameMaths.rtpOptionVect.size(); rtpIndex++)
						{
							if (gameMaths.rtpSetOption(rtpIndex))
								doMonteCarloSimBB(bbMode, simSize * MILLION);
						}
					}
				}
				else
				{			// valid single RTP option
					for (int bbMode = 0; bbMode < 4; bbMode++)
						doMonteCarloSimBB(bbMode, simSize * MILLION);
				}
			}
			else
			{					// single BB Monte-Carlo
				int bbMode = (int)simMode - (int)simFunction::bbMontecarlo;
				if (rtpOption == 0)
				{				// all RTP options
					for (int rtpIndex = 0; rtpIndex < gameMaths.rtpOptionVect.size(); rtpIndex++)
					{
						if (rtpOption = gameMaths.rtpSetOption(rtpIndex))
							doMonteCarloSimBB(bbMode, simSize * MILLION);
					}
				}
				else
				{				// valid single RTP option
					doMonteCarloSimBB(bbMode, simSize * MILLION);
				}
			}
		}
	}

	return 0;
#endif
}
