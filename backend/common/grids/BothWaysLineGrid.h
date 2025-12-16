#pragma once

// cpp-dev-library
#include "../headers/pch.h"
#include "LineGrid.h"


class BothWaysLineGrid :
		public LineGrid {
public:
	uint32_t lrMask = UINT32_MAX;
	uint32_t rlMask = UINT32_MAX;

	BothWaysLineGrid(int reels, int rows) : LineGrid(reels, rows) {
	}

	uint32_t quickWinCheck(void) {
		int stopReel = 0;

		uint32_t quickWinMask = lrMask = rlMask = 0; // default no wins
		if (lpPayTable)
			stopReel = lpPayTable->minWinSymbols;
		else {
			reportError("PayGrid:quickWinCheck", "No pay table set");
			return quickWinMask;
		}
		int symId = -1;

		// check left_right
		lrMask = UINT32_MAX;
		for (int reel = 0; reel < stopReel; ++reel) {
			uint32_t reelMask = 0;
			for (int row = 0; row < reelHeightVect[reel]; ++row) {
				if ((symId = getReelSymbol(reel, row)) >= 0)
					reelMask |= symbolVect[symId].payMask;
			}
			lrMask &= reelMask;
		}

		// check right_left
		stopReel = maxReels - lpPayTable->minWinSymbols;
		rlMask = UINT32_MAX;
		for (int reel = maxReels - 1; reel >= stopReel; --reel) {
			uint32_t reelMask = 0;
			for (int row = 0; row < reelHeightVect[reel]; ++row) {
				if ((symId = getReelSymbol(reel, row)) >= 0)
					reelMask |= symbolVect[symId].payMask;
			}
			rlMask &= reelMask;
		}

		quickWinMask = lrMask | rlMask;

		return quickWinMask;
	}

	// ReClear winVector then calculate wins. Applys line multipliers in multiplierVect
	// Re-populate winVector and return total win
	// payMaxOakTwice = true: pays both directions for 5oak win, L-R and R-L
	// payMaxOakTwice = false: pays 5oak win as a single pay line, not both directions
	int evaluateWins(int betMultiplier, bool doMeters, bool payMaxOakTwice = true) {
		int totalWin = 0;

		winVector.clear();
		quickWinCheck();
		if (lrMask)
			totalWin = lrWinCheck(betMultiplier, doMeters);
		if (rlMask)
			totalWin += rlWinCheck(betMultiplier, doMeters);

		if (!payMaxOakTwice && totalWin)
			totalWin -= removeMaxOakDuplicates();
		return totalWin;
	}

	// Calculate win for each symbol present in first reel and in lrMask
	int lrWinCheck(int betMultiplier, bool doMeters) {
		int totalWin = 0;
		int lineMultiplier, wildMultiplier = 1;
		int number, consecutiveWilds, wildCount = 0;
		int startSymbol, symbolToMatch;
		int startReel = 0;

		for (int line = 0; line < lpPayLines->lineCount; ++line) {
			bool blnCheckLine;
			int row = lpPayLines->getRow(line, startReel);
			startSymbol = symbolToMatch = symbols[startReel][row];
			symbolData &startSymData = symbolVect[startSymbol];

			if (startSymData.isScatter && !startSymData.isWild)
				blnCheckLine = false;
			else
				blnCheckLine = startSymData.isWild || (lrMask & startSymData.mask);

			if (!blnCheckLine)
				continue; // based on 1st symbol there is no need to check this line


			number = consecutiveWilds = wildCount = 0;
			wildMultiplier = lineMultiplier = 1;
			for (int reel = startReel; reel < maxReels; reel++) {
				// start from beginning again to account for multipliers
				row = lpPayLines->getRow(line, reel);
				int nextSym = symbols[reel][row];
				symbolData &nextSymData = symbolVect[nextSym];

				if (nextSymData.isScatter && !nextSymData.isWild)
					break;
				if (symbolVect[symbolToMatch].isWild || nextSymData.isWild || nextSym == symbolToMatch) {
					number++;
					if (multiplierVect[reel][row] > 0)
						lineMultiplier *= multiplierVect[reel][row];
					if (symbolVect[symbolToMatch].isWild) {
						if (nextSymData.isWild) {
							consecutiveWilds++;
							if (multiplierVect[reel][row] > 0)
								wildMultiplier *= multiplierVect[reel][row];
						} else
							symbolToMatch = nextSym;
					}
					if (nextSymData.isWild)
						wildCount++;
				} else
					break;
			}

			if (number >= lpPayTable->minPaySymbolsVect[symbolToMatch]) {
				if (consecutiveWilds >= lpPayTable->minWinSymbols) {
					// Wilds make a payout. If wins are equal pay more symbols in preference to less wilds
					if ((wildMultiplier * lpPayTable->getWin(startSymbol, consecutiveWilds)) > (
						    lineMultiplier * lpPayTable->getWin(symbolToMatch, number))) {
						//  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
						symbolToMatch = startSymbol;
						number = consecutiveWilds;
						lineMultiplier = wildMultiplier;
					}
				}

				int win = betMultiplier * lpPayTable->getWin(symbolToMatch, number);
				if (win) {
					totalWin += (win * lineMultiplier);
					//if (doMeters)
					//	lpPayTable->meterWin(symbolToMatch, number);

					// add additional line win data - possibly only needed for playable
					Win lineWin;
					lineWin.payLine = line;
					lineWin.win = lineWin.totalWin = win;
					if (lineMultiplier > 1) {
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

	// Calculate win for each symbol present in last reel and in rlMask
	int rlWinCheck(int betMultiplier, bool doMeters) {
		int totalWin = 0;
		int lineMultiplier, wildMultiplier = 1;
		int number, consecutiveWilds, wildCount = 0;
		int startSymbol, symbolToMatch;
		int startReel = maxReels - 1;

		for (int line = 0; line < lpPayLines->lineCount; ++line) {
			bool blnCheckLine;
			int row = lpPayLines->getRow(line, startReel);
			startSymbol = symbolToMatch = symbols[startReel][row];
			symbolData &startSymData = symbolVect[startSymbol];

			if (startSymData.isScatter && !startSymData.isWild)
				blnCheckLine = false;
			else
				blnCheckLine = startSymData.isWild || (rlMask & startSymData.mask);

			if (!blnCheckLine)
				continue; // based on 1st symbol there is no need to check this line


			number = consecutiveWilds = wildCount = 0;
			wildMultiplier = lineMultiplier = 1;
			for (int reel = startReel; reel >= 0; reel--) {
				// start from beginning again to account for multipliers
				row = lpPayLines->getRow(line, reel);
				int nextSym = symbols[reel][row];
				symbolData &nextSymData = symbolVect[nextSym];

				if (nextSymData.isScatter && !nextSymData.isWild)
					break;
				if (symbolVect[symbolToMatch].isWild || nextSymData.isWild || nextSym == symbolToMatch) {
					number++;
					if (multiplierVect[reel][row] > 0)
						lineMultiplier *= multiplierVect[reel][row];
					if (symbolVect[symbolToMatch].isWild) {
						if (nextSymData.isWild) {
							consecutiveWilds++;
							if (multiplierVect[reel][row] > 0)
								wildMultiplier *= multiplierVect[reel][row];
						} else
							symbolToMatch = nextSym;
					}
					if (nextSymData.isWild)
						wildCount++;
				} else
					break;
			}

			if (number >= lpPayTable->minPaySymbolsVect[symbolToMatch]) {
				if (consecutiveWilds >= lpPayTable->minWinSymbols) {
					// Wilds make a payout. If wins are equal pay more symbols in preference to less wilds
					if ((wildMultiplier * lpPayTable->getWin(startSymbol, consecutiveWilds)) > (
						    lineMultiplier * lpPayTable->getWin(symbolToMatch, number))) {
						//  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym.
						symbolToMatch = startSymbol;
						number = consecutiveWilds;
						lineMultiplier = wildMultiplier;
					}
				}

				int win = betMultiplier * lpPayTable->getWin(symbolToMatch, number);
				if (win) {
					totalWin += (win * lineMultiplier);
					//if (doMeters)
					//	lpPayTable->meterWin(symbolToMatch, number);

					// add additional line win data - possibly only needed for playable
					Win lineWin;
					lineWin.payLine = line;
					lineWin.win = lineWin.totalWin = win;
					if (lineMultiplier > 1) {
						lineWin.multiplier = lineMultiplier;
						lineWin.totalWin *= lineMultiplier;
					}
					lineWin.oak = number;
					lineWin.wildCount = wildCount;
					lineWin.symbol = symbolToMatch;
					lineWin.direction = WinDirections::right_left;
					winVector.push_back(lineWin);
				}
			}
		}
		return totalWin;
	}


	int removeMaxOakDuplicates(void) {
		std::vector<Win>::iterator it1, it2;
		int lostWin = 0;

		it1 = winVector.begin();
		do {
			if (it1->oak == this->maxReels) {
				bool done = false;
				it2 = it1 + 1;
				do {
					if (it1->payLine == it2->payLine) {
						lostWin += it2->totalWin;
						it2 = winVector.erase(it2);
						done = true;
					} else
						it2++;
				} while (!done && it2 != winVector.end());
			}
		} while (++it1 != winVector.end());
		return lostWin;
	}
};

