#pragma once

// cpp-dev-library
#include "../headers/pch.h"
#include "../mathElements/PayTable.h"
#include "../console/Console.h"
#include "Grid.h"

// game globals
extern Console gameConsole;

/************************************/
// Symbols  
/************************************/
// DEL TEL - move into a new class SymbolSet
typedef struct {
	int index;
	bool isWild;
	int wildIndex;
	bool substWild;

	bool isScatter;
	int scatterIndex;

	bool isOverlay;
	int overlayIndex;

	uint32 mask;
	uint32 payMask;
} symbolData;

enum class WinDirections {
	none, left_right, right_left, both
};

enum class MultipliersCombineType {
	none, add, multiply
};

class PayGrid : public Grid {
protected:
	const PayTable *lpPayTable = nullptr;

public:
	std::vector<std::vector<bool> > isWinVect; // [reel][row]
	std::vector<int> winIndexVect; // undefined size, populate with index of winning positions
	int gridMultiplier = 1;

	PayGrid(const int reels, const int rows) : Grid(reels, rows) {
		isWinVect.resize(maxReels);
	}

	void reset() {
		Grid::reset();
		for (int reel = 0; reel < maxReels; reel++)
			isWinVect[reel].assign(maxRows, false);
		winIndexVect.clear();
	}

	// Pass the required PayTable
	void setPayTable(const PayTable &payTable) { lpPayTable = &payTable; }

	/************************************/
	// Win Analysis
	/************************************/


	uint32_t quickWinCheck() const {
		uint32_t quickWinMask = UINT32_MAX;

		quickWinMask = 0xFFFFFFFF;
		int reelsToCheck = 0;

		if (lpPayTable)
			reelsToCheck = lpPayTable->minWinSymbols;
		else
			reportError("PayGrid:quickWinCheck", "No pay table set");

		int symId = -1;
		for (int reel = 0; quickWinMask && reel < reelsToCheck; ++reel) {
			uint32_t reelMask = 0;
			for (int row = 0; row < reelHeightVect[reel]; ++row) {
				if ((symId = getReelSymbol(reel, row)) >= 1)
					reelMask |= symbolVect[symId].payMask;
			}
			quickWinMask &= reelMask;
		}
		return quickWinMask;
	}

	/************************************/
	// Symbols
	/************************************/
	// DEL TEL - move below into a new class SymbolSet

	// Data set in game
	std::vector<int> paySymVect; // symbols that have a payout
	std::vector<int> wildSymVect; // symbols that are wild
	std::vector<int> wildNoSubstVect; // symbols that wilds do not match
	std::vector<int> scatterSymVect; // symbols that are scatters
	std::vector<int> overlaySymVect; // symbols that are overlays
	int overlayOffset = 0; // subtract this value to get symbol id for wins

	// Data populated by method "symbolsInit() "
	std::vector<symbolData> symbolVect;
	uint32 wildMask = 0;

	// create a unique mask for each pay symbol
	// use by quickWinCheck()
	void symbolsInit(int symbolCount) {
		wildMask = 0;
		symbolVect.resize(symbolCount);

		for (int i = 0; i < symbolCount; i++) {
			symbolVect[i].index = i;
			symbolVect[i].substWild = true; // assume wilds substitutes for all
			symbolVect[i].mask = (1 << i); // create a mask for all
			symbolVect[i].payMask = 0; // assume not a pay symbol
			symbolVect[i].isWild = false; // assume not wild
			symbolVect[i].wildIndex = -1;
			symbolVect[i].isScatter = false; // assume not scatter
			symbolVect[i].scatterIndex = -1;
			symbolVect[i].isOverlay = false; // assume not overlay
			symbolVect[i].overlayIndex = -1;
		}

		// update for symbols that cannot use Wild
		for (int symId: wildNoSubstVect)
			symbolVect[symId].substWild = false;

		// update for overlay symbols
		int i = 0;
		for (int symId: overlaySymVect) {
			symbolVect[symId].isOverlay = true;
			symbolVect[symId].overlayIndex = i++;
		}

		// update for pay symbols and fill the wildMask
		for (int symId: paySymVect) {
			symbolVect[symId].payMask = symbolVect[symId].mask;
			if (symbolVect[symId].substWild)
				wildMask |= symbolVect[symId].mask;
		}

		// apply the wildMask to all wild symbols
		i = 0;
		for (int symId: wildSymVect) {
			symbolVect[symId].isWild = true;
			symbolVect[symId].payMask = wildMask;
			symbolVect[symId].wildIndex = i++;
		}

		// update for scatter symbols
		i = 0;
		for (int symId: scatterSymVect) {
			symbolVect[symId].isScatter = true;
			symbolVect[symId].scatterIndex = i++;
			//scatterWinVect.push_back(new ScatterWin(symId, maxReels));
		}
	}

	inline void setPaySymbols(const std::vector<int> &payVect) {
		paySymVect = payVect;
	}

	inline void setScatterSymbols(const std::vector<int> &scatterVect) {
		scatterSymVect = scatterVect;
	}

	inline void setWildSymbols(const std::vector<int> &wildVect) {
		wildSymVect = wildVect;
	}

	// DEL TEL - move above into the new class SymbolSet
};

