#include <algorithm>
#include "Grid.h"
#include "../rng/MathsRng.h"
#include "../gameMaths/GameMaths.h"

#ifndef _WIN32
#include <cassert>
#endif

#ifdef _WIN32
#define MEMCPY_SAFE(dest, destSz, src, count) memcpy_s(dest, destSz, src, count)
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
Grid::Grid(const size_t reels, const size_t rows)
	: reelDataBytes(sizeof(Symbol) * rows),
	  multiplierDataBytes(sizeof(unsigned int) * rows),
	  maxReels(reels),
	  maxRows(rows),
	  maxCells(maxReels * maxRows)
{
	reelSymbolVect.resize(maxReels);
	multiplierVect.resize(maxReels);
	clearSpinLock();
	clearSticky();
	for (int reel = 0; reel < maxReels; reel++)
	{
		reelSymbolVect[reel].resize(maxRows, SYMBOL_NONE);
		multiplierVect[reel].resize(maxRows, 1);
	}

	reelHeightVect.resize(maxReels);
	setReelHeights();

	// prep analysis
}

// Set all grid symbols to SYMBOL_NONE
void Grid::reset()
{
	for (int reel = 0; reel < static_cast<int>(maxReels); reel++)
	{
		for (int row = 0; row < static_cast<int>(maxRows); row++)
		{
			reelSymbolVect[reel][row] = SYMBOL_NONE;
		}
	}
	clearSpinLock();
	resetMultipliers();
	cascadedSymbolsPerReel.resize(maxReels, 0);
}

// Set all multipliers to 1
void Grid::resetMultipliers()
{
	for (int reel = 0; reel < static_cast<int>(maxReels); reel++)
	{
		for (int row = 0; row < static_cast<int>(maxRows); row++)
		{
			multiplierVect[reel][row] = 1;
		}
	}
	multipliersSet = false;
}

// Convert linear index to {reel,row}
std::vector<size_t> Grid::getGridPosition(const size_t index) const
{
	size_t row = index / maxReels;
	size_t reel = index - (row * maxReels);
	return {reel, row};
}

// Convert {reel,row} to linear index
size_t Grid::getGridIndex(const size_t reel, const size_t row) const
{
	return (row * maxReels) + reel;
}

Grid::Grid(const GameMaths& maths)
	: Grid(maths.ints["numReels"], maths.ints["numRows"])
{
}

// DEL TEL - consider a constructor for irregular sized grid e.g. [3,4,5,4,3]

/**************************************************/
// Initialise
/**************************************************/

// set all reel heights to maxRows
void Grid::setReelHeights()
{
	for (int i = 0; i < maxReels; i++)
		reelHeightVect[i] = maxRows;
}

// set reel heights to values in passed vector
void Grid::setReelHeights(const std::vector<size_t>& heightVect)
{
	if (heightVect.size() == reelHeightVect.size())
		reelHeightVect = heightVect;
}


// if Grid dimensions match, then copies 'reelSymbolVect'
void Grid::copy(const Grid& sourceGrid)
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
	if (sourceGrid.multiplierDataBytes != multiplierDataBytes)
	{
		reportError("Grid::copy", "SourceGrid.multiplierDataBytes != DestGrid.multiplierDataBytes");
		return;
	}
	for (int reel = 0; reel < maxReels; reel++)
	{
		MEMCPY_SAFE(reelSymbolVect[reel].data(), reelDataBytes,
		            sourceGrid.reelSymbolVect[reel].data(), reelDataBytes);
		MEMCPY_SAFE(multiplierVect[reel].data(), multiplierDataBytes, sourceGrid.multiplierVect[reel].data(),
		            multiplierDataBytes);
	}
}

// if Grid dimensions match, then copies all occurrences of 'symbol' to 'reelSymbolVect'
void Grid::copySymbol(const Grid& sourceGrid, const Symbol symbol)
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
// Apply spin
/**************************************************/

void Grid::applySpin(const SpinResult& spinResult, const bool getLockedReels)
{
	for (size_t reelNum = 0; reelNum < maxReels; reelNum++)
	{
		if (!lockedReelVect[reelNum] || getLockedReels)
		{
			setSymbolsForReel(reelNum, &spinResult.reelSet, spinResult.stopIndexVect[reelNum]);
		}
	}
	spinResultPtr = std::make_unique<SpinResult>(std::move(spinResult));
}

void Grid::setSymbolsForReel(const size_t reelNum, const ReelSet* reelSetPtr, const size_t reelStop)
{
	if (!reelHasStickySymbolVect[reelNum]) [[likely]]
	{
		MEMCPY_SAFE(reelSymbolVect[reelNum].data(), reelDataBytes,
		            &reelSetPtr->wrappedBandVect[reelNum][reelStop],
		            reelDataBytes);
		return;
	}

	for (size_t rowNum = 0; rowNum < reelHeightVect[reelNum]; rowNum++)
		if (!stickySymbolVect[reelNum][rowNum])
			reelSymbolVect[reelNum][rowNum] = reelSetPtr->wrappedBandVect[reelNum][rowNum + reelStop];
}

/**************************************************/
// Spin locks
/**************************************************/

// Set lockedVect for each reel in noSpinVect.
// noSpinVect is a list of reel indexes that are to be locked
void Grid::setSpinLock(const std::vector<int>& noSpinVect)
{
	clearSpinLock();
	for (int i : noSpinVect)
		lockedReelVect[i] = true;
}

// Set lockedVect = spinStateVect.
// spinStateVect contains a value for every reel
void Grid::setSpinLock(const std::vector<bool>& spinStateVect)
{
	if (maxReels != spinStateVect.size())
	{
		reportError("ReelSet::setSpinLock", "Passed 'spinStateVect' is the wrong size");
		return;
	}
	lockedReelVect = spinStateVect;
}


void Grid::clearSpinLock()
{
	lockedReelVect.resize(maxReels, false);
}

/**************************************************/
// Sticky Symbols
/**************************************************/

void Grid::clearSticky()
{
	reelHasStickySymbolVect.resize(maxReels, false);
	stickySymbolVect.resize(maxReels);
	for (size_t reel = 0; reel < maxReels; reel++)
		stickySymbolVect[reel].resize(maxRows, false);
}

void Grid::setSticky(const size_t reel, const size_t row, const bool isSticky)
{
	stickySymbolVect[reel][row] = isSticky;
	if (isSticky && !reelHasStickySymbolVect[reel])
	{
		reelHasStickySymbolVect[reel] = true;
		return;
	}
	if (!isSticky && reelHasStickySymbolVect[reel])
	{
		for (const bool cellSticky : stickySymbolVect[reel])
			if (cellSticky) return;
		reelHasStickySymbolVect[reel] = false;
	}
}

void Grid::setSticky(const Symbol symbol, const bool isSticky)
{
	for (size_t reel = 0; reel < maxReels; reel++)
		for (size_t row = 0; row < maxRows; row++)
			if (reelSymbolVect[reel][row] == symbol)
				setSticky(reel, row, isSticky);
}

void Grid::setSticky(const std::vector<Symbol>& symbols, const bool isSticky)
{
	for (const Symbol symbol : symbols)
		setSticky(symbol, isSticky);
}

bool Grid::isSticky(const size_t reel, const size_t row)
{
	return stickySymbolVect[reel][row];
}


/**************************************************/
// Set Symbols
/**************************************************/

// replace current reel symbols with 'count' of 'newSymbol' from top to bottom


void Grid::setReelSymbols(const size_t reel, const Symbol newSymbol, const int count)
{
	for (int row = 0; row < count; row++)
		reelSymbolVect[reel][row] = newSymbol;
}

// fill 'reel' with 'newSymbol'
void Grid::setReelSymbols(const size_t reel, const Symbol newSymbol)
{
	for (int row = 0; row < reelHeightVect[reel]; row++)
		reelSymbolVect[reel][row] = newSymbol;
}

// replace current reel symbols with 'newSymbol' for each element in rowVect.
// rowVect may contain:
//		1. An entry for each row position, each non-zero entry is set e.g. { 0,0,1}
//		2. The indexes of rows to be set e.g. {2}.
// Returns vector of grid indexes that have been set.
std::vector<size_t> Grid::setReelSymbols(const size_t reel, const Symbol newSymbol,
                                         const std::vector<size_t>& rowVect)
{
	std::vector<size_t> setIndexVect;
	if (rowVect.size() == maxRows)
	{
		// Option 1 - An entry for each row position
		for (int row = 0; row < reelHeightVect[reel]; row++)
		{
			if (rowVect[row])
			{
				this->setSymbol(reel, row, newSymbol);
				setIndexVect.push_back(this->getGridIndex(reel, row));
			}
		}
	}
	else
	{
		// Option 2 - The indexes of rows to set
		for (const size_t row : rowVect)
		{
			this->setSymbol(reel, row, newSymbol);
			setIndexVect.push_back(this->getGridIndex(reel, row));
		}
	}
	return setIndexVect;
}

/************************************/
// Analysis
/************************************/

// count of sym on passed reel
// optional parameter matchType
int Grid::getReelSymbolCount(const size_t reel, const Symbol sym,
                             const symbolMatchTypes matchType) const
{
	int count = 0;
	for (size_t row = 0; row < reelHeightVect[reel]; ++row)
		if (doSymbolsMatch(reelSymbolVect[reel][row], sym, matchType))
			count++;
	return count;
}

int Grid::getSymbolCount(const Symbol sym, const symbolMatchTypes matchType) const
{
	int total = 0;
	for (size_t reel = 0; reel < maxReels; ++reel)
		total += getReelSymbolCount(reel, sym, matchType);
	return total;
}

// returns first row of passed symbol on passed reel
// returns -1 if symbol not found
std::optional<size_t> Grid::getReelSymbolRow(const size_t reel, const Symbol sym,
                                             const symbolMatchTypes matchType) const
{
	for (size_t row = 0; row < reelHeightVect[reel]; ++row)
		if (doSymbolsMatch(reelSymbolVect[reel][row], sym, matchType))
			return row;
	return {}; // symbol not found
}

bool Grid::reelContainsSymbol(const size_t reel, const Symbol sym, const symbolMatchTypes matchType) const
{
	for (size_t row = 0; row < reelHeightVect[reel]; ++row)
		if (doSymbolsMatch(reelSymbolVect[reel][row], sym, matchType))
			return true;
	return false;
}

bool Grid::reelContainsSymbol(const size_t reel, const std::vector<Symbol>& symVect,
                              const symbolMatchTypes matchType) const
{
	for (size_t row = 0; row < reelHeightVect[reel]; ++row)
		for (const Symbol sym : symVect)
			if (doSymbolsMatch(reelSymbolVect[reel][row], sym, matchType))
				return true;
	return false;
}

bool Grid::doSymbolsMatch(const Symbol a, const Symbol b, const symbolMatchTypes matchType)
{
	if (matchType == symbolMatchTypes::exact)
		return a == b;
	if (matchType == symbolMatchTypes::greaterEqual)
		return a >= b;
	if (matchType == symbolMatchTypes::greater)
		return a > b;
	if (matchType == symbolMatchTypes::lessEqual)
		return a <= b;
	if (matchType == symbolMatchTypes::less)
		return a < b;
	return false;
}


/************************************/
// Grid processing - expand
/************************************/

// Look for symbol on reel. If found then expand symbol to fill reel
// If optional parameter newSym is passed then expand and replace with newSym
// return number of symbol on reel before expanded
int Grid::expandReelSymbol(const size_t reel, const Symbol symbol, const Symbol newSym)
{
	int count = getReelSymbolCount(reel, symbol, symbolMatchTypes::exact);
	if (count)
		setReelSymbols(reel, newSym == SYMBOL_NONE ? symbol : newSym);
	return count;
}

// Look for symbol on all reels. If found then expand symbol to fill reel
// Return number of reels expanded
int Grid::expandSymbol(Symbol symbol)
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
int Grid::expandSymbol(const Symbol symbol, const std::vector<size_t>& reelCheckVect)
{
	int count = 0;
	for (const size_t reel : reelCheckVect)
	{
		if (expandReelSymbol(reel, symbol))
			count++;
	}
	return count;
}

// Look for symbol on each reel
// If found then expand symbol to fill reel with newSym
// Return number of reels expanded
int Grid::expandSymbolToNew(Symbol symbol, Symbol newSym)
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
int Grid::expandSymbolToNew(const Symbol symbol, const Symbol newSym, const std::vector<int>& reelCheckVect)
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
int Grid::replaceReelSymbol(size_t reel, Symbol symOld, Symbol symNew)
{
	int count = 0;
	for (size_t row = 0; row < maxRows; row++)
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
int Grid::replaceSymbol(const Symbol symOld, const Symbol symNew)
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
// optionally pass list of indexes which cannot be replaced
std::vector<int> Grid::replaceRandomSymbols(const Symbol newSymbol, const int count,
                                            const std::vector<int>& blockedIndexVect)
{
	std::vector<int> gridPosVect;

	// quick check to see if it can be done
	const int symbolCount = getSymbolCount(newSymbol);
	if (maxCells - symbolCount < count)
		return gridPosVect;

	// create the list of options to replace
	std::vector<int> gridPosOptions;
	if (!blockedIndexVect.empty())
	{
		// build a list of grid indexes that do not contain newSymbol and are not blocked
		for (int i = 0; i < maxCells; i++)
		{
			if (getSymbol(i) != newSymbol)
			{
				const bool allow = // allow if not in blockedIndexVect
					std::ranges::find(blockedIndexVect, i) ==
					blockedIndexVect.end();
				if (allow)
					gridPosOptions.push_back(i);
			}
		}
	}
	else
	{
		// build a list of all grid indexes that do not contain newSymbol
		for (int i = 0; i < maxCells; i++)
		{
			if (getSymbol(i) != newSymbol)
				gridPosOptions.push_back(i);
		}
	}

	// pick and replace symbols at count positions in grid
	for (int i = 0; i < count; i++)
	{
		const uint32 index = mathsRng.getInt(static_cast<uint32_t>(gridPosOptions.size()),
		                                     "replace random symbols");
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
int Grid::replaceSymbol(const Symbol symOld, const Symbol symNew, const std::vector<int>& reelCheckVect)
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
int Grid::replaceReelNotSymbol(const int reel, const Symbol excludedSym, const Symbol symNew)
{
	int count = 0;
	for (int row = 0; row < maxRows; row++)
	{
		if (reelSymbolVect[reel][row] != excludedSym)
		{
			reelSymbolVect[reel][row] = symNew;
			count++;
		}
	}
	return count;
}

// In the whole grid replace all symbols that are NOT symOld with symNew
// return number reels where symbol was replaced
int Grid::replaceNotSymbol(const Symbol excludedSym, const Symbol symNew)
{
	int count = 0;
	for (int reel = 0; reel < maxReels; reel++)
	{
		if (replaceReelNotSymbol(reel, excludedSym, symNew))
			count++;
	}
	return count;
}

/************************************/
// Grid processing - cascade
/************************************/

void Grid::cascadeSymbols(std::vector<std::pair<size_t, size_t>> symbolsToRemove)
{
	const std::vector<size_t> symbolsRemovedPerReel = removeSymbols(symbolsToRemove);
	addSymbolsToTopOfReels(symbolsRemovedPerReel);
}

std::vector<std::pair<size_t, size_t>> Grid::cascadeWins(const std::vector<std::unique_ptr<Win>>& wins)
{
	using Position = std::pair<size_t, size_t>;
	std::vector<Position> symbolsToRemove;

	size_t totalPositions = 0;
	for (const auto& win : wins)
		totalPositions += win->positions.size();
	symbolsToRemove.reserve(totalPositions);

	for (const auto& win : wins)
		symbolsToRemove.insert(symbolsToRemove.end(), win->positions.begin(), win->positions.end());

	std::ranges::sort(symbolsToRemove, [](const auto& a, const auto& b)
	{
		if (a.second != b.second)
			return a.second > b.second;
		return a.first < b.first;
	});

	symbolsToRemove.erase(std::ranges::unique(symbolsToRemove).begin(), symbolsToRemove.end());

	const std::vector<size_t> symbolsRemovedPerReel = removeSymbols(symbolsToRemove, true);
	addSymbolsToTopOfReels(symbolsRemovedPerReel);

	return symbolsToRemove;
}

std::vector<size_t> Grid::removeSymbols(std::vector<std::pair<size_t, size_t>>& symbolsToRemove,
                                        const bool sorted)
{
	if (!sorted)
		std::ranges::sort(symbolsToRemove, [](const auto& a, const auto& b)
		{
			return a.second < b.second;
		});
	std::vector<size_t> symbolsRemovedPerReel(maxReels, 0);
	for (const auto& [reel, row] : symbolsToRemove)
	{
		reelSymbolVect[reel].erase(reelSymbolVect[reel].begin() + row);
		symbolsRemovedPerReel[reel]++;
	}
	return symbolsRemovedPerReel;
}

void Grid::addSymbolsToTopOfReels(const std::vector<size_t>& symbolsPerReel)
{
	if (symbolsPerReel.size() != maxReels) [[unlikely]]
		throw std::runtime_error("addSymbolsToTopOfReels: passed vector size must match number of reels.");
	for (size_t reel = 0; reel < maxReels; reel++)
	{
		if (symbolsPerReel[reel] == 0) continue;
		using diff_t = std::vector<Symbol>::difference_type;
		const auto reelSetStartPosition = static_cast<diff_t>((spinResultPtr->stopIndexVect[reel] -
			symbolsPerReel[reel] - cascadedSymbolsPerReel[reel]) % spinResultPtr->reelSet.reelSizeVect[reel]);
		const auto& reelSetStartPositionIt = spinResultPtr->reelSet.wrappedBandVect[reel].begin() +
			reelSetStartPosition;
		reelSymbolVect[reel].insert(reelSymbolVect[reel].begin(), reelSetStartPositionIt,
		                            reelSetStartPositionIt + static_cast<diff_t>(symbolsPerReel[reel]));
		cascadedSymbolsPerReel[reel] += symbolsPerReel[reel];
	}
}
