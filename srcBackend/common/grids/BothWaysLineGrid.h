#pragma once
/**
 * @file BothWaysLineGrid.h
 * @brief Line grid that evaluates wins in both directions (L->R and R->L).
 */

#include "../headers/pch.h"
#include "LineGrid.h"
#include "../wins/Win.h"


/**
 * @brief Extends `LineGrid` to evaluate lines in both left-to-right and right-to-left directions.
 */
class BothWaysLineGrid :
	public LineGrid
{
public:
	uint32_t lrMask = UINT32_MAX;
	uint32_t rlMask = UINT32_MAX;

	/**
	 * @brief Construct a bidirectional line grid with dimensions and payTable.
	 */
	BothWaysLineGrid(const int reels, const int rows, const PayTable& payTable) : LineGrid(
		reels, rows, payTable)
	{
	}

	/**
	 * @brief Construct a bidirectional line grid with dimensions, payTable and payLines.
	 */
	BothWaysLineGrid(const int reels, const int rows, const PayTable& payTable, const PayLines& payLines) :
		LineGrid(
			reels, rows, payTable, payLines)
	{
	}

	/**
	 * @brief Construct a bidirectional line grid with GameMaths.
	 */
	explicit BothWaysLineGrid(const GameMaths& maths) : LineGrid(maths)
	{
	}

	/**
	 * @brief Compute quick-win masks for both directions and return their union.
	 * @return Bitmask of symbols that can still form wins in either direction.
	 */
	uint32_t quickWinCheck();

	/**
	 * @brief Evaluate both L->R and R->L wins and optionally de-duplicate max-of-a-kind.
	 * @param betMultiplier Wager multiplier.
	 * @param payMaxOakTwice If false, remove duplicate 5OAK lines across directions.
	 * @return Total win across both directions (after any de-duplication).
	 */
	int evaluateWins(int betMultiplier, bool payMaxOakTwice = true);

	int evaluateScatterWins(int betMultiplier);


	/**
	 * @brief Calculate wins, scanning from left to right.
	 * @param betMultiplier Bet multiplier (multiple of minBet) used to calculate wins.
	 * @return Total win from L->R lines.
	 */
	int lrWinCheck(int betMultiplier);


	/**
	 * @brief Calculate wins scanning from right to left over paylines.
	 * @param betMultiplier Bet multiplier (multiple of minBet) used to calculate wins.
	 * @return Total win from R->L lines.
	 */
	int rlWinCheck(int betMultiplier);


	/**
	 * @brief Remove duplicated max-of-a-kind wins that appear in both directions.
	 * @return Sum of removed duplicate win amounts.
	 */
	int removeMaxOakDuplicates()
	{
		int lostWin = 0;

		std::vector<std::unique_ptr<Win>>::iterator it1 = winVector.begin();
		do
		{
			if ((*it1)->oak == this->maxReels)
			{
				bool done = false;
				std::vector<std::unique_ptr<Win>>::iterator it2 = it1 + 1;
				do
				{
					if ((*it1)->winType != "line") continue;
					if ((*it2)->winType != "line") continue;
					auto& lineWin1 = dynamic_cast<LineWin&>(*(*it1));
					auto& lineWin2 = dynamic_cast<LineWin&>(*(*it2));
					if (lineWin1.payLine == lineWin2.payLine)
					{
						lostWin += (*it2)->totalWin;
						it2 = winVector.erase(it2);
						done = true;
					}
					else
						++it2;
				}
				while (!done && it2 != winVector.end());
			}
		}
		while (++it1 != winVector.end());
		return lostWin;
	}
};

