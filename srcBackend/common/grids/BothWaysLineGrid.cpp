#include "BothWaysLineGrid.h"

uint32_t BothWaysLineGrid::quickWinCheck()
{
	size_t stopReel = 0;
	const std::vector<SymbolData>& symbolVect = payTable.symbolVect;

	quickWinMask = lrMask = rlMask = 0; // default no wins
	stopReel = payTable.minWinSymbols;
	Symbol symId = SYMBOL_NONE;

	// check left_right
	lrMask = UINT32_MAX;
	for (int reel = 0; reel < stopReel; ++reel)
	{
		uint32_t reelMask = 0;
		for (int row = 0; row < reelHeightVect[reel]; ++row)
		{
			if ((symId = getReelSymbol(reel, row)) != SYMBOL_NONE)
				reelMask |= symbolVect[symId].payMask;
		}
		lrMask &= reelMask;
	}

	// check right_left
	stopReel = maxReels - payTable.minWinSymbols;
	rlMask = UINT32_MAX;
	for (size_t reel = maxReels - 1; reel >= stopReel; --reel)
	{
		uint32_t reelMask = 0;
		for (int row = 0; row < reelHeightVect[reel]; ++row)
		{
			if ((symId = getReelSymbol(reel, row)) != SYMBOL_NONE)
				reelMask |= symbolVect[symId].payMask;
		}
		rlMask &= reelMask;
	}

	quickWinMask = lrMask | rlMask;

	return quickWinMask;
}

int BothWaysLineGrid::evaluateWins(const int betMultiplier,
                                   const MultipliersCombineType combineType,
                                   const bool payMaxOakTwice)
{
	int totalWin = 0;

	winVector.clear();
	quickWinCheck();
	if (lrMask)
		totalWin = lrWinCheck(betMultiplier, combineType);
	if (rlMask)
		totalWin += rlWinCheck(betMultiplier, combineType);
	totalWin += evaluateScatterWins(betMultiplier);

	if (!payMaxOakTwice && totalWin)
		totalWin -= removeMaxOakDuplicates();
	return totalWin;
}

int BothWaysLineGrid::evaluateScatterWins(const int betMultiplier)
{
	int totalScatterWin = 0;
	for (const Symbol scatterSymbol : payTable.scatterSymVect)
	{
		const unsigned int symbolCount = this->getSymbolCount(scatterSymbol);
		if (symbolCount < payTable.minPaySymbolsVect[scatterSymbol])
			continue;
		if (const unsigned int win = betMultiplier * static_cast<unsigned int>(lpPayLines->lineCount) *
			payTable.getWin(scatterSymbol, symbolCount))
		{
			totalScatterWin += win;

			ScatterWin scatterWin;
			scatterWin.win = scatterWin.totalWin = win;
			scatterWin.oak = symbolCount;
			scatterWin.symbol = scatterSymbol;
			scatterWin.winType = "scatter";

			unsigned int scattersToFind = symbolCount;
			for (size_t reel = 0; reel < reelSymbolVect.size(); reel++)
			{
				for (size_t row = 0; row < reelSymbolVect[reel].size(); row++)
					if (reelSymbolVect[reel][row] == scatterSymbol)
					{
						scattersToFind--;
						scatterWin.positions.push_back(std::make_pair(reel, row));
						if (scattersToFind == 0) break;
					}
				if (scattersToFind == 0) break;
			}
			winVector.emplace_back(std::make_unique<ScatterWin>(scatterWin));
		}
	}
	return totalScatterWin;
}

int BothWaysLineGrid::lrWinCheck(const int betMultiplier, const MultipliersCombineType combineType)
{
	int totalWin = 0;
	int lineMultiplier;
	unsigned int consecutiveWilds, wildCount = 0;
	Symbol symbolToMatch;
	const std::vector<SymbolData>& symbolVect = payTable.symbolVect;

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
				lineMultiplier = combineMultipliers(lineMultiplier, multiplierVect[reel][row], combineType);
				if (symbolVect[symbolToMatch].isWild)
				{
					if (nextSymData.isWild)
					{
						consecutiveWilds++;
						wildMultiplier = combineMultipliers(wildMultiplier, multiplierVect[reel][row],
						                                    combineType);
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

		if (number >= payTable.minPaySymbolsVect[symbolToMatch])
		{
			if (consecutiveWilds >= payTable.minWinSymbols)
			{
				// Wilds make a payout. If wins are equal pay more symbols in preference to fewer wilds
				if ((wildMultiplier * payTable.getWin(startSymbol, consecutiveWilds)) > (
					lineMultiplier * payTable.getWin(symbolToMatch, number)))
				{
					//  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
					symbolToMatch = startSymbol;
					number = consecutiveWilds;
					lineMultiplier = wildMultiplier;
				}
			}

			if (const int win = betMultiplier * payTable.getWin(symbolToMatch, number))
			{
				totalWin += (win * lineMultiplier);
				LineWin lineWin;
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
				winVector.emplace_back(std::make_unique<LineWin>(lineWin));
			}
		}
	}
	return totalWin;
}

int BothWaysLineGrid::rlWinCheck(const int betMultiplier, const MultipliersCombineType combineType)
{
	int totalWin = 0;
	int lineMultiplier;
	unsigned int consecutiveWilds, wildCount = 0;
	Symbol symbolToMatch;
	const size_t startReel = maxReels - 1;
	const std::vector<SymbolData>& symbolVect = payTable.symbolVect;

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
				lineMultiplier = combineMultipliers(lineMultiplier, multiplierVect[reel][row], combineType);
				if (symbolVect[symbolToMatch].isWild)
				{
					if (nextSymData.isWild)
					{
						consecutiveWilds++;
						wildMultiplier = combineMultipliers(wildMultiplier, multiplierVect[reel][row],
						                                    combineType);
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

		if (number >= payTable.minPaySymbolsVect[symbolToMatch])
		{
			if (consecutiveWilds >= payTable.minWinSymbols)
			{
				// Wilds make a payout. If wins are equal pay more symbols in preference to fewer wilds
				if ((wildMultiplier * payTable.getWin(startSymbol, consecutiveWilds)) > (
					lineMultiplier * payTable.getWin(symbolToMatch, number)))
				{
					//  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
					symbolToMatch = startSymbol;
					number = consecutiveWilds;
					lineMultiplier = wildMultiplier;
				}
			}

			if (const int win = betMultiplier * payTable.getWin(symbolToMatch, number))
			{
				totalWin += (win * lineMultiplier);
				LineWin lineWin;
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
				winVector.emplace_back(std::make_unique<LineWin>(lineWin));
			}
		}
	}
	return totalWin;
}
