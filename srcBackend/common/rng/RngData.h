#pragma once
#include "../headers/pch.h"
#include "Rng.h"
#include "CSVIterator.h"
#include "../utils/utils.h"
#include <json/json.h>
#include <filesystem>
// Forward declarations to avoid pulling gameplay/state (and grids) into most TUs via this header
struct GameCycle;
struct GameSession;

enum Problem
{
	ProblemSpinMismatch,
	ProblemGameMismatch,
	ProblemExcessSpins,
	ProblemNotEnoughSpins,
	ProblemExcessRNs,
	ProblemNotEnoughRNs,
	ProblemRNOutOfRange
};

class RngDataSpin
{
public:
	int payout;
	int spinNum;
	std::vector<uint32_t> rnStream;
	bool used = false;

	RngDataSpin(const int payout, const int spinNum) : payout(payout), spinNum(spinNum)
	{
	}

	RngDataSpin(const std::string& spinString);

	std::string toString() const;
};

class RngDataGame
{
public:
	uint32_t gameNum;
	uint32_t bet;
	unsigned int totalWin{};
	std::vector<RngDataSpin> spins;
	bool hasProblem = false;

	RngDataGame(const uint32_t gameNum, const uint32_t bet)
		: gameNum(gameNum), bet(bet)
	{
	}

	RngDataGame(const std::string& gameString);

	static std::vector<std::string> split(const std::string& str, const char delim);


	std::string toString(const GameSession& session) const;
};

class RngData : public Rng
{
	friend class MathsRng;

protected:
	size_t mDataIndex = 0;
	CSVRow mDataRow;
	std::vector<RngDataGame> games;
	std::vector<RngDataGame>::iterator currentGameIt;
	RngDataGame* currentGamePtr;
	std::vector<RngDataSpin>::iterator currentSpinIt;
	std::vector<uint32_t>::iterator rnStreamIt;
	std::ifstream mDataFile;
	GameCycle* gameCyclePtr;
	//MTRand* lpMtRng = nullptr;

	CSVIterator mRngDataCSVIt;

	void setSeed(const uint32_t newSeed [[maybe_unused]]) override
	{
	}

	uint32_t randInt(uint32_t range, const std::string& sz) override;

	uint32_t randInt(uint32_t range) override;

	int readHeader();

	void storeGames(int bet);

	void startSpinSequence();

	void reportProblem(Problem problem, RngDataGame& game, std::vector<RngDataSpin>::iterator spinIt,
	                   int mathsWin = -1, uint32_t range = 0, uint32_t rn = 0,
	                   const std::string& sz = "");

	static void outputGameToConsole(const RngDataGame& gameToOutput, const GameCycle& gameCycle,
	                                const GameSession& session, const GameSession& sessionBeforeStart);

	static std::string toCurrency(int pence);

	static void prettyPrintJson(const Json::Value& root, const std::string& baseIndent);

public:
	void setGameCycle(GameCycle* gameCycle) { gameCyclePtr = gameCycle; }

	// Open CSV files containing RNG stream
	// Return: the streams bet level in cent. Zero if error
	int openDataFile(const std::filesystem::path& dataFilePath);


	// set the sequence of RNG results to use for the next game
	// returns the 1st entry from the sequence, game number
	void startGameSequence(RngDataGame* gamePtr = nullptr);


	// Check end of sequence was reached, reports errors and returns true if error
	void endSpinSequence(int mathsWin);

	// Stop using the current sequence
	// Check end of sequence was reached and return the win multiplier
	// pass game bet and mathsWin as cash
	void endGameSequence(const GameCycle& gameCycle, const GameSession& session,
	                     const GameSession& sessionBeforeStart);

	bool noMoreGames() const
	{
		return currentGameIt == games.end();
	}
};

