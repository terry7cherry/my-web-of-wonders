#pragma once
#include <map>

#include "../mathElements/ReelSet.h"
#include "../spin/SpinResult.h"


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
	std::map<int, std::string> mTextMap;
	std::map<int, int> mColourMap;
	Console* lpConsole = nullptr;

public:
	/************************************/
	// generic grid details
	/************************************/

	int reelDataBytes; // how many bytes in reelSymbolVect[reel] upto maxRows
	int maxReels, maxRows, maxCells;
	std::vector<int> reelHeightVect;

	/************************************/
	// state
	/************************************/

	std::vector<bool> lockedVect;
	std::vector<std::vector<int>> reelSymbolVect; // [reel][row]
	std::vector<std::vector<int>> multiplierVect; // [reel][row]

	// constructor
	Grid(int reels, int rows);

	/************************************/
	// Initialise
	/************************************/

	// set all reel heights to maxRows
	void setReelHeights();

	// set reel heights to values in the passed vector
	void setReelHeights(const std::vector<int>& heightVect);

	// if Grid dimensions match, then copies 'reelSymbolVect'
	void copy(const Grid& sourceGrid);

	// if Grid dimensions match, then copies all occurances of 'symbol' to 'reelSymbolVect'
	void copySymbol(const Grid& sourceGrid, int symbol);

	/************************************/
	// Pre Spin
	/************************************/

	// set all grid symbols and multipliers to -1
	void reset(void)
	{
		for (int reel = 0; reel < maxReels; reel++)
		{
			for (int row = 0; row < maxRows; row++)
			{
				reelSymbolVect[reel][row] = multiplierVect[reel][row] = -1;
			}
		}
	}

	// set all grid multipliers to -1
	void resetMultipliers(void)
	{
		for (int reel = 0; reel < maxReels; reel++)
		{
			for (int row = 0; row < maxRows; row++)
			{
				multiplierVect[reel][row] = -1;
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
	int getReelSymbol(const size_t reel, const size_t row) const
	{
		return reelSymbolVect[reel][row];
	}

	// return the symbol at the passed {reel, row}
	int getSymbol(const std::vector<size_t>& gridPos) const
	{
		return reelSymbolVect[gridPos[0]][gridPos[1]];
	}

	// return the symbol at the passed index e.g.
	//   0,  1,  2,  3,  4,
	//	 5,  6,  7,  8,  9,
	//	10, 11, 12, 13, 14
	int getSymbol(const size_t index) const
	{
		return getSymbol(getGridPosition(index));
	}

	/**************************************************/
	// Set Symbols
	/**************************************************/

	// set the symbol at the passed reel, row
	void setSymbol(const size_t reel, const size_t row, const int newSymbol)
	{
		reelSymbolVect[reel][row] = newSymbol;
	}

	// set the symbol at the passed gridPos {reel, row}
	inline void setSymbol(const std::vector<size_t>& gridPos, const int newSymbol)
	{
		reelSymbolVect[gridPos[0]][gridPos[1]] = newSymbol;
	}

	// return the symbol at the passed index:
	//		 0,  1,  2,  3,  4,
	//		 5,  6,  7,  8,  9,
	//		10, 11, 12, 13, 14
	void setSymbol(int index, int newSymbol)
	{
		setSymbol(getGridPosition(index), newSymbol);
	}

	// replace current reel symbols with 'count' of 'newSymbol' from top to bottom
	void setReelSymbols(size_t reel, int newSymbol, int count);

	// fill 'reel' with 'newSymbol'
	void setReelSymbols(size_t reel, int newSymbol);

	// replace current reel symbols with 'newSymbol' for each element in rowVect
	// rowVect may contain:
	//		1. An entry for each row position, each non-zero entry is set e.g. { 0,0,1}
	//		2. The indexes of rows to set e.g. {2}
	// returns vector of grid indexes that have been set
	std::vector<size_t> setReelSymbols(size_t reel, int newSymbol,
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
	int getReelSymbolCount(size_t reel, int sym,
	                       symbolMatchTypes matchType = symbolMatchTypes::exact) const;

	// count number of sym in the grid
	// optional parameter matchType
	// returns total count and fills reelSymbolCountVect with individual reel counts
	int getSymbolCount(int sym, symbolMatchTypes matchType = symbolMatchTypes::exact) const;

	// returns row of passed symbol on passed reel
	// returns -1 if symbol not found
	std::optional<size_t> getReelSymbolRow(size_t reel, int sym,
	                                       symbolMatchTypes matchType = symbolMatchTypes::exact)
	const;

	/************************************/
	// Grid processing - expand
	/************************************/

	// Look for symbol on reel. If found then expand symbol to fill reel
	// If optional newSym then expand and replace with newSym
	// return number of symbol on reel before expanded
	int expandReelSymbol(size_t reel, int symbol, int newSym = -1);

	// Look for symbol on all reels. If found then expand symbol to fill reel
	// Return number of reels expanded
	int expandSymbol(int symbol);

	// Look for symbol on each reel in reelCheckVect parameter can be passed as{ 0, 2, 4 } or as an
	// If found then expand symbol to fill reel
	// Return number of reels expanded
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int expandSymbol(int symbol, std::vector<int> reelCheckVect);

	// Look for symbol on each reel.
	// If found then expand symbol to fill reel with newSym
	// Return number of reels expanded
	int expandSymbolToNew(int symbol, int newSym);

	// Look for symbol on each reel in reelCheckVect.
	// If found then expand symbol to fill reel with newSym
	// Return number of reels expanded
	int expandSymbolToNew(int symbol, int newSym, std::vector<int> reelCheckVect);

	/************************************/
	// Grid processing - replace
	/************************************/

	// On passed reel replace all occurances of symOld with symNew
	// return number of replaced symbols
	int replaceReelSymbol(int reel, int symOld, int symNew);

	// replace all grid occurances of symOld with symNew
	// return number reels where symbol was replaced
	int replaceSymbol(int symOld, int symNew);

	// For each reel in reelCheckVect, replace all occurances of symOld with symNew.
	// return number reels where symbol was replaced
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int replaceSymbol(int symOld, int symNew, std::vector<int> reelCheckVect);

	// replace count random symbols in the grid with newSymbol (not already newSymbol)
	std::vector<int> replaceRandomSymbols(int newSymbol, int count,
	                                      std::vector<int> blockedIndexVect = {});

	/************************************/
	// Grid processing - replace not
	/************************************/

	// On passed reel replace all symbols that are NOT symOld with symNew
	// return number of replaced symbols
	int replaceReelNotSymbol(int reel, int symOld, int symNew);

	// In the whole grid replace all symbols that are NOT symOld with symNew
	// return number reels where symbol was replaced
	int replaceNotSymbol(int symOld, int symNew);

	/************************************/
	// Display
	/************************************/

	inline void setConsole(Console* console)
	{
		lpConsole = console;
	}

	inline void setTextMap(const std::map<int, std::string>& textMap)
	{
		mTextMap = textMap;
	}

	inline void setColourMap(const std::map<int, int>& colourMap)
	{
		mColourMap = colourMap;
	}

	void showReels(int tabCount) const;

	void showReels(int tabCount, std::vector<int>& highlightVect, int colour) const;

	std::vector<std::string> getGridTextVect(void);
};

