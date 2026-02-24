#pragma once
/**
 * @file PayGrid.h
 * @brief Grid with pay evaluation context (pay table, quick-win checks, multipliers).
 */

#include "../gameMaths/mathElements/PayTable.h"
#include "../console/Console.h"
#include "Grid.h"
#include "../wins/Win.h"
#include <memory>


/**
 * @brief Grid that stores payTable pointer and performs win analysis with efficient quick win checks.
 */
class PayGrid : public Grid
{
protected:
    const PayTable& payTable;

public:
	uint32_t quickWinMask = UINT32_MAX;
	std::vector<std::vector<bool>> isWinVect; // [reel][row]
	std::vector<int> winIndexVect; // undefined size, populate with index of winning positions
	std::vector<std::unique_ptr<Win>> winVector;
	int gridMultiplier = 1;

 /**
  * @brief Construct a grid and store reference to payTable.
  * @param reels Number of reels.
  * @param rows Number of rows.
  * @param payTable Pay table to use with this grid.
  */
 PayGrid(const size_t reels, const size_t rows, const PayTable& payTable);

	/**
	 * @brief Construct from maths config (uses "default" pay table).
	 * @param maths GameMaths object.
	 */
	explicit PayGrid(const GameMaths& maths);

	PayGrid() = delete;

	/**
	 * @brief Reset symbols in parent Grid and
	 */
	void reset();

	static unsigned int combineMultipliers(unsigned int previousMultiplier, unsigned int newMultiplier,
	                                       MultipliersCombineType combineType =
		                                       MultipliersCombineType::multiply);


	/************************************/
	// Win Analysis
	/************************************/


	/**
	 * @brief Quick pre-check: compute symbol pay-mask across first N reels.
	 * @return Bitmask of symbols that can still form wins given current grid state.
	 */
	uint32_t quickWinCheck();
};
