#pragma once

// cpp-dev-library
#include "../mathElements/PayTable.h"
#include "../console/Console.h"
#include "Grid.h"

// game globals
extern Console gameConsole;


enum class WinDirections
{
	none, left_right, right_left, both
};

enum class MultipliersCombineType
{
	none, add, multiply
};

class PayGrid : public Grid
{
protected:
	const PayTable* lpPayTable;

public:
	uint32_t quickWinMask = UINT32_MAX;
	std::vector<std::vector<bool>> isWinVect; // [reel][row]
	std::vector<int> winIndexVect; // undefined size, populate with index of winning positions
	int gridMultiplier = 1;

	PayGrid(const size_t reels, const size_t rows, const PayTable& payTable)
		: Grid(reels, rows),
		  lpPayTable(&payTable)
	{
		isWinVect.resize(maxReels);
	}

	void reset()
	{
		Grid::reset();
		for (int reel = 0; reel < maxReels; reel++)
			isWinVect[reel].assign(maxRows, false);
		winIndexVect.clear();
	}


	/************************************/
	// Win Analysis
	/************************************/


	uint32_t quickWinCheck()
	{
		quickWinMask = 0xFFFFFFFF;
		int reelsToCheck = 0;

		reelsToCheck = lpPayTable->minWinSymbols;

		for (int reel = 0; quickWinMask && reel < reelsToCheck; ++reel)
		{
			uint32_t reelMask = 0;
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				const int symId = getReelSymbol(reel, row);
				reelMask |= lpPayTable->symbolVect[symId].payMask;
			}
			quickWinMask &= reelMask;
		}
		return quickWinMask;
	}
};

