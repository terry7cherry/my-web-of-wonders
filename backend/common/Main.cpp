#include <iostream>
#include <cstdio>
#include <algorithm>
#include <string>
#include <thread>
#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <csignal>
#include <drogon/drogon.h>
#include <atomic>
#include <thread>
// MathLibrary includes
#include "headers/cpp-math-library.h"

// cpp-dev-library includes
#include "console/Console.h"

// Game includes
#include "defunct/GameMaths.h"
#include "../thisTitle/GameCycle.h"
#include "../thisTitle/GameLogic.h"
#include "../thisTitle/GameSession.h"
#include "../thisTitle/simConfig.h"
#include "events/BasicEvent.h"
#include "events/GameCycleCompleteEvent.h"
#include "events/FreeSpinsCompleteEvent.h"
#include "gameMaths/parser/excelParser.h"
#include "NodeDevServer.h"
#include "events/threadSafeMeters/MeterUpdate.h"


#ifdef FRONTEND_DEV

NodeDevServer* g_nodeServer = nullptr;

void shutdownHook()
{
	if (g_shuttingDown.exchange(true)) return;
	if (g_nodeServer) g_nodeServer->stop();
}

void signalHandler(int)
{
	shutdownHook();
	std::_Exit(1);
}

#endif

// global variables
Console gameConsole;
std::string gameTitle;

TaggedSheet* parSheet;

bool showGame = false;

enum class simFunction
{
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
int maxPayLines = 20; // data for cycle eval

int bet = 0;

int rtpOption = -1; // the user selected rtp [90,91,92...] or 0 if all rtps wanted in the sim
int forceOption = 0;

int reportAtCount = MILLION / 10;
int reportCountLimit = 100 * MILLION;
int reportCount = reportAtCount;

void setParSheet(const std::string& parSheetName)
{
	try
	{
		parSheet = new TaggedSheet(parSheetName);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error loading PAR sheet: " << e.what() << std::endl;
	}
	gameTitle = parSheet->gameTitle;
}

void setHeader(std::string modeText = "", const bool show = true, const bool clearScreen = true)
{
	std::string consoleHeader = gameTitle;
	if (rtpOption != -1)
	{
		consoleHeader += " RTP";
		consoleHeader += (rtpOption ? std::to_string(rtpOption) : "Cycle");
	}

	if (RN_STREAM_TEST)
		consoleHeader += " - RN_STREAM_TEST";
	else
	{
		if (PLAY_MODE)
			consoleHeader += " - PLAYABLE";
		else if (OUTPUT_REELS)
			consoleHeader += " - DATA OUTPUT";
		else if (simMode == simFunction::reelTest)
			consoleHeader += " - REEL TEST";
		else
			consoleHeader += " - MONTE_CARLO";

		if (FIXED_SEED)
			consoleHeader += "(FIXED SEED)";

		if (forceOption == 1)
			consoleHeader += " - FORCE FS";
	}
	if (modeText.size())
		consoleHeader += (" - " + modeText);

	gameConsole.setHeader(consoleHeader, show, clearScreen);
}

void outputData()
{
	//FIX
	/*
	if (OUTPUT_REELS)
	{
		// output all reel in their own files
		for (auto& reelSet : gameMaths.reelSetVect)
			reelSet.fileOutput(gameMaths.symbolTextMap);
	}
	*/
}


// parameter rtp is the value: 96, 91, 90 etc
void apply_cycle_evaluation(int rtp)
{
}

void doMonteCarloSimBB(int bbMode, uint64_t simGames)
{
}

void showRtpReport(const uint64_t gamesPlayed, const uint64_t totalBet)
{
	std::printf("%s%10I64d spins. RTP ", "\n", gamesPlayed);
	const long double percent = static_cast<long double>(GameCycleCompleteEvent::eventTotalWin.global()) * 100
		/ static_cast<double>(totalBet);
	std::printf(", %s=%6.2Lf%%", "total", percent);
}

void threadWorker(const std::stop_token& st,
                  const GameMathsNew& gameMaths,
                  std::atomic<uint64_t>& nextBatch,
                  const uint64_t totalBatches,
                  const int batchSize,
                  const int gameBet,
                  const uint32 globalSeed)
{
	GameCycleReused reused(gameMaths);

	while (!st.stop_requested())
	{
		const uint64_t batch = nextBatch.fetch_add(1, std::memory_order_relaxed);

		if (batch >= totalBatches)
		{
			break;
		}

		mathRng.setSeed(globalSeed + batch);

		GameSession gameSession(gameBet, BetTypeDefault, "96", gameMaths);

		for (int i = 0; i < batchSize; i++)
		{
			GameCycle gameCycle(reused);
			GameLogic::playGame(gameMaths, gameSession, gameCycle);
		}
		MeterUpdate::triggerUpdate();
	}
}


void doMonteCarloStdMultiThread(const GameMathsNew& gameMaths, const int gameBet, const uint64_t simGames,
                                const uint32 globalSeed)
{
	constexpr int BATCH_SIZE = 1'000'000;
	const uint64_t totalBatches = simGames / BATCH_SIZE;
	const unsigned int threadCount = std::thread::hardware_concurrency() - 3;
	std::cout << "Using " << threadCount << " threads" << std::endl;

	gameConsole.clearScreen();
	setHeader("Std");
	//gameConsole.setCursorPos(2);

	std::atomic<uint64_t> nextBatch{0};
	std::vector<std::jthread> threads;

	threads.reserve(threadCount);
	Timer simTimer("Monte Carlo");

	for (int i = 0; i < threadCount; i++)
	{
		threads.emplace_back(
			threadWorker,
			std::cref(gameMaths),
			std::ref(nextBatch),
			totalBatches,
			BATCH_SIZE,
			gameBet,
			globalSeed
		);
	}
	uint64_t last_seen = 0;
	while (last_seen < simGames)
	{
		const uint64_t current = GameCycleCompleteEvent::eventTotalHits.global();
		if (current != last_seen)
		{
			showRtpReport(current, bet * current);
			last_seen = current;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	showRtpReport(simGames, bet * simGames);
	simTimer.outputTime();
	std::printf("%20s,%12s,%12s,%12s,%12s,%12s,%12s\n",
	            "Event", "RTP", "HitRate", "AvgPay", "AvgWin", "WinChance", "WinRate");
	GameCycleCompleteEvent::finalReport(simGames, bet);
	FreeSpinsCompleteEvent::finalReport(simGames, bet);
	GameCycleCompleteEvent::outputDistribution(false, true);
}

void doMonteCarloStdMultiThread(const GameMathsNew& gameMaths, const int gameBet, const uint64_t simGames)
{
	const uint32 seed = std::rand();
	doMonteCarloStdMultiThread(gameMaths, gameBet, simGames, seed);
}

void doMonteCarloStd(const GameMathsNew& gameMaths, const int gameBet, const uint64_t simGames)
{
	gameConsole.clearScreen();
	setHeader("Std");
	//gameConsole.setCursorPos(2);

	GameCycleReused reused(gameMaths);


	GameSession gameSession(gameBet, BetTypeDefault, "96", gameMaths);

	reportCount = reportAtCount = MILLION / 10;
	Timer simTimer("Monte Carlo");

	// Start the simulation loop
	uint64_t gameCount = 0;
	uint64_t sessionPlays = 0;

	for (; gameCount < simGames; gameCount++)
	{
		GameCycle gameCycle(reused);
		GameLogic::playGame(gameMaths, gameSession, gameCycle);

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
			showRtpReport(gamesPlayed, bet * gamesPlayed);
			if ((reportAtCount != reportCountLimit) && (gamesPlayed == (10 * reportAtCount)))
				reportAtCount *= 10;
			reportCount = reportAtCount;
		}
	}

	//*****************************
	// Montecarlo over
	//*****************************
	//FIX
	//gameMaths.finalReport(gameCount, bet * gameCount);
	//gameMaths.distributionsSave();

	printf("\n");
	simTimer.outputTime();
	GameCycleCompleteEvent::outputDistribution(false, true);
}

void doMonteCarloStd(const GameMathsNew& gameMaths, const int gameBet, const uint64_t simGames,
                     const int globalSeed)
{
	mathRng.setSeed(globalSeed);
	doMonteCarloStd(gameMaths, gameBet, simGames);
}


//FIX
//void doRnStreamTest(int gameBet) {
//	gameConsole.clearScreen();
//	setHeader();
//	gameConsole.setCursorPos(2);
//
//	// Start the stream loop
//	for (int64_t gameCount = 0; !rnStream.rnStreamDone; gameCount++) {
//		showGame = false;
//		int breakGame = RN_TEST_GAME; // RN stream broken game
//		if (rnStream.startGameSequence(breakGame) == breakGame)
//			showGame = true; // show what happens this game
//		gameMaths.playGame();
//	}
//}

void doReelTest(int gameBet, uint64_t simGames, int reelsetId)
{
	/*
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
			gameMaths.rtpReport(gamesPlayed, bet * gamesPlayed);
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
	*/
}

//FIX
//void doPlayMode(void) {
//	gameMaths.setConsole(&gameConsole);
//	setHeader();
//	gameConsole.clearBuffer();
//	int totBet = 0, totalWin = 0, balance = 0;
//	for (int game = 0; true; game++, totBet += bet) {
//		gameConsole.setCursorPos(2);
//		printf("Game:%5d\t Bet: %4dc\t Balance $%8.2f\tRTP: %5.2f\n", game, bet, (float) balance / 100,
//		       totBet ? ((float) totalWin * 100 / totBet) : 0);
//
//		gameConsole.waitAnyKeyText(game == 0 ? "TO PLAY" : "TO PLAY AGAIN", LINE_WINS_COLUMN,
//		                           gameMaths.getPressToPlayLine());
//		balance -= bet;
//
//		gameConsole.showHeader(gameMaths.clearScreen());
//		gameConsole.setCursorPos(2);
//		printf("Game:%5d\t Bet: %4dc\t Balance $%8.2f\tRTP: %5.2f\n", game + 1, bet, (float) balance / 100,
//		       totBet ? ((float) totalWin * 100 / totBet) : 0);
//
//
//		int win = gameMaths.playGame(forceOption);
//		totalWin += win;
//		balance += win;
//	}
//}

int getUserChoiceFromList(const std::vector<std::string>& choices, const std::string& prompt)
{
	setHeader();
	std::cout << std::endl;
	gameConsole.coutStyle("\t" + prompt, CONSOLE_INVERT);
	std::cout << std::endl;
	int choiceNum = 0;
	for (const auto& choice : choices)
		std::cout << std::format("\t\t{}) {}\n", choiceNum++, choice);

	int newChoice = 0;
	do
	{
		std::cout << "\tEnter a valid Option >";
		std::cin >> newChoice;
	}
	while (newChoice < 0 || newChoice >= (int)choices.size());
	return newChoice;
}

// return index + 1 from gameMaths options
// return 0 to sim all available options
int getUsersRtpChoice()
{
	constexpr bool giveAllRtpOption = !RN_STREAM_TEST;

	std::vector<std::string> rtpOptionStrings;
	if (giveAllRtpOption) rtpOptionStrings.emplace_back("All");
	for (const int rtp : parSheet->rtps)
		rtpOptionStrings.emplace_back(std::to_string(rtp));

	const int userChoice = getUserChoiceFromList(rtpOptionStrings, "SELECT RTP");

	return userChoice;
}

// return index + 1 from gameMaths options
// return 0 to sim all availble options
std::string getUserBetTypeChoice(void)
{
	const int userChoice = getUserChoiceFromList(parSheet->betTypes, "SELECT RTP");
	return parSheet->betTypes.at(userChoice);
}

// Prompt user to select play zone
int getUserForceSelection(void)
{
	std::vector<std::string> forceStrings;
	forceStrings.emplace_back("None - Random play");
	forceStrings.emplace_back("Free Spins");
	const int userChoice = getUserChoiceFromList(forceStrings, "SET GAME FORCE");
	return userChoice;
}

int getUsersPlayModeChoice(void)
{
	// get user configuration
	int newMode = 0;
	//gameConsole.setCursorPos(2);
	gameConsole.coutStyle("\tSET PLAY MODE\t", CONSOLE_INVERT);
	std::cout << std::endl;
	printf("\t\t1) All games\n");
	printf("\t\t2) Special Games\n");
	do
	{
		std::cout << "\tEnter a valid Option >";
		std::cin >> newMode;
	}
	while (newMode < 1 || newMode > 2);

	return newMode;
}

#ifdef FRONTEND
void startBackEndDevServer()
{
	setParSheet("test.xlsx");
	const GameMathsNew& gameMaths = *parSheet->gameMathsVariants["default"][96];
	GameSession gameSession(gameMaths.ints.at("minBet"), BetTypeDefault, "96", gameMaths);
	gameConsole.initialise(PLAY_MODE);
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
			                      eventsArray[i] = gameCycle.events[i]->toJson();
		                      }
		                      result["events"] = eventsArray;

		                      auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
		                      callback(resp);
	                      }
	);
	app().addListener("0.0.0.0", 8080);

#ifndef FRONTEND_DEV
// Serve static files from "../frontend/dist" in production
app()
		.setDocumentRoot (FRONTEND_DIST_DIR)
		.setFileTypes (



{
	"gif", "png", "jpg", "js", "css", "html", "ico", "swf", "xap", "apk", "cur", "xml",
		"mp3"
});
#endif

std::cout<< "Starting dev server" << std::endl;
app().run();
}
#endif


#ifdef FRONTEND_DEV
void startNodeDevServer()
{
	const std::string cmd = "npm --prefix \"" + std::string(FRONTEND_DIR) + "\" run dev";
	std::system(cmd.c_str());
}
#endif

int main()
{
#ifdef FRONTEND
#ifdef FRONTEND_DEV
	NodeDevServer node;
	g_nodeServer = &node;

	std::atexit(shutdownHook);
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::set_terminate([]
	{
		shutdownHook();
		std::_Exit(1);
	});

	node.start();
#endif
	startBackEndDevServer();
	return 0;
#else
	uint64_t simSize = 0;
	gameConsole.initialise(PLAY_MODE);

	if (OUTPUT_REELS)
	{
		setHeader();
		outputData();
		return 0;
	}

	setParSheet("test.xlsx");
	if (RN_STREAM_TEST)
	{
		mathRng.setRng(RngOptions::data);
		bet = rnStream.openDataFile(RN_STREAM_FILE, bet);
	}
	else if (FIXED_SEED)
		mathRng.setSeed(0);
	else
		mathRng.setSeed();

	if (RN_STREAM_TEST)
	{
		/*
		rtpOption = gameMaths.rtpSetOption(0);
		setHeader();
		doRnStreamTest(bet);
		*/
	}
	else if (PLAY_MODE)
	{
		/*
		forceOption = getUserForceSelection();
		setHeader();
		gameMaths.playableMode = gameMaths.PlayableOptions::allGames;
		doPlayMode();
		*/
	}
	else
	{
		simMode = simFunction::stdMontecarlo;
		setHeader("", true, false);

		printf("\nEnter number of Simulation runs in Millions: ");
		std::cin >> simSize;

		if (simMode == simFunction::reelTest)
		{
			//FIX
			/*
			// reel test
			int reelset = -1;
			const auto reelNameVect = gameMaths.getReelList();
			gameConsole.setCursorPos(2);
			gameConsole.coutStyle("\tSELECT TEST REELSET\t", CONSOLE_INVERT);
			std::cout << std::endl;
			for (size_t i = 0; i < reelNameVect.size(); i++)
				printf("\t\t%2zd %s\n", i, reelNameVect[i].c_str());
			do
			{
				std::cout << "\tEnter a valid Option >";
				std::cin >> reelset;
			}
			while (reelset < 0 || reelset >= reelNameVect.size());

			doReelTest(bet, simSize * MILLION, reelset);
			*/
		}
		else
		{
			const int rtpChoice = getUsersRtpChoice();
			if (rtpChoice == 0)
				rtpOption = -1;
			else
				rtpOption = parSheet->rtps.at(rtpChoice - 1);
			const std::string betType = getUserBetTypeChoice();
			forceOption = getUserForceSelection();
			setHeader();

			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::string seedString;
			printf("\nEnter fixed seed. (Leave blank for random.)");
			std::getline(std::cin, seedString);
			const bool seedSet = !seedString.empty();
			int globalSeed = 0;
			if (seedSet)
				globalSeed = std::stoi(seedString);
			if (simMode == simFunction::stdMontecarlo)
			{
				// Std-Game Monte-Carlo
				if (rtpOption == 0)
				{
					// all RTP options
					for (int rtp : parSheet->rtps)
					{
						GameMathsNew& gameMaths = *parSheet->gameMathsVariants[betType][rtp];
						bet = gameMaths.ints.at("minBet");
						if (seedSet)
							doMonteCarloStdMultiThread(gameMaths, bet, simSize * MILLION, globalSeed);
						else
							doMonteCarloStdMultiThread(gameMaths, bet, simSize * MILLION);
					}
				}
				else
				{
					// valid single RTP option
					GameMathsNew& gameMaths = *parSheet->gameMathsVariants[betType][rtpOption];
					bet = gameMaths.ints.at("minBet");
#ifdef SINGLETHREAD
					if (seedSet)
						doMonteCarloStd(gameMaths, bet, simSize * MILLION, globalSeed);
					else
						doMonteCarloStd(gameMaths, bet, simSize * MILLION);
#else
					if (seedSet)
						doMonteCarloStdMultiThread(gameMaths, bet, simSize * MILLION, globalSeed);
					else
						doMonteCarloStdMultiThread(gameMaths, bet, simSize * MILLION);
#endif
				}
			}
		}
	}

	return 0;
#endif
}
