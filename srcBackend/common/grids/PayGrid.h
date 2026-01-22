#pragma once
/**
 * @file PayGrid.h
 * @brief Grid with pay evaluation context (pay table, quick-win checks, multipliers).
 */

#include "../gameMaths/mathElements/PayTable.h"
#include "../console/Console.h"
#include "Grid.h"

extern Console gameConsole;


enum class WinDirections
{
	none, left_right, right_left, both
};

enum class MultipliersCombineType
{
	none, add, multiply
};

/**
 * @brief Grid that stores paytable pointer and performs win analysis with efficient quick win checks.
 */
class PayGrid : public Grid
{
protected:
	const PayTable* lpPayTable;

public:
	uint32_t quickWinMask = UINT32_MAX;
	std::vector<std::vector<bool>> isWinVect; // [reel][row]
	std::vector<int> winIndexVect; // undefined size, populate with index of winning positions
	int gridMultiplier = 1;

	/**
	 * @brief Construct a grid and store pointer to paytable.
	 * @param reels Number of reels.
	 * @param rows Number of rows.
	 * @param payTable Pay table to use with this grid.
	 */
	PayGrid(const size_t reels, const size_t rows, const PayTable& payTable)
		: Grid(reels, rows),
		  lpPayTable(&payTable)
	{
		isWinVect.resize(maxReels);
	}

	/**
	 * @brief Construct from maths config (uses "default" pay table).
	 * @param maths GameMaths object.
	 */
	explicit PayGrid(const GameMaths& maths)
		: PayGrid(maths.ints["numReels"], maths.ints["numRows"], maths.payTables["default"])
	{
	}

	/**
	 * @brief Reset symbols in parent Grid and
	 */
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


	/**
	 * @brief Quick pre-check: compute symbol pay-mask across first N reels.
	 * @return Bitmask of symbols that can still form wins given current grid state.
	 */
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
				const Symbol symId = getReelSymbol(reel, row);
				reelMask |= lpPayTable->symbolVect[symId].payMask;
			}
			quickWinMask &= reelMask;
		}
		return quickWinMask;
	}
};

