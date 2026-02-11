#pragma once

// cpp-dev-library
#include "../headers/pch.h"
#include "../mathElements/PayTable.h"
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

	// details of wins in the current grid

	WaysGrid(const size_t reels, const size_t rows, const PayTable& payTable) : PayGrid(reels, rows, payTable)
	{
		maxWays = static_cast<int>(pow(rows, reels));
	}

	~WaysGrid()
	{
		winVectorReset();
	}

	// implements Grid::reset(); and WaysGrid::winVectorReset()
	void reset(void)
	{
		Grid::reset();
		winVectorReset();
	}

	/************************************/
	// Win methods
	/************************************/

	// deletes all entries from the winVector
	void winVectorReset(void)
	{
		winVector.clear();
	}

	int evaluateWins(PayTable* lpPayTable, const int betMultiplier)
	{
		std::map<Symbol, WaysWin*> winMap;
		winVectorReset();

		if (!quickWinCheck())
			return 0;

		std::map<Symbol, WaysWin*>::iterator it;

		// check for symbols on 1st reel and in quickWinMask, includes wilds
		int reel = 0;
		Symbol symId;
		for (int row = 0; row < reelHeightVect[reel]; row++)
		{
			// build list of symbols to check for matches
			if ((symId = getReelSymbol(reel, row)) == SYMBOL_NONE)
				break;
			const SymbolData& symData = lpPayTable->symbolVect[symId];
			if (symData.isWild)
			{
				// wild on reel 1 - create a win for evey paying symbol in quickWinMask
				for (auto sym : lpPayTable->paySymVect)
				{
					if (lpPayTable->symbolVect[sym].payMask & quickWinMask)
					{
						if ((it = winMap.find(sym)) == winMap.end())
						{
							// new symbol
							winMap[sym] = new WaysWin(sym, maxReels, maxRows);
							winMap[sym]->setWildWin(lpPayTable->symbolVect[sym].isWild);
							winMap[sym]->addPosition(reel, row, true);
						}
						else
							it->second->addPosition(reel, row, true);
					}
				}
			}
			else if (symData.payMask && (symData.payMask & quickWinMask) == symData.payMask)
			{
				if ((it = winMap.find(symId)) == winMap.end())
				{
					winMap[symId] = new WaysWin(symId, maxReels, maxRows);
					winMap[symId]->addPosition(reel, row, false);
				}
				else
					it->second->addPosition(reel, row, false);
			}
		}

		// Check the other reels for winning symbols
		bool blnWinning = true;
		for (reel = 1; blnWinning && reel < maxReels; reel++)
		{
			blnWinning = false;
			for (int row = 0; row < reelHeightVect[reel]; row++)
			{
				// look for winning symbols on this reel
				if ((symId = getReelSymbol(reel, row)) == SYMBOL_NONE)
					break;
				SymbolData& symData = lpPayTable->symbolVect[symId];

				if (symData.isWild)
				{
					for (it = winMap.begin(); it != winMap.end(); ++it)
					{
						//  for all winning symbols - remember wild position
						if (lpPayTable->isWild(it->first) || lpPayTable->wildSubstitutes(it->first))
							it->second->addPosition(reel, row, true);
					}
					blnWinning = true;
				}
				else if (symData.payMask && (symData.payMask & quickWinMask) == symData.payMask)
				{
					// found a win symbol so add
					if ((it = winMap.find(symId)) != winMap.end())
					{
						// found a start symbol so add as a win position
						it->second->addPosition(reel, row, false);
						blnWinning = true;
					}
				}
			}
		}

		// set win values
		int spinWin = 0;
		for (it = winMap.begin(); it != winMap.end(); ++it)
		{
			int winVal = it->second->setWin(lpPayTable, betMultiplier);
			if (winVal)
			{
				spinWin += winVal;
				winVector.emplace_back(std::make_unique<WaysWin>(*it->second));
			}
			else
			{
				// free memory that's no longer needed
				delete it->second;
			}
		}
		return spinWin;
	}

	// Checks if there is a win for passed "symbol"
	// If so applies multipler to the win and returns the incremental win: (initialWin * multiplier) - initialWin
	int setWinMultiplier(const Symbol symbol, const int multiplier)
	{
		int newWin = 0;
		int initialWin = 0;
		for (auto& win : winVector)
		{
			if (win->symbol == symbol)
			{
				initialWin = win->totalWin;
				win->totalWin *= multiplier;
				newWin = win->totalWin - initialWin;
				return newWin;
			}
		}
		return newWin;
	}

	// show the ways wins, 1 per line.
	// DEL TEL - should be deprecate. Use getWinTextVect() to get the strings and display them in calling method
	int showWins(std::map<Symbol, std::string>& symbolMap, int startRow, int col)
	{
		int row = startRow;
		for (auto& win : winVector)
		{
			gameConsole.setCursorPos(row++, col);
			std::cout << win->getWinText(symbolMap);
		}
		return row - startRow;
	}

	// generates the vector using passed symbolMap
	std::vector<std::string> getWinTextVect(std::map<Symbol, std::string>& symbolMap)
	{
		std::vector<std::string> winTextVect;
		for (auto& win : winVector)
			winTextVect.push_back(win->getWinText(symbolMap));
		return winTextVect;
	}

	// generates the vector using preset, Grid.mTextMap
	std::vector<std::string> getWinTextVect(void)
	{
		std::vector<std::string> winTextVect;
		for (auto& win : winVector)
			winTextVect.push_back(win->getWinText(mTextMap));
		return winTextVect;
	}

	//	generates the vector of winPosVect for each active win
	//	winPosVect[reel][row]: 0= no win, 1= winning symbol, 2= winning wild
	std::vector<std::vector<std::vector<int>>> getWinPosVect(void)
	{
		std::vector<std::vector<std::vector<int>>> winPosVect;
		for (auto& win : winVector)
		{
			if (win->winType != "ways") continue;
			const auto& waysWin = dynamic_cast<const WaysWin&>(*win);
			winPosVect.push_back(waysWin.winPosVect);
		}
		return winPosVect;
	}
};

