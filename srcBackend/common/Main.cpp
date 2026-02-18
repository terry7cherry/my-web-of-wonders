// Precompiled header
#include "headers/pch.h"

#include <csignal>
#include <filesystem>
#include <cinttypes>
#include <drogon/drogon.h>
#include "rng/MathsRng.h"
#include "utils/Timer.h"

// cpp-dev-library includes
#include "console/Console.h"

// Game includes
#include <random>

#include "../thisTitle/state/GameCycle.h"
#include "../thisTitle/GameLogic.h"
#include "../thisTitle/state/GameSession.h"
#include "../thisTitle/stats/StatsOutput.h"
#include "events/GameCycleCompleteEvent.h"
#include "events/FreeSpinsCompleteEvent.h"
#include "gameMaths/parser/ParSheet.h"
#include "server/NodeDevServer.h"
#include "server/API.h"
#include "events/threadSafeMeters/MeterUpdate.h"
#include "config/UserConfig.h"

Console gameConsole;

int reportAtCount = MILLION / 10;
int reportCountLimit = 100 * MILLION;
int reportCount = reportAtCount;


void showRtpReport(const uint64_t gamesPlayed, const uint64_t totalBet, const uint64_t simSize)
{
	std::printf("\033[1A\r%s%10" PRIu64 " spins. %.2f%% complete. " "RTP", "\n", gamesPlayed,
	            static_cast<double>(100 * static_cast<long double>(gamesPlayed) / static_cast<long double>(
		            simSize)));
	const long double percent = static_cast<long double>(GameCycleCompleteEvent::eventTotalWin.global()) * 100
		/ static_cast<double>(totalBet);
	std::printf(": %s=%6.2Lf%%", "total", percent);
}

void threadWorker(const std::stop_token& st,
                  const GameMaths& gameMaths,
                  std::atomic<uint64_t>& nextBatch,
                  const uint64_t totalBatches,
                  const int batchSize,
                  const int bet,
                  const UserConfig& config)
{
	GameCycleReused reused(gameMaths);

	while (!st.stop_requested())
	{
		const uint64_t batch = nextBatch.fetch_add(1, std::memory_order_relaxed);

		if (batch >= totalBatches)
		{
			break;
		}

		mathsRng.setSeed(static_cast<int>(config.seed + batch));

		GameSession session(bet, gameMaths, config.force);

		for (int i = 0; i < batchSize; i++)
		{
			GameCycle gameCycle(session, reused, gameMaths);
			GameLogic::playGame(gameMaths, gameCycle);
		}
		MeterUpdate::triggerUpdate();
	}
}


void doMonteCarloStdMultiThread(const GameMaths& gameMaths, const int bet, const UserConfig& config)
{
	constexpr int BATCH_SIZE = 1'000'000;
	const uint64_t totalBatches = config.simSize / BATCH_SIZE;
	const unsigned int threadCount = std::thread::hardware_concurrency() - 3;
	std::cout << "Using " << threadCount << " threads" << std::endl;

	config.gameConsole.clearScreen();
	config.gameConsole.setHeader(config.headerText, true, true);

	std::atomic<uint64_t> nextBatch{0};
	std::vector<std::jthread> threads;

	threads.reserve(threadCount);
	Timer simTimer("Monte Carlo");

	for (unsigned int i = 0; i < threadCount; i++)
	{
		threads.emplace_back(
			threadWorker,
			std::cref(gameMaths),
			std::ref(nextBatch),
			totalBatches,
			BATCH_SIZE,
			bet,
			std::cref(config)
		);
	}
	uint64_t last_seen = 0;
	while (last_seen < config.simSize)
	{
		const uint64_t current = GameCycleCompleteEvent::eventTotalHits.global();
		if (current != last_seen)
		{
			showRtpReport(current, bet * current, config.simSize);
			last_seen = current;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::cout << std::endl;
	simTimer.outputTime();
	Stats::outputStats(config.simSize, gameMaths.betType, gameMaths.rtp, bet,
	                   gameMaths.payTables["default"].symbolNames);
}

void doMonteCarloStd(const GameMaths& gameMaths, const int bet, const UserConfig& config)
{
	config.gameConsole.clearScreen();
	config.gameConsole.setHeader(config.headerText, true, true);

	GameCycleReused reused(gameMaths);


	GameSession session(bet, gameMaths, config.force);

	reportCount = reportAtCount = MILLION / 10;
	Timer simTimer("Monte Carlo");

	// Start the simulation loop
	uint64_t gameCount = 0;

	for (; gameCount < config.simSize; gameCount++)
	{
		GameCycle gameCycle(session, reused, gameMaths);
		GameLogic::playGame(gameMaths, gameCycle);

		// Print progress every reportCount plays
		if (--reportCount == 0)
		{
			int64_t gamesPlayed = gameCount + 1;
			showRtpReport(gamesPlayed, bet * gamesPlayed, config.simSize);
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
	Stats::outputStats(config.simSize, gameMaths.betType, gameMaths.rtp, bet,
	                   gameMaths.payTables["default"].symbolNames);
}

void doRnStreamCheck(const GameMaths& gameMaths, const int bet, const UserConfig& config)
{
	config.gameConsole.clearScreen();
	config.gameConsole.setHeader(config.headerText, true, true);

	std::atomic<uint64_t> nextBatch{0};
	std::vector<std::jthread> threads;

	Timer simTimer("Monte Carlo");
	GameCycleReused reused(gameMaths);
	auto& rnStream = dynamic_cast<RngData&>(*mathsRng.lpRng);
	GameSession session(bet, gameMaths, config.force);

	do
	{
		const GameSession sessionBeforeStart = session;
		rnStream.startGameSequence();
		GameCycle gameCycle(session, reused, gameMaths);
		GameLogic::playGame(gameMaths, gameCycle);
		rnStream.endGameSequence(gameCycle, session, sessionBeforeStart);
	}
	while (rnStream.currentGameIt != rnStream.games.end());

	std::cout << std::endl;
	simTimer.outputTime();
}

int frontendMain()
{
#ifdef FRONTEND
#ifdef FRONTEND_DEV
	NodeDevServer node;
	NodeDevServer::g_nodeServer = &node;

	std::atexit(NodeDevServer::shutdownHook);
	std::signal(SIGINT, NodeDevServer::signalHandler);
	std::signal(SIGTERM, NodeDevServer::signalHandler);
	std::set_terminate([]
	{
		NodeDevServer::shutdownHook();
		std::_Exit(1);
	});

	node.start();
#endif
	API::startAPIServer();
#endif
	return 0;
}

int simulatorMain()
{
#ifndef FRONTEND
	UserConfig config = UserConfig::getNewOrCachedConfig();
	ParSheet& parSheet = config.parSheet;
	if (config.rtp == -1)
	{
		for (int rtp : parSheet.rtps)
		{
			if (!config.seedChoice.has_value()) config.seed = std::random_device{}();
			GameMaths& gameMaths = *parSheet.gameMathsVariants[config.betType][rtp];
			const int bet = static_cast<int>(gameMaths.minBet * gameMaths.betTypeCost);
			doMonteCarloStdMultiThread(gameMaths, bet, config);
		}
	}
	else
	{
		// valid single RTP option
		GameMaths& gameMaths = *parSheet.gameMathsVariants[config.betType][config.rtp];
		const int bet = static_cast<int>(gameMaths.minBet * gameMaths.betTypeCost);
#ifdef SINGLETHREAD
		doMonteCarloStd(gameMaths, bet, config);
#else
		doMonteCarloStdMultiThread(gameMaths, bet, config);
#endif
	}
#endif
	return
		0;
}

int rnStreamMain()
{
	mathsRng.setRng(RngOptions::data);
	UserConfig config = UserConfig::getNewOrCachedConfig(true);
	const std::filesystem::path filePath = std::filesystem::path(GAME_SOURCE_DIR) / "srcBackend" / "thisTitle"
		/
		"parSheets" / config.rnStreamFileName;
	const int bet = dynamic_cast<RngData*>(mathsRng.lpRng)->openDataFile(filePath);
	doRnStreamCheck(*config.parSheet.gameMathsVariants[config.betType][config.rtp], bet, config);
	return 0;
}

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
	try
	{
#ifdef FRONTEND
		return frontendMain();
#else
#ifdef RN_STREAM
		return rnStreamMain();
#else
		return simulatorMain();
#endif
#endif
	}
	catch (std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what();
	}
}
