#pragma once
#include <map>

#include "../gameMaths/mathElements/ReelSet.h"
#include "../spin/SpinResult.h"
#include "../gameMaths/GameMaths.h"


enum class symbolMatchTypes
{
	exact, greater, less, greaterEqual, lessEqual
};

enum class GridDataTypes
{
	symbol, multiplier
};

class Grid
{
protected:
	std::map<Symbol, std::string> mTextMap;
	std::map<int, short> mColourMap;
	Console* lpConsole = nullptr;

public:
	/************************************/
	// generic grid details
	/************************************/

	size_t reelDataBytes; // how many byte in reelSymbolVect[reel] upto maxRows
	size_t maxReels, maxRows, maxCells;
	std::vector<size_t> reelHeightVect;

	/************************************/
	// state
	/************************************/

	std::vector<bool> lockedVect;
	std::vector<std::vector<Symbol>> reelSymbolVect; // [reel][row]
	std::vector<std::vector<unsigned int>> multiplierVect; // [reel][row]

	// constructor
	Grid(size_t reels, size_t rows);

	// constructor from GameMaths
	Grid(const GameMaths& maths);

	/************************************/
	// Initialise
	/************************************/

	// set all reel heights to maxRows
	void setReelHeights();

	// set reel heights to values in the passed vector
	void setReelHeights(const std::vector<size_t>& heightVect);

	// if Grid dimensions match, then copies 'reelSymbolVect'
	void copy(const Grid& sourceGrid);

	// if Grid dimensions match, then copies all occurances of 'symbol' to 'reelSymbolVect'
	void copySymbol(const Grid& sourceGrid, Symbol symbol);

	/************************************/
	// Pre Spin
	/************************************/

	// set all grid symbols to SYMBOL_NONE and multipliers to 1
	void reset(void)
	{
		for (int reel = 0; reel < maxReels; reel++)
		{
			for (int row = 0; row < maxRows; row++)
			{
				reelSymbolVect[reel][row] = SYMBOL_NONE;
			}
		}
	}

	// set all grid multipliers to 1
	void resetMultipliers(void)
	{
		for (int reel = 0; reel < maxReels; reel++)
		{
			for (int row = 0; row < maxRows; row++)
			{
				multiplierVect[reel][row] = 1;
			}
		}
	}


	/**************************************************/
	// Apply spin
	/**************************************************/

	/**
	 * Sets symbols according to the passed spinResult object.
	 *
	 * Note you can pass ReelSet or ReelSetWeightTable (or other SpinResult constructor parameters), and implicit conversion will perform the spin automatically.
	 *
	 * @param spinResult A reference to a SpinResult object that contains the details of the spin to be applied.
	 * @param getLockedReels Reels marked as locked will remain unchanged unless getLockedReels is set to true.
	 */
	void applySpin(const SpinResult& spinResult, bool getLockedReels = false);

	void setSymbolsForReel(size_t reelNum, const ReelSet* reelSetPtr, size_t reelStop);

	/**************************************************/
	// Spin lock
	/**************************************************/

	// Set lockedVect for each reel in noSpinVect.
	// noSpinVect is a list of reel indexes that are to be locked
	void setSpinLock(const std::vector<int>& noSpinVect);

	// Set lockedVect = noSpinVect
	// spinStateVect contains a value for every reel
	void setSpinLock(const std::vector<bool>& spinStateVect);

	// remove all locked reels
	void clearSpinLock();


	/**************************************************/
	// Convert index <-> grid positions
	/**************************************************/

	// returns {reel, row} for the passed grid index e.g.
	//        Reel 0   1   2   3   4
	//  Row 0    0   1   2   3   4 \
	//  Row 1    5   6   7   8   9  |< index
	//  Row 2   10  11  12  13  14 /
	std::vector<size_t> getGridPosition(const size_t index) const
	{
		size_t row = index / maxReels;
		size_t reel = index - (row * maxReels);
		return {reel, row};
	}


	// returns grid index for passed reel, row
	size_t getGridIndex(const size_t reel, const size_t row) const
	{
		return (row * maxReels) + reel;
	}

	// returns grid index for passed {reel, row}
	size_t getGridIndex(const std::vector<size_t>& gridPos) const
	{
		return getGridIndex(gridPos[0], gridPos[1]);
	}

	/**************************************************/
	// Access Symbols
	/**************************************************/

	// return the symbol at the passed reel, row
	Symbol getReelSymbol(const size_t reel, const size_t row) const
	{
		return reelSymbolVect[reel][row];
	}

	// return the symbol at the passed {reel, row}
	Symbol getSymbol(const std::vector<size_t>& gridPos) const
	{
		return reelSymbolVect[gridPos[0]][gridPos[1]];
	}

	// return the symbol at the passed index e.g.
	//   0,  1,  2,  3,  4,
	//	 5,  6,  7,  8,  9,
	//	10, 11, 12, 13, 14
	Symbol getSymbol(const size_t index) const
	{
		return getSymbol(getGridPosition(index));
	}

	/**************************************************/
	// Set Symbols
	/**************************************************/

	// set the symbol at the passed reel, row
	void setSymbol(const size_t reel, const size_t row, const Symbol newSymbol)
	{
		reelSymbolVect[reel][row] = newSymbol;
	}

	// set the symbol at the passed gridPos {reel, row}
	inline void setSymbol(const std::vector<size_t>& gridPos, const Symbol newSymbol)
	{
		reelSymbolVect[gridPos[0]][gridPos[1]] = newSymbol;
	}

	// return the symbol at the passed index:
	//		 0,  1,  2,  3,  4,
	//		 5,  6,  7,  8,  9,
	//		10, 11, 12, 13, 14
	void setSymbol(const size_t index, const Symbol newSymbol)
	{
		setSymbol(getGridPosition(index), newSymbol);
	}

	// replace current reel symbols with 'count' of 'newSymbol' from top to bottom
	void setReelSymbols(size_t reel, Symbol newSymbol, int count);

	// fill 'reel' with 'newSymbol'
	void setReelSymbols(size_t reel, Symbol newSymbol);

	// replace current reel symbols with 'newSymbol' for each element in rowVect
	// rowVect may contain:
	//		1. An entry for each row position, each non-zero entry is set e.g. { 0,0,1}
	//		2. The indexes of rows to set e.g. {2}
	// returns vector of grid indexes that have been set
	std::vector<size_t> setReelSymbols(size_t reel, Symbol newSymbol,
	                                   const std::vector<size_t>& rowVect);


	/**************************************************/
	// Set Multipliers
	/**************************************************/

	// set the multiplier at passed gridPos [reel][row]
	void setMultiplier(const size_t reel, const size_t row, const int val)
	{
		multiplierVect[reel][row] = val;
	}

	// set the multiplier at the passed gridPos {reel, row}
	void setMultiplier(const std::vector<size_t>& gridPos, const int val)
	{
		setMultiplier(gridPos[0], gridPos[1], val);
	}

	// set the multiplier at the passed index
	//   0,  1,  2,  3,  4,
	//   5,  6,  7,  8,  9,
	//  10, 11, 12, 13, 14
	void setMultiplier(const size_t index, const int val)
	{
		setMultiplier(getGridPosition(index), val);
	}

	/**************************************************/
	// Get Multipliers
	/**************************************************/

	// get the multiplier at passed gridPos [reel][row]
	int getMultiplier(const size_t reel, const size_t row) const
	{
		return multiplierVect[reel][row];
	}

	// get the multiplier at the passed gridPos {reel, row}
	int getMultiplier(const std::vector<size_t>& gridPos) const
	{
		return getMultiplier(gridPos[0], gridPos[1]);
	}

	// get the multiplier at the passed index
	//   0,  1,  2,  3,  4,
	//   5,  6,  7,  8,  9,
	//  10, 11, 12, 13, 14
	inline int getMultiplier(const int index) const
	{
		return getMultiplier(getGridPosition(index));
	}

	/************************************/
	// Analysis
	/************************************/

	// count number of sym on passed reel
	// optional parameter matchType
	int getReelSymbolCount(size_t reel, Symbol sym,
	                       symbolMatchTypes matchType = symbolMatchTypes::exact) const;

	// count number of sym in the grid
	// optional parameter matchType
	// returns total count and fills reelSymbolCountVect with individual reel counts
	int getSymbolCount(Symbol sym, symbolMatchTypes matchType = symbolMatchTypes::exact) const;

	// returns row of passed symbol on passed reel
	// returns -1 if symbol not found
	std::optional<size_t> getReelSymbolRow(size_t reel, uint8_t sym,
	                                       symbolMatchTypes matchType = symbolMatchTypes::exact)
	const;

	/************************************/
	// Grid processing - expand
	/************************************/

	// Look for symbol on reel. If found then expand symbol to fill reel
	// If optional newSym then expand and replace with newSym
	// return number of symbol on reel before expanded
	int expandReelSymbol(size_t reel, uint8_t symbol, uint8_t newSym = SYMBOL_NONE);

	// Look for symbol on all reels. If found then expand symbol to fill reel
	// Return number of reels expanded
	int expandSymbol(uint8_t symbol);

	// Look for symbol on each reel in reelCheckVect parameter can be passed as{ 0, 2, 4 } or as an
	// If found then expand symbol to fill reel
	// Return number of reels expanded
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int expandSymbol(uint8_t symbol, const std::vector<size_t>& reelCheckVect);

	// Look for symbol on each reel.
	// If found then expand symbol to fill reel with newSym
	// Return number of reels expanded
	int expandSymbolToNew(uint8_t symbol, uint8_t newSym);

	// Look for symbol on each reel in reelCheckVect.
	// If found then expand symbol to fill reel with newSym
	// Return number of reels expanded
	int expandSymbolToNew(uint8_t symbol, uint8_t newSym, const std::vector<int>& reelCheckVect);

	/************************************/
	// Grid processing - replace
	/************************************/

	// On passed reel replace all occurances of symOld with symNew
	// return number of replaced symbols
	int replaceReelSymbol(size_t reel, uint8_t symOld, uint8_t symNew);

	// replace all grid occurances of symOld with symNew
	// return number reels where symbol was replaced
	int replaceSymbol(uint8_t symOld, uint8_t symNew);

	// For each reel in reelCheckVect, replace all occurances of symOld with symNew.
	// return number reels where symbol was replaced
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int replaceSymbol(uint8_t symOld, uint8_t symNew, const std::vector<int>& reelCheckVect);

	// replace count random symbols in the grid with newSymbol (not already newSymbol)
	std::vector<int> replaceRandomSymbols(uint8_t newSymbol, int count,
	                                      std::vector<int> blockedIndexVect = {});

	/************************************/
	// Grid processing - replace not
	/************************************/

	// On passed reel replace all symbols that are NOT symOld with symNew
	// return number of replaced symbols
	int replaceReelNotSymbol(int reel, uint8_t symOld, uint8_t symNew);

	// In the whole grid replace all symbols that are NOT symOld with symNew
	// return number reels where symbol was replaced
	int replaceNotSymbol(uint8_t symOld, uint8_t symNew);

	/************************************/
	// Display
	/************************************/

	void setConsole(Console* console)
	{
		lpConsole = console;
	}

	void setTextMap(const std::map<uint8_t, std::string>& textMap)
	{
		mTextMap = textMap;
	}

	void setColourMap(const std::map<int, short>& colourMap)
	{
		mColourMap = colourMap;
	}

	void showReels(int tabCount) const;

	void showReels(int tabCount, std::vector<int>& highlightVect, unsigned short colour) const;

	std::vector<std::string> getGridTextVect(void);
};

