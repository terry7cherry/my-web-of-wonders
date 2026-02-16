#pragma once

// cpp-dev-library
#include "../gameMaths/mathElements/PayTable.h"
#include "PayGrid.h"
#include "../wins/WaysWin.h"
#include "../console/Console.h"

// game globals
extern Console gameConsole;


// A reel Grid with Ways wins
class WaysGrid :
	public PayGrid
{
public:
	int winWays = 0;
	int maxWays = 0;

	explicit WaysGrid(const GameMaths& maths);

	WaysGrid(size_t reels, size_t rows, const PayTable& payTable);

	~WaysGrid();

	// implements Grid::reset(); and WaysGrid::winVectorReset()
	void reset();

	/************************************/
	// Win methods
	/************************************/

	// deletes all entries from the winVector
	void winVectorReset();

	int evaluateWins(unsigned int betMultiplier,
	                 MultipliersCombineType combineType = MultipliersCombineType::multiply);

	// Checks if there is a win for passed "symbol"
	// If so applies multipler to the win and returns the incremental win: (initialWin * multiplier) - initialWin
	int setWinMultiplier(Symbol symbol, int multiplier) const;

	// generates the vector using passed symbolMap
	std::vector<std::string> getWinTextVect(const std::map<Symbol, std::string>& symbolMap) const;

	// generates the vector using preset, Grid.mTextMap
	std::vector<std::string> getWinTextVect() const;

	//	generates the vector of winPosVect for each active win
	//	winPosVect[reel][row]: 0= no win, 1= winning symbol, 2= winning wild
	std::vector<std::vector<std::vector<int>>> getWinPosVect(void) const;

	unsigned int applyGridMultiplier(const MultipliersCombineType combineType,
	                                 const std::optional<int> specialMultiplier = std::nullopt);

	void applySpin(const SpinResult& spinResult, bool getLockedReels = false);

private:
	std::vector<std::unique_ptr<WaysWin>> winsToProcess;
};
