#include "../headers/pch.h"
#include "Grid.h"
#include "../rng/MathsRng.h"
#include "../console/Console.h"
#include <cstring>
#include <algorithm>
#ifndef _WIN32
	#include <cassert>
#endif


#ifdef _WIN32
#define MEMCPY_SAFE(dest, destsz, src, count) memcpy_s(dest, destsz, src, count)
#else
#define MEMCPY_SAFE(dest, destsz, src, count)                                  \
	do {                                                                       \
		size_t _d = static_cast<size_t>(destsz);                               \
		size_t _c = static_cast<size_t>(count);                                \
		assert(_c <= _d && "MEMCPY_SAFE: count > dest size");                  \
		std::memcpy((dest), (src), _c);                                        \
	} while (0)

#endif

/**************************************************/
// constructor
/**************************************************/
namespace MathLibrary
{
	Grid::Grid(int reels, int rows)
	{
		maxReels = reels;
		maxRows = rows;
		maxCells = maxReels * maxRows;
		reelDataBytes = sizeof(int) * maxRows;

		reelSymbolVect.resize(maxReels);
		multiplierVect.resize(maxReels);
		for (int reel = 0; reel < maxReels; reel++)
		{
			reelSymbolVect[reel].resize(maxRows, -1);
			multiplierVect[reel].resize(maxRows, -1);
		}

		reelHeightVect.resize(maxReels);
		setReelHeights();

		// prep analysis 
	}

	// DEL TEL - consider a constructor for irregular sized grid e.g. [3,4,5,4,3]

	/**************************************************/
	// Initialise
	/**************************************************/

	// set all reel heights to maxRows
	void Grid::setReelHeights(void)
	{
		for (int i = 0; i < maxReels; i++)
			reelHeightVect[i] = maxRows;
	}

	// set reel heights to values in the passed vetor 
	void Grid::setReelHeights(std::vector <int> heightVect)
	{
		if (heightVect.size() == reelHeightVect.size())
			reelHeightVect = heightVect;
	}



	// if Grid dimensions match then copies 'reelSymbolVect'
	void Grid::copy(Grid & sourceGrid)
	{
		if (sourceGrid.maxReels != maxReels)
		{
			reportError("Grid::copy", "SourceGrid.maxReels != DestGrid.maxReels");
			return;
		}
		if (sourceGrid.reelDataBytes != reelDataBytes)
			{
				reportError("Grid::copy", "SourceGrid.reelDataBytes != DestGrid.reelDataBytes");
				return;
		}
		for (int reel = 0; reel < maxReels; reel++)
		{
			MEMCPY_SAFE(reelSymbolVect[reel].data(), reelDataBytes, sourceGrid.reelSymbolVect[reel].data(), reelDataBytes);
			//MEMCPY_SAFE(multipliersInView[reel].data(), reelDataBytes, sourceGrid.multipliersInView[reel].data(), reelDataBytes);
		}

		//mReelLockCount = sourceGrid.getReelLockCount();
		//mReelLockMask = sourceGrid.mReelLockMask;

		//if (inDebug)
		//{
		//	for (int reel = 0; reel < maxReels; reel++)
		//	{
		//		for (int row = 0; row < maxRows; ++row)
		//			debugRowVect[row][reel] = symbolsInView[reel][row];
		//	}
		//}
	}

	// if Grid dimensions match then copies all occurances of 'symbol' to 'reelSymbolVect'
	void Grid::copySymbol(Grid& sourceGrid, int symbol)
	{
		if (sourceGrid.maxReels != maxReels)
		{
			reportError("Grid::copySymbol", "SourceGrid.maxReels != DestGrid.maxReels");
			return;
		}
		if (sourceGrid.reelDataBytes != reelDataBytes)
		{
			reportError("Grid::copySymbol", "SourceGrid.reelDataBytes != DestGrid.reelDataBytes");
			return;
		}
		for (int reel = 0; reel < maxReels; reel++)
		{
			for (int row = 0; row < maxRows; ++row)
			{
				if (sourceGrid.reelSymbolVect[reel][row] == symbol)
					reelSymbolVect[reel][row] = symbol;
			}
		}
	}


	/**************************************************/
	// Apply reels
	/**************************************************/

	// Copy symbols from reels to Grid. 
	// Does not copy symbols from locked reels unless getLockedReels is passed and true
	void Grid::setSymbolsFromReels(ReelSet* lpReelSet, bool getLockedReels)
	{
		if (getLockedReels)
		{				// copy data from all reels
			for (int reel = 0; reel < maxReels; reel++)
				MEMCPY_SAFE(reelSymbolVect[reel].data(), reelDataBytes, lpReelSet->getReelStopPointer(reel), reelDataBytes);
		}
		else
		{				// copy data from each reel that did a spin
			for (int reel = 0; reel < maxReels; reel++)
			{	
				if (lpReelSet->didSpinVect[reel])
					MEMCPY_SAFE(reelSymbolVect[reel].data(), reelDataBytes, lpReelSet->getReelStopPointer(reel), reelDataBytes);
			}
		}
	}

	/**************************************************/
	// Set Symbols
	/**************************************************/

	// replace current reel symbols with 'count' of 'newSymbol' from top to bottom 
	void Grid::setReelSymbols(int reel, int newSymbol, int count)
	{
		for (int row = 0; row < count; row++)
			reelSymbolVect[reel][row] = newSymbol;
	}

	// fill 'reel' with 'newSymbol'
	void Grid::setReelSymbols(int reel, int newSymbol)
	{
		for (int row = 0; row < reelHeightVect[reel]; row++)
			reelSymbolVect[reel][row] = newSymbol;
	}

	// replace current reel symbols with 'newSymbol' for each element in rowVect
	// rowVect may contain:
	//		1. An entry for each row position, each non-zero entry is set e.g. { 0,0,1}
	//		2. The indexes of rows to set e.g. {2} 
	// returns vector of grid indexes that have been set
	std::vector <int> Grid::setReelSymbols(int reel, int newSymbol, std::vector <int>rowVect)
	{
		std::vector <int> setIndexVect;
		if (rowVect.size() == maxRows)
		{			// Option 1 - An entry for each row position
			for (int row = 0; row < reelHeightVect[reel]; row++)
			{
				if (rowVect[row])
				{
					this->setSymbol (reel,row, newSymbol);
					setIndexVect.push_back(this->getGridIndex(reel, row));
				}
			}
		}
		else
		{			// Option 2 - The indexes of rows to set
			for (int row : rowVect){
				this->setSymbol(reel, row, newSymbol);
				setIndexVect.push_back(this->getGridIndex(reel, row));
			}
		}
		return setIndexVect;
	}
	
	/************************************/
	// Analysis
	/************************************/

	// count number of sym on passed reel
	// optional parameter matchType
	int Grid::getReelSymbolCount(int reel, int sym, symbolMatchTypes matchType) const 
	{
		int count = 0;

		if (matchType == symbolMatchTypes::exact)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] == sym)
					count++;
			}
		}
		else if (matchType == symbolMatchTypes::greaterEqual)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] >= sym)
					count++;
			}
		}
		else if (matchType == symbolMatchTypes::greater)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] > sym)
					count++;
			}
		}
		else if (matchType == symbolMatchTypes::lessEqual)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] <= sym)
					count++;
			}
		}
		else if (matchType == symbolMatchTypes::less)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] < sym)
					count++;
			}
		}
		return count;
	}

	int Grid::getSymbolCount(int sym, symbolMatchTypes matchType) const
	{
		int total = 0;
		for (int reel = 0; reel < maxReels; ++reel)
		{
			int count = getReelSymbolCount(reel, sym, matchType);
			total += count;
		}
		return total;
	}

	// returns first row of passed symbol on passed reel
	// returns -1 if symbol not found
	int Grid::getReelSymbolRow(int reel, int sym, symbolMatchTypes matchType) const
	{
		if (matchType == symbolMatchTypes::exact)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] == sym)
					return row;
			}
		}
		else if (matchType == symbolMatchTypes::greaterEqual)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] >= sym)
					return row;
			}
		}
		else if (matchType == symbolMatchTypes::greater)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] > sym)
					return row;
			}
		}
		else if (matchType == symbolMatchTypes::lessEqual)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] <= sym)
					return row;
			}
		}
		else if (matchType == symbolMatchTypes::less)
		{
			for (int row = 0; row < reelHeightVect[reel]; ++row)
			{
				if (reelSymbolVect[reel][row] < sym)
					return row;
			}
		}
		return -1;		// symbol not found
	}


	/************************************/
	// Grid processing - expand
	/************************************/

	// Look for symbol on reel. If found then expand symbol to fill reel
	// If optional parameter newSym is passed then expand and replace with newSym
	// return number of symbol on reel before expanded 
	int Grid::expandReelSymbol(int reel, int symbol, int newSym)
	{
		int count = getReelSymbolCount(reel, symbol, symbolMatchTypes::exact);
		if (count)
			setReelSymbols(reel, newSym == -1 ? symbol : newSym);
		return count;
	}

	// Look for symbol on all reels. If found then expand symbol to fill reel
	// Return number of reels expanded
	int Grid::expandSymbol(int symbol)
	{
		int count = 0;
		for (int reel = 0; reel < maxReels; ++reel)
		{
			if (expandReelSymbol(reel, symbol))
				count++;
		}
		return count;
	}

	// Look for symbol on each reel in reelCheckVect parameter can be passed as {0, 2, 4} or as an
	// If found then expand symbol to fill reel
	// Return number of reels expanded 
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int Grid::expandSymbol(int symbol, std::vector<int> reelCheckVect)
	{
		int count = 0;
		for (int reel : reelCheckVect)
		{
			if (expandReelSymbol(reel, symbol))
				count++;
		}
		return count;
	}

	// Look for symbol on each reel
	// If found then expand symbol to fill reel with newSym
	// Return number of reels expanded 
	int Grid::expandSymbolToNew(int symbol, int newSym)
	{
		int count = 0;
		for (int reel = 0; reel < maxReels; reel++)
		{
			if (expandReelSymbol(reel, symbol, newSym))
				count++;
		}
		return count;
	}

	// Look for symbol on each reel in reelCheckVect
	// If found then expand symbol to fill reel with newSym
	// Return number of reels expanded 
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int Grid::expandSymbolToNew(int symbol, int newSym, std::vector<int> reelCheckVect)
	{
		int count = 0;
		for (int reel : reelCheckVect)
		{
			if (expandReelSymbol(reel, symbol, newSym))
				count++;
		}
		return count;
	}

	/************************************/
	// Grid processing - replace
	/************************************/

	// On passed reel replace all occurances of symOld with symNew
	// return number of replaced symbols
	int Grid::replaceReelSymbol(int reel, int symOld, int symNew)
	{
		int count = 0;
		for (int row = 0; row < maxRows; row++)
		{
			if (reelSymbolVect[reel][row] == symOld)
			{
				reelSymbolVect[reel][row] = symNew;
				count++;
			}
		}
		return count;
	}

	// replace all grid occurances of symOld with symNew
	// return number reels where symbol was replaced
	int Grid::replaceSymbol(int symOld, int symNew)
	{
		int count = 0;
		for (int reel = 0; reel < maxReels; reel++)
		{
			if (replaceReelSymbol(reel, symOld, symNew))
				count++;
		}
		return count;
	}

	
	// replace count random symbols in the grid with newSymbol (not already newSymbol)
	// optionally pass list of indexes wich cannot be replaced
	std::vector<int> Grid::replaceRandomSymbols(int newSymbol, int count, std::vector<int> blockedIndexVect)
	{
		std::vector<int> gridPosVect;

		// quick check to see if it can be done
		int symbolCount = getSymbolCount(newSymbol);
		if (maxCells - symbolCount < count)
			return gridPosVect;
	
		// create the list of options to replace
		std::vector<int> gridPosOptions;
		if (blockedIndexVect.size())
		{				// build a list of grid indexes that do not contain newSymbol and are not blocked
			for (int i = 0; i < maxCells; i++)
			{
				if (getSymbol(i) != newSymbol)
				{
					bool allow =		// allow if not in blockedIndexVect
						std::find(blockedIndexVect.begin(), blockedIndexVect.end(), i) == blockedIndexVect.end();
					if (allow)
						gridPosOptions.push_back(i);
				}
			}
		}
		else
		{				// build a list of all grid indexes that do not contain newSymbol
			for (int i = 0; i < maxCells; i++){
				if (getSymbol(i) != newSymbol)
					gridPosOptions.push_back(i);
			}
		}

		// pick and replace symbols at count positions in grid
		for (int i=0; i<count; i++)
		{
			int index = mathRng.getInt((uint32_t)gridPosOptions.size(), "replaceRandomSymbolsIndex");
			int gridPos = gridPosOptions[index];
			gridPosOptions.erase(gridPosOptions.begin() + index);
			gridPosVect.push_back(gridPos);
			setSymbol(gridPos, newSymbol);
		}

		return gridPosVect;
	}


	// For each reel in reelCheckVect, replace all occurances of symOld with symNew. 
	// return number reels where symbol was replaced
	// The reelCheckVect parameter can be passed as { 0, 2, 4 } or as a defined variable like scatterReelsVect
	// e.g. std::vector <int> scatterReelsVect = { 0, 2, 4 };
	int Grid::replaceSymbol(int symOld, int symNew, std::vector<int> reelCheckVect)
	{
		int count = 0;
		for (int reel : reelCheckVect)
		{
			if (replaceReelSymbol(reel, symOld, symNew))
				count++;
		}
		return count;
	}


	/************************************/
	// Grid processing - replace not
	/************************************/

	// On passed reel replace all symbols that are NOT symOld with symNew
	// return number of replaced symbols
	int Grid::replaceReelNotSymbol(int reel, int symOld, int symNew)
	{
		int count = 0;
		for (int row = 0; row < maxRows; row++)
		{
			if (reelSymbolVect[reel][row] != symOld)
			{
				reelSymbolVect[reel][row] = symNew;
				count++;
			}
		}
		return count;
	}

	// In the whole grid replace all symbols that are NOT symOld with symNew
	// return number reels where symbol was replaced
	int Grid::replaceNotSymbol(int symOld, int symNew)
	{
		int count = 0;
		for (int reel = 0; reel < maxReels; reel++)
		{
			if (replaceReelNotSymbol(reel, symOld, symNew))
				count++;
		}
		return count;
	}

	/************************************/
	// Display
	/************************************/

	void Grid::showReels(int tabCount, std::vector<int> &highlightVect, int colour)
	{
		if (!mTextMap.size())
			reportError("Grid::showReels", "Call 'Grid::setTextMap()' before using this method");
		// show reel grid
		int index = 0;
		for (int row = 0; row < maxRows; row++)
		{
			showTabs(tabCount);
			for (int reel = 0; reel < maxReels; reel++, index++)
			{
				std::cout << " | ";
				int symb = reelSymbolVect[reel][row];
				auto it = std::find(highlightVect.begin(), highlightVect.end(), index);

				if(it != highlightVect.end())
					lpConsole->coutStyle(mTextMap[symb], colour);
				else if (mColourMap[symb])
					lpConsole->coutStyle(mTextMap[symb], mColourMap[symb]);
				else
					std::cout << mTextMap[symb];
			}
			std::cout << " | ";
		}
		std::cout << std::endl;
	}

	void Grid::showReels(int tabCount)
	{
		if (!mTextMap.size())
			reportError("Grid::showReels", "Call 'Grid::setTextMap()' before using this method");
		// show reel grid
		for (int row = 0; row < maxRows; row++)
		{
			showTabs(tabCount);
			for (int reel = 0; reel < maxReels; reel++)
			{
				std::cout << " | ";
				int symb = reelSymbolVect[reel][row];
				if (mColourMap[symb])
					lpConsole->coutStyle(mTextMap[symb], mColourMap[symb]);
				else
					std::cout << mTextMap[symb];
			}
			std::cout << " | ";
		}
		std::cout << std::endl;
	}

	std::vector <std::string> Grid::getGridTextVect(void)
	{
		std::vector <std::string> gridTextVector (maxRows, "| ");
		for (int row = 0; row < maxRows; row++)
		{
			for (int reel = 0; reel < maxReels; reel++)
				gridTextVector[row] += (mTextMap[reelSymbolVect[reel][row]] + " | ");
		}
		return gridTextVector;
	}
}
