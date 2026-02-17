#include "RngData.h"

#include "../../thisTitle/state/GameCycle.h"
#include "../../thisTitle/state/GameSession.h"
#include <filesystem>
#include <ranges>

#include "../events/RNStreamProblemEvent.h"


std::vector<std::string> RngDataGame::split(const std::string& str, const char delim)
{
	std::vector<std::string> parts;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delim)) parts.push_back(item);
	return parts;
}

std::string RngDataGame::toString(const GameSession& session) const
{
	Json::StreamWriterBuilder builder;
	builder["indentation"] = ""; // compact (like JSON.stringify default)

	std::string gameString = Json::writeString(builder, session.toJson()) + "|" +
		std::to_string(gameNum) + "|" + std::to_string(bet) + "|" +
		std::to_string(totalWin) + "|";
	for (const RngDataSpin& spin : spins) gameString += spin.toString() + "|";
	return gameString;
}

RngDataGame::RngDataGame(const std::string& gameString)
{
	const std::vector<std::string> gameVector = split(gameString, '|');
	gameNum = std::stoi(gameVector[0]);
	bet = std::stoi(gameVector[1]);
	totalWin = std::stoi(gameVector[2]);
	for (size_t i = 3; i < gameVector.size(); i++) spins.push_back(RngDataSpin(gameVector[i]));
}


std::string RngDataSpin::toString() const
{
	std::string spinString = std::to_string(payout) + "," + std::to_string(spinNum);
	for (const uint32_t rn : rnStream) spinString += "," + std::to_string(rn);
	return spinString;
}

RngDataSpin::RngDataSpin(const std::string& spinString)
{
	const std::vector<std::string> spinVector = RngDataGame::split(spinString, ',');
	payout = std::stoi(spinVector[0]);
	spinNum = std::stoi(spinVector[1]);
	for (size_t i = 2; i < spinVector.size(); i++) rnStream.push_back(std::stoul(spinVector[i]));
}


/**********************************************/
// Implement the Rng virtual methods
/**********************************************/


uint32_t RngData::randInt(const uint32_t range, const std::string& sz)
{
	if (!currentSpinIt->used) startSpinSequence();
	if (currentSpinIt == currentGamePtr->spins.end()) return 0;
	if (rnStreamIt == currentSpinIt->rnStream.end())
	{
		reportProblem(ProblemNotEnoughRNs, *currentGamePtr, currentSpinIt, currentSpinIt->payout, range, 0,
		              sz);
		return 0;
	}

	const uint32_t value = *rnStreamIt++;
	if (value > range)
	{
		reportProblem(ProblemRNOutOfRange, *currentGamePtr, currentSpinIt, currentSpinIt->payout, range,
		              value, sz);
		return 0;
	}
	return value;
}

uint32_t RngData::randInt(const uint32_t range)
{
	return randInt(range, "");
}

/***************************************************************/
// Input sequence methods
/***************************************************************/

// Open CSV files containing RNG stream
// Return: the streams bet level in cent. Zero if error
int RngData::openDataFile(const std::filesystem::path& dataFilePath)
{
	std::string buffer = "";
	const std::string fileName = dataFilePath.filename().string();
	const std::string szMethod = "RngData.openDataFile";

	// Load RNG file
	mDataFile = std::ifstream(dataFilePath);

	if (!mDataFile)
	{
		reportError(szMethod, "Cannot open sequence file <" + fileName + ">");
	}
	else
	{
		mRngDataCSVIt = CSVIterator(mDataFile);
		if (mRngDataCSVIt == CSVIterator())
		{
			reportError(szMethod, "Sequence file <" + fileName + "> is empty");
		}
	}
	const int streamBet = readHeader();
	storeGames(streamBet);
	currentGameIt = games.begin();
	currentGamePtr = &*currentGameIt;
	return streamBet;
}

// Returns the stream's bet level in cent. Zero if error
int RngData::readHeader()
{
	const std::string szMethod = "RngData.readHeader";
	const std::string szRow1 = "bet";
	const std::vector<std::string> expectedHeaders = {
		"job", "betInJob", "betAsGrowingTotal", "play", "payout", "frig"
	};

	mDataRow = *mRngDataCSVIt;

	// row 1 - bet
	mDataIndex = 0;

	std::string firstCell = mDataRow.getString(mDataIndex++);
	if (firstCell.size() >= 3 &&
		static_cast<unsigned char>(firstCell[0]) == 0xEF &&
		static_cast<unsigned char>(firstCell[1]) == 0xBB &&
		static_cast<unsigned char>(firstCell[2]) == 0xBF)
	{
		firstCell.erase(0, 3);
	}

	if (firstCell != szRow1)
	{
		reportError(szMethod, "DataFile - Row1[0] != '" + szRow1 + "'");
		return 0;
	}
	int bet = mDataRow.getUint32Data(1);
	bet *= 100;

	// row 2 - headers
	mDataRow = *(++mRngDataCSVIt);
	if (mDataRow.size() != expectedHeaders.size())
	{
		if (mDataRow.size() > expectedHeaders.size())
			reportError(szMethod, "DataFile - More headers than expected", "Warning");
		else
			reportError(szMethod, "DataFile - Less headers than expected", "Warning");
		// Output header entries
		for (mDataIndex = 0; mDataIndex < static_cast<int>(expectedHeaders.size()); mDataIndex++)
			std::cout << mDataRow.getString(mDataIndex) << (mDataIndex + 1 < static_cast<int>(expectedHeaders.
				                                                size())
				                                                ? ", "
				                                                : "\r\n");
		++mRngDataCSVIt;
	}

	if (++mRngDataCSVIt == CSVIterator())
	{
		reportError("readHeader", "No data after Header");
		mDataFile.close();
		return 0;
	}
	return bet;
}

void RngData::storeGames(const int streamBet)
{
	uint32_t gameNum = 0;
	mDataRow = *mRngDataCSVIt;
	do
	{
		if (mDataRow.getUint32Data(1) != gameNum)
			throw std::runtime_error("Game number mismatch in betInJob column.");
		RngDataGame& game = games.emplace_back(gameNum, streamBet);
		do
		{
			int spinPayout = static_cast<int>(mDataRow.getDoubleData(4) * 100 / (streamBet / 100) + 0.5);
			int spinNum = mDataRow.getUint32Data(3);
			game.totalWin += spinPayout;
			RngDataSpin& spin = game.spins.emplace_back(spinPayout, spinNum);
			size_t colIndex = 5;
			do
			{
				spin.rnStream.push_back(mDataRow.getUint32Data(colIndex));
			}
			while (++colIndex < mDataRow.size());
			mDataRow = *(++mRngDataCSVIt);
		}
		while (mRngDataCSVIt != CSVIterator() && mDataRow.getUint32Data(1) == gameNum);

		gameNum++;
	}
	while (mRngDataCSVIt != CSVIterator());
}

void RngData::startGameSequence(RngDataGame* gamePtr)
{
	if (gamePtr != nullptr) currentGamePtr = gamePtr;
	currentSpinIt = currentGamePtr->spins.begin();
	startSpinSequence();
}


void RngData::startSpinSequence()
{
	if (currentSpinIt == currentGamePtr->spins.end())
	{
		reportProblem(ProblemNotEnoughSpins, *currentGamePtr, currentSpinIt);
	}
	currentSpinIt->used = true;
	rnStreamIt = currentSpinIt->rnStream.begin();
}


// Check end of sequence was reached, reports errors and returns true if error
void RngData::endSpinSequence(const int mathsWin)
{
	if (currentSpinIt == currentGamePtr->spins.end() || !currentSpinIt->used)
		return;

	const std::string method = "RngData.endSpinSequence";
	if (mathsWin > -1)
	{
		if (rnStreamIt != currentSpinIt->rnStream.end())
		{
			reportProblem(ProblemExcessRNs, *currentGamePtr, currentSpinIt);
		}
		else
		{
			const bool winMatch = currentSpinIt->payout == mathsWin;
			if (!winMatch)
			{
				reportProblem(ProblemSpinMismatch, *currentGamePtr, currentSpinIt, mathsWin);
			}
		}
	}

	++currentSpinIt;
}

// Stop using the current sequence
// Check end of sequence was reached and return the win multiplier
// pass game bet and mathsWin as cash
void RngData::endGameSequence(const GameCycle& gameCycle, const GameSession& session,
                              const GameSession& sessionBeforeStart)
{
	const std::string method = "RngData.endGameSequence";

	if (currentSpinIt != currentGamePtr->spins.end() && currentSpinIt->used) endSpinSequence(-1);
	if (currentSpinIt != currentGamePtr->spins.end())
	{
		reportProblem(ProblemExcessSpins, *currentGamePtr, currentGamePtr->spins.end());
	}
	else
	{
		bool winMatch = false;
		winMatch = (currentGamePtr->totalWin == gameCycle.totalWin);
		if (!winMatch)
		{
			reportProblem(ProblemGameMismatch, *currentGamePtr, currentGamePtr->spins.end(),
			              gameCycle.totalWin);
		}
	}
	if (currentGamePtr->hasProblem)
		outputGameToConsole(*currentGamePtr, gameCycle, session, sessionBeforeStart);
	currentGamePtr = &*(++currentGameIt);
}

std::string RngData::toCurrency(const int pence)
{
	std::ostringstream oss;
	oss << "£" << std::fixed << std::setprecision(2)
		<< (pence / 100.0);
	return oss.str();
}


void RngData::reportProblem(const Problem problem, RngDataGame& game,
                            const std::vector<RngDataSpin>::iterator spinIt, const int mathsWin,
                            const uint32_t range, const uint32_t rn, const std::string& sz)
{
	const bool spinProvided = spinIt != game.spins.end();
	std::string shortMessage;
	std::string longMessage;
	switch (problem)
	{
	case ProblemExcessRNs:
		shortMessage = "Excess RNs";
		longMessage = "Spin did not use all provided random numbers in RN stream.";
		break;
	case ProblemNotEnoughRNs:
		shortMessage = "Not enough RNs";
		longMessage =
			"Spin requested more random numbers than provided by RN stream. Next requested number had range of "
			+ std::to_string(range) + ".";
		if (sz != "") longMessage += "\n\tRN identifier: '" + sz + "'.";
		break;
	case ProblemSpinMismatch:
		shortMessage = "Spin payout mismatch";
		if (!spinProvided)
			longMessage =
				"Spin payout mismatch provided but spin was not provided to reportProblem method. Maths payout: "
				+ toCurrency(mathsWin) + ".";
		longMessage = "RN stream payout (" + toCurrency(spinIt->payout) + ") != maths payout (" +
			toCurrency(mathsWin) + ").";
		break;
	case ProblemGameMismatch:
		shortMessage = "Game payout mismatch";
		longMessage = "RN stream payout (" + toCurrency(game.totalWin) + ") != maths payout (" +
			toCurrency(mathsWin) + ").";
		break;
	case ProblemExcessSpins:
		shortMessage = "Excess spins";
		longMessage = "Game did not use all spins provided by RN stream.";
		break;
	case ProblemNotEnoughSpins:
		shortMessage = "Not enough spins";
		longMessage = "Game requested more spins than provided by RN stream.";
		break;
	case ProblemRNOutOfRange:
		shortMessage = "Random number out of range";
		longMessage = "Random number provided (" + std::to_string(rn) + ") was out of requested range (" +
			std::to_string(range) + ").";
		if (sz != "") longMessage += "\n\tRN identifier: '" + sz + "'.";
		break;
	default:
		shortMessage = "Unknown problem";
		longMessage = "RngData class reported a problem but didn't say what type.";
	}

#ifdef FRONTEND
	gameCyclePtr->recordEvent<RNStreamProblemEvent>(shortMessage, longMessage);
#else
	if (game.hasProblem)
	{
		std::cout << "\t(Additional problem: " << shortMessage;
		std::cout << ".)" << std::endl;
		return;
	}
	game.hasProblem = true;

	std::cout << std::endl << std::endl;
	std::cerr << "Problem with game " + std::to_string(game.gameNum);
	if (spinProvided) std::cerr << ", Spin " + std::to_string(spinIt->spinNum);
	std::cerr << ": ";
	std::cerr << longMessage << std::endl;
#endif
}

void RngData::outputGameToConsole(const RngDataGame& gameToOutput, const GameCycle& gameCycle,
                                  const GameSession& session, const GameSession& sessionBeforeStart)
{
#ifdef RN_STREAM
	std::cout <<
		"\n\tIf there is no persistence or persistence doesn't matter here, you can play this game in the\n\tthe frontend by putting the following in RN Stream text box and press spin:"
		<<
		std::endl;
	std::cout << "\t\t" << gameToOutput.toString(sessionBeforeStart) << std::endl;
	std::cout << "\n\tSESSION BEFORE GAME: " << std::endl;
	prettyPrintJson(sessionBeforeStart.toJson(), "\t\t");
	std::cout << std::endl << "\n\tEVENTS: " << std::endl;
	for (const auto& event : gameCycle.events)
	{
		const std::string eventString = event->prettyPrint("\t\t");
		if (eventString.size() > 0)
			std::cout << event->prettyPrint("\t\t");
	}
	std::cout << "\n\tSESSION AFTER GAME: " << std::endl;
	prettyPrintJson(session.toJson(), "\t\t");
	std::cout << std::endl << std::string(100, '*');
#endif
}

void RngData::prettyPrintJson(const Json::Value& root, const std::string& baseIndent)
{
	Json::StreamWriterBuilder builder;
	builder["indentation"] = "  "; // normal nesting indent

	std::string json = Json::writeString(builder, root);

	std::ostringstream out;
	std::istringstream in(json);
	std::string line;

	while (std::getline(in, line))
	{
		out << baseIndent << line << '\n';
	}

	std::cout << out.str();
}
