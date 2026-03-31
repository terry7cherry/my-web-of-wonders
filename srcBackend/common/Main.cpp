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
#include "utils/utils.h"
#include "../thisTitle/stats/SessionStrategies.h"
#include "../thisTitle/stats/SessionStats.h"

Console gameConsole;

int reportAtCount = MILLION / 10;
int reportCountLimit = 100 * MILLION;
int reportCount = reportAtCount;


void showRtpReport(const uint64_t gamesPlayed, const uint64_t totalBet, const uint64_t simSize,
                   const bool winCapSet, Timer simTimer)
{
	// Header with spins and percent complete
	std::printf("\033[1A\r%s%10" PRIu64 " spins. %.2f%% complete. ", "\n", gamesPlayed,
	            static_cast<double>(100 * static_cast<long double>(gamesPlayed) / static_cast<long double>(
		            simSize)));

	// Insert ETA between % complete and RTP, based on elapsed time since sim start
	if (gamesPlayed > 0 && simSize > gamesPlayed)
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto elapsed = now - simTimer.tStart;
		const long double elapsedSec = std::chrono::duration_cast<std::chrono::duration<long double>>(elapsed)
			.count();
		const long double perGameSec = elapsedSec / static_cast<long double>(gamesPlayed);
		const long double remainingGames = static_cast<long double>(simSize - gamesPlayed);
		long double etaSecLD = perGameSec * remainingGames;
		if (etaSecLD < 0)
			etaSecLD = 0;
		const long long etaSec = static_cast<long long>(etaSecLD + 0.5L); // round to nearest second
		const long long h = etaSec / 3600;
		const long long m = (etaSec % 3600) / 60;
		const long long s = etaSec % 60;
		std::printf("ETA: %lld:%02lld:%02lld. ", h, m, s);
	}

	// Continue with RTP output
	std::printf("RTP");
	const long double percent = static_cast<long double>(GameCycleCompleteEvent::eventTotalWin.global()) * 100
		/ static_cast<double>(totalBet);
	std::printf(": %s=%.2Lf%%", "total", percent);
	if (!winCapSet) return;

	const long double cappedPercent = static_cast<long double>(GameCycleCompleteEvent::cappedTotalWin.
			global()) * 100
		/ static_cast<double>(totalBet);
	std::printf(", %s=%.2Lf%%", "capped", cappedPercent);

	const long double lostToCap = static_cast<long double>(
		GameCycleCompleteEvent::eventTotalWin.global() -
		GameCycleCompleteEvent::cappedTotalWin.global()
	) * 100 / static_cast<double>(totalBet);
	std::printf(", %s=%.3Lf%%", "lost to cap", lostToCap);

	const uint64_t capHitCount = GameCycleCompleteEvent::capHitCount.global();
	std::printf(". %s=", "Cap hit rate");
	if (!capHitCount)
		std::printf("Inf");
	else
		std::printf("%s", addThousandsSeparator(gamesPlayed / capHitCount).c_str());
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

	try
	{
		while (!st.stop_requested())
		{
			const uint64_t batch = nextBatch.fetch_add(1, std::memory_order_relaxed);
			if (batch >= totalBatches) break;
#ifndef RN_STREAM
			mathsRng.setSeed(static_cast<int>(config.seed + batch));
#endif

			GameSession session(bet, gameMaths, config.force);

			for (int i = 0; i < batchSize; i++)
			{
				GameCycle gameCycle(session, reused, gameMaths);
				GameLogic::playGame(gameMaths, gameCycle);
			}
			MeterUpdate::triggerUpdate();
		}
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "Error playing game: " << e.what() << std::endl;
	}
}


#ifndef RN_STREAM
void doMonteCarloStdMultiThread(const GameMaths& gameMaths, const int bet, const UserConfig& config)
{
	constexpr int BATCH_SIZE = 100'000;
	const uint64_t totalBatches = config.simSize / BATCH_SIZE;
#ifdef DEBUG
	const unsigned int threadCount = 1;
#else
	const unsigned int threadCount = std::thread::hardware_concurrency() - 3;
#endif
	std::cout << "Using " << threadCount << " threads" << std::endl;

	config.gameConsole.clearScreen();
	config.gameConsole.setHeader(config.headerText, true, true);

	std::atomic<uint64_t> nextBatch{0};
	std::vector<std::jthread> threads;

	const bool winCapSet = gameMaths.ints.contains("winCap");

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
			showRtpReport(current, bet * current, config.simSize, winCapSet, simTimer);
			last_seen = current;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::cout << std::endl;
	simTimer.outputTime();
	Stats::outputStats(config.simSize, gameMaths.betType, gameMaths.rtp, bet,
	                   Forces::simForceNames.at(config.force),
	                   config.seed,
	                   gameMaths.payTables["default"].symbolNames,
	                   gameMaths.ints.contains("winCap") ? gameMaths.ints["winCap"] : 0);
}

void doMonteCarloStd(const GameMaths& gameMaths, const int bet, const UserConfig& config)
{
	config.gameConsole.clearScreen();
	config.gameConsole.setHeader(config.headerText, true, true);

	GameCycleReused reused(gameMaths);

	const bool winCapSet = gameMaths.ints.contains("winCap");

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
			showRtpReport(gamesPlayed, bet * gamesPlayed, config.simSize, winCapSet, simTimer);
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
	                   Forces::simForceNames.at(config.force),
	                   config.seed,
	                   gameMaths.payTables["default"].symbolNames,
	                   gameMaths.ints.contains("winCap") ? gameMaths.ints["winCap"] : 0);
}
#endif

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
	while (!rnStream.noMoreGames());

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

#ifndef FRONTEND
#ifndef RN_STREAM
int simulatorMain(const int argc, char* argv[])
{
	UserConfig config = UserConfig::getNewOrCachedConfig(argc, argv);
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
	return
		0;
}
#endif
#endif

#ifdef SESSION
int sessionMain(int argc, char* argv[])
{
	// Get session-specific configuration
	UserConfig config = UserConfig::getNewOrCachedConfig(argc, argv);
	ParSheet& parSheet = config.parSheet;
	// Single RTP only (as per requirement)
	GameMaths& gameMaths = *parSheet.gameMathsVariants[config.betType][config.rtp];
	const int bet = static_cast<int>(gameMaths.minBet * gameMaths.betTypeCost);

	const auto& [name, initialBalanceMult, shouldStop] = config.sessionStrategy;

	// Multithreaded sessions execution
	constexpr uint64_t SESSIONS_PER_BATCH = 1000;
	const uint64_t totalBatches = std::max<uint64_t>(1, config.simSize / SESSIONS_PER_BATCH);
#ifdef DEBUG
const unsigned int threadCount = 1;
#else
const unsigned int threadCount = std::max(1u, std::thread::hardware_concurrency() - 3);
#endif

// Prepare output folder: same as normal but with "session" in the name after timestamp
namespace fs = std::filesystem;
const fs::path simsRoot = fs::path(GAME_SOURCE_DIR) / "Simulations";
fs::create_directories (simsRoot);
const auto now = std::chrono::system_clock::now();
const std::time_t t = std::chrono::system_clock::to_time_t(now);
std::tm tm{};
#if defined(_WIN32)
localtime_s (&tm, &t);
#else
localtime_r (&t, &tm);
#endif
std::ostringstream oss;
oss<< std::put_time (&tm, "%Y-%m-%d_%H.%M");
oss<< "_session";
	if (config.simSize> BILLION)
oss<< "_" << (config.simSize/ BILLION) << "b";
	else
oss<< "_" << (config.simSize/ MILLION) << "m";
oss<< "_RTP" << gameMaths.rtp<< "_bet" << bet;
	if (gameMaths.betType!= "default") oss<< "_" << gameMaths.betType;
const fs::path outFolder = simsRoot / oss.str();
fs::create_directories (outFolder);
const fs::path csvPath = outFolder / (std::string("Sessions_") + name + ".csv");

// Shared results container
std::mutex rowsMutex;
std::vector<std::vector<float>> rows;
rows.reserve (static_cast<size_t>(config.simSize));

std::atomic<uint64_t> nextBatch{0};
std::vector<std::jthread> threads;
threads.reserve (threadCount);

auto sessionWorker = [&](const std::stop_token& st,
                         const uint64_t totalBatchesLocal,
                         const uint64_t sessionsPerBatchLocal,
                         const int baseSeed)
{
	GameCycleReused reused(gameMaths);
	try
	{
		while (!st.stop_requested())
		{
			const uint64_t batch = nextBatch.fetch_add(1, std::memory_order_relaxed);
			if (batch >= totalBatchesLocal) break;
			mathsRng.setSeed(baseSeed + static_cast<int>(batch));

			for (uint64_t i = 0; i < sessionsPerBatchLocal; ++i)
			{
				// Initialise a new logical session
				GameSession session(bet, gameMaths, config.force);
				long long balance = static_cast<long long>(initialBalanceMult) * static_cast<long
					long>(bet);
				SessionStats::beginSession(balance);

				uint64_t spinsPlayed = 0;
				bool done = false;
				while (!done)
				{
					GameCycle cycle(session, reused, gameMaths);
					GameLogic::playGame(gameMaths, cycle);

					// Bankroll update
					balance -= bet;
					balance += cycle.totalWin;

					// Per-spin stats
					SessionStats::storeSessionStatsForSpin(cycle, bet);

					++spinsPlayed;
					done = shouldStop(balance, spinsPlayed, session, cycle);
				}

				// Finalise one row
				std::vector<float> row = SessionStats::storeSessionStatsForSession(
					balance, gameMaths.payTables["default"].symbolNames);
				// Prepend SessionIndex (1-based across whole run). Approximate index by current size+1; will adjust after join
				// We will store without index here and add indices during CSV write to avoid races.

				// Push thread-safely
				{
					std::lock_guard lk(rowsMutex);
					rows.emplace_back(std::move(row));
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Session worker error: " << e.what() << std::endl;
	}
};

// Launch threads
	for (unsigned int ti = 0; ti<threadCount;++ti)
	{
		threads.emplace_back(sessionWorker,
		                     totalBatches,
		                     SESSIONS_PER_BATCH,
		                     static_cast<int>(config.seed));
	}

// Wait for threads to finish (jthread joins on destruction)
threads.clear();

// If we produced slightly more rows due to rounding, trim
	if (rows.size()> config.simSize) rows.resize (static_cast<size_t>(config.simSize));

// Write CSV
std::ofstream out(csvPath);
	if (!out.is_open())
	{
		std::cerr << "Failed to open sessions CSV: " << csvPath << std::endl;
	}
	else
	{
		// Compose headers: include SessionIndex first
		std::vector<std::string> headers = SessionStats::headers();
		// Ensure first header is SessionIndex
		if (headers.empty() || headers.front() != std::string("SessionIndex"))
			headers.insert(headers.begin(), "SessionIndex");
		for (size_t i = 0; i < headers.size(); ++i)
		{
			out << headers[i];
			if (i + 1 < headers.size()) out << ",";
		}
		out << "\n";
		// Write rows with SessionIndex prepended
		for (size_t i = 0; i < rows.size(); ++i)
		{
			out << (i + 1);
			const auto& r = rows[i];
			for (float v : r)
			{
				out << "," << v;
			}
			out << "\n";
		}
		out.close();
		std::cout << "Wrote sessions CSV: " << csvPath << std::endl;
	}
	return
		0;
}
#endif

#ifdef RN_STREAM
int rnStreamMain(int argc, char* argv[])
{
	mathsRng.setRng(RngOptions::data);
	UserConfig config = UserConfig::getNewOrCachedConfig(argc, argv);
	const std::filesystem::path filePath = std::filesystem::path(GAME_SOURCE_DIR) / "srcBackend" / "thisTitle"
		/
		"parSheets" / config.rnStreamFileName;
	const int bet = dynamic_cast<RngData*>(mathsRng.lpRng)->openDataFile(filePath);
	doRnStreamCheck(*config.parSheet.gameMathsVariants[config.betType][config.rtp], bet, config);
	return 0;
}
#endif

int main(int argc, char* argv[])
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
		return rnStreamMain(argc, argv);
#else
#ifdef SESSION
		return sessionMain(argc, argv);
#else
		return simulatorMain(argc, argv);
#endif
#endif
#endif
	}
	catch (std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what();
	}
}
