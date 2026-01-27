#pragma once
/**
 * @file BothWaysLineGrid.h
 * @brief Line grid that evaluates wins in both directions (L->R and R->L).
 */

#include "../headers/pch.h"
#include "LineGrid.h"
#include "Win.h"


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


	/**
	 * @brief Calculate wins, scanning from left to right.
	 * @param betMultiplier Bet multiplier (multiple of minBet) used to calculate wins.
	 * @return Total win from L->R lines.
	 */
	int lrWinCheck(const int betMultiplier)
	{
		int totalWin = 0;
		int lineMultiplier;
		unsigned int consecutiveWilds, wildCount = 0;
		Symbol symbolToMatch;
		const std::vector<SymbolData>& symbolVect = lpPayTable->symbolVect;

		for (int line = 0; line < lpPayLines->lineCount; ++line)
		{
			constexpr int startReel = 0;
			bool blnCheckLine;
			size_t row = lpPayLines->getRow(line, startReel);
			const Symbol startSymbol = symbolToMatch = reelSymbolVect[startReel][row];
			const SymbolData& startSymData = symbolVect[startSymbol];

			if (startSymData.isScatter && !startSymData.isWild)
				blnCheckLine = false;
			else
				blnCheckLine = startSymData.isWild || (lrMask & startSymData.mask);

			if (!blnCheckLine)
				continue; // based on 1st symbol there is no need to check this line


			unsigned int number = consecutiveWilds = wildCount = 0;
			int wildMultiplier = lineMultiplier = 1;
			for (int reel = startReel; reel < maxReels; reel++)
			{
				// start from beginning again to account for multipliers
				row = lpPayLines->getRow(line, reel);
				const Symbol nextSym = reelSymbolVect[reel][row];
				const SymbolData& nextSymData = symbolVect[nextSym];

				if (nextSymData.isScatter && !nextSymData.isWild)
					break;
				if (symbolVect[symbolToMatch].isWild || nextSymData.isWild || nextSym == symbolToMatch)
				{
					number++;
					if (multiplierVect[reel][row] > 1)
						lineMultiplier *= multiplierVect[reel][row];
					if (symbolVect[symbolToMatch].isWild)
					{
						if (nextSymData.isWild)
						{
							consecutiveWilds++;
							if (multiplierVect[reel][row] > 1)
								wildMultiplier *= multiplierVect[reel][row];
						}
						else
							symbolToMatch = nextSym;
					}
					if (nextSymData.isWild)
						wildCount++;
				}
				else
					break;
			}

			if (number >= lpPayTable->minPaySymbolsVect[symbolToMatch])
			{
				if (consecutiveWilds >= lpPayTable->minWinSymbols)
				{
					// Wilds make a payout. If wins are equal pay more symbols in preference to fewer wilds
					if ((wildMultiplier * lpPayTable->getWin(startSymbol, consecutiveWilds)) > (
						lineMultiplier * lpPayTable->getWin(symbolToMatch, number)))
					{
						//  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
						symbolToMatch = startSymbol;
						number = consecutiveWilds;
						lineMultiplier = wildMultiplier;
					}
				}

				if (const int win = betMultiplier * lpPayTable->getWin(symbolToMatch, number))
				{
					totalWin += (win * lineMultiplier);
					Win lineWin;
					lineWin.payLineNum = line;
					lineWin.payLine = getPayLine(line);
					lineWin.win = lineWin.totalWin = win;
					if (lineMultiplier > 1)
					{
						lineWin.multiplier = lineMultiplier;
						lineWin.totalWin *= lineMultiplier;
					}
					lineWin.oak = number;
					lineWin.wildCount = wildCount;
					lineWin.symbol = symbolToMatch;
					lineWin.direction = WinDirections::left_right;
					winVector.push_back(lineWin);
				}
			}
		}
		return totalWin;
	}

	/**
	 * @brief Calculate wins scanning from right to left over paylines.
	 * @param betMultiplier Bet multiplier (multiple of minBet) used to calculate wins.
	 * @return Total win from R->L lines.
	 */
	int rlWinCheck(const int betMultiplier)
	{
		int totalWin = 0;
		int lineMultiplier;
		unsigned int consecutiveWilds, wildCount = 0;
		Symbol symbolToMatch;
		const size_t startReel = maxReels - 1;
		const std::vector<SymbolData>& symbolVect = lpPayTable->symbolVect;

		for (int line = 0; line < lpPayLines->lineCount; ++line)
		{
			bool blnCheckLine;
			size_t row = lpPayLines->getRow(line, startReel);
			const Symbol startSymbol = symbolToMatch = reelSymbolVect[startReel][row];
			const SymbolData& startSymData = symbolVect[startSymbol];

			if (startSymData.isScatter && !startSymData.isWild)
				blnCheckLine = false;
			else
				blnCheckLine = startSymData.isWild || (rlMask & startSymData.mask);

			if (!blnCheckLine)
				continue; // based on 1st symbol there is no need to check this line


			unsigned int number = consecutiveWilds = wildCount = 0;
			int wildMultiplier = lineMultiplier = 1;
			for (size_t reel = startReel; reel <= startReel; reel--)
			{
				// start from beginning again to account for multipliers
				row = lpPayLines->getRow(line, reel);
				const Symbol nextSym = reelSymbolVect[reel][row];
				const SymbolData& nextSymData = symbolVect[nextSym];

				if (nextSymData.isScatter && !nextSymData.isWild)
					break;
				if (symbolVect[symbolToMatch].isWild || nextSymData.isWild || nextSym == symbolToMatch)
				{
					number++;
					if (multiplierVect[reel][row] > 1)
						lineMultiplier *= multiplierVect[reel][row];
					if (symbolVect[symbolToMatch].isWild)
					{
						if (nextSymData.isWild)
						{
							consecutiveWilds++;
							if (multiplierVect[reel][row] > 1)
								wildMultiplier *= multiplierVect[reel][row];
						}
						else
							symbolToMatch = nextSym;
					}
					if (nextSymData.isWild)
						wildCount++;
				}
				else
					break;
			}

			if (number >= lpPayTable->minPaySymbolsVect[symbolToMatch])
			{
				if (consecutiveWilds >= lpPayTable->minWinSymbols)
				{
					// Wilds make a payout. If wins are equal pay more symbols in preference to fewer wilds
					if ((wildMultiplier * lpPayTable->getWin(startSymbol, consecutiveWilds)) > (
						lineMultiplier * lpPayTable->getWin(symbolToMatch, number)))
					{
						//  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
						symbolToMatch = startSymbol;
						number = consecutiveWilds;
						lineMultiplier = wildMultiplier;
					}
				}

				if (const int win = betMultiplier * lpPayTable->getWin(symbolToMatch, number))
				{
					totalWin += (win * lineMultiplier);
					Win lineWin;
					lineWin.payLine = &lpPayLines->lineVect[line];
					lineWin.win = lineWin.totalWin = win;
					if (lineMultiplier > 1)
					{
						lineWin.multiplier = lineMultiplier;
						lineWin.totalWin *= lineMultiplier;
					}
					lineWin.oak = number;
					lineWin.wildCount = wildCount;
					lineWin.symbol = symbolToMatch;
					lineWin.winType = "line";
					lineWin.direction = WinDirections::right_left;
					winVector.push_back(lineWin);
				}
			}
		}
		return totalWin;
	}


	/**
	 * @brief Remove duplicated max-of-a-kind wins that appear in both directions.
	 * @return Sum of removed duplicate win amounts.
	 */
	int removeMaxOakDuplicates()
	{
		int lostWin = 0;

		std::vector<Win>::iterator it1 = winVector.begin();
		do
		{
			if (it1->oak == this->maxReels)
			{
				bool done = false;
				std::vector<Win>::iterator it2 = it1 + 1;
				do
				{
					if (it1->payLine == it2->payLine)
					{
						lostWin += it2->totalWin;
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

