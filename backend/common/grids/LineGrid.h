#pragma once

// cpp-dev-library
#include "../headers/pch.h"
#include "../mathElements/PayTable.h"
#include "PayGrid.h"
#include "../console/Console.h"
#include "../mathElements/PayLines.h"

#include <algorithm>  // for std::remove
#include <cstdio>     // for std::snprintf

// game globals
extern Console gameConsole;

#define MAX_LINE_TEXT_LEN 45

class Win
{
public:
	int symbol = -1;
	int oak = 0;
	int wildCount = 0;
	int multiplier = 1;
	int win = 0;
	int totalWin = 0;
	WinDirections direction = WinDirections::none;
	std::vector<int>* payLine = nullptr;
	int payLineNum = -1;
	std::string winType;

	Win()
	{
		reset(true);
	}

	void reset(bool force = false)
	{
		if (win || force)
		{
			symbol = -1;
			multiplier = 1;
			oak = wildCount = win = totalWin = 0;
			direction = WinDirections::none;
		}
	}

	int applyMultiplier(int newMultiplier, MultipliersCombineType combineType)
	{
		if (newMultiplier <= 0)
			reportError("Win.applyMultiplier", "Passed multiplier <= 0");
		else if (multiplier <= 0)
			reportError("Win.applyMultiplier", "Win multiplier <= 0");
		else
		{   // update the win multiplier and win
			if (combineType == MultipliersCombineType::add)
				multiplier = (multiplier <= 1) ? newMultiplier : multiplier + newMultiplier;
			else if (combineType == MultipliersCombineType::multiply)
				multiplier *= newMultiplier;
			totalWin = win * multiplier;
		}
		return totalWin;
	}

	/****************************************/
	// Display methods
	/****************************************/

	// Generates text that describes the LineWin
	// returns string in the format, "Line 7 L-R: 3 H1 = 40 (x2) = 80"
	std::string getWinText(std::string szSymbol, bool showDirection = true)
	{
		char buffer[MAX_LINE_TEXT_LEN];

		// remove spaces from symbol
		auto end_pos = std::remove(szSymbol.begin(), szSymbol.end(), ' ');
		szSymbol.erase(end_pos, szSymbol.end());

		int space = MAX_LINE_TEXT_LEN;
		int size = 0;

		if (winType == "line") {
			if (showDirection)
			{
				size = std::snprintf(
					buffer, space,
					"Line%2d %s: %d %s = %d ",
					payLineNum,
					(direction == WinDirections::left_right) ? "L-R" : "R-L",
					oak,
					szSymbol.c_str(),
					win
				);
			}
			else
			{
				size = std::snprintf(
					buffer, space,
					"Line%2d: %d %s = %d ",
					payLineNum,
					oak,
					szSymbol.c_str(),
					win
				);
			}
		}
		else {
			size = std::snprintf(
				buffer, space,
				"Scatter: %d %s = %d ",
				oak,
				szSymbol.c_str(),
				win
			);
		}

		if (size < 0) size = 0;
		if (size > space) size = space;
		space -= size;

		if (multiplier > 1 && space > 0)
		{
			int added = std::snprintf(
				buffer + size, space,
				"(x%d) = %d",
				multiplier,
				totalWin
			);
			if (added < 0) added = 0;
			if (added > space) added = space;
			size += added;
			space -= added;
		}

		return std::string(buffer, size);
	}

	// Generates text that describes the LineWin
	// returns string in the format, "Line 7: 3 H1 = 40 [x2 line multiplier = 80]"
	std::string getWinText(std::map<int, std::string>& symbolMap, bool showDirection = false)
	{
		std::string szSymbol = symbolMap[symbol];
		return getWinText(szSymbol, showDirection);
	}
};



// A reel Grid with Line wins
class LineGrid :
	public PayGrid
{
protected:
	std::string szWinLineHeader = "Line Wins";
	PayLines* lpPayLines = nullptr;

public:
	std::vector<Win> winVector;
	std::string szBlankWin;

	// default constructor
	// winDirection is left_right 
	LineGrid(int reels, int rows) :
		PayGrid(reels, rows)
	{
		szBlankWin = std::string(MAX_LINE_TEXT_LEN, ' ');
	}

	// Pass the required PayLines
	void setPayLines(PayLines& payLineData)
	{
		lpPayLines = &payLineData;
		winVector.resize(lpPayLines->lineCount);
	}

	// extend the method PayGrid::reset()
	void reset(void)
	{
		PayGrid::reset();
	}


	/************************************/
	// Win evaluation methods
	/************************************/

	// Reset the winVector then calculate wins. Applys line multipliers in multiplierVect
	// Re-populate winVector and return total win
	int evaluateWins(int betMultiplier, bool doMeters, bool useDebugLine = false)
	{
		int totalWin = 0;
		int lineMultiplier, wildMultiplier = 1;
		int number, consecWilds, wildCount = 0;
		int startSymbol, symbolToMatch;

		winVector.clear();

		if (quickWinCheck())
			for (int line = 0; line < lpPayLines->lineCount; ++line)
			{
				bool blnCheckLine;
				int row = lpPayLines->getRow(line, 0);
				startSymbol = symbolToMatch = reelSymbolVect[0][row];
				symbolData& startSymData = symbolVect[startSymbol];

				if (startSymData.isScatter && !startSymData.isWild)
					blnCheckLine = false;
				else
					blnCheckLine = startSymData.isWild || (quickWinMask & startSymData.mask);

				if (!blnCheckLine)
					continue;                // based on 1st symbol there is no need to check this line

				number = consecWilds = wildCount = 0;
				wildMultiplier = lineMultiplier = 1;
				for (int reel = 0; reel < maxReels; reel++)
				{                   // start from beginning again to account for multipliers
					row = lpPayLines->getRow(line, reel);
					int nextSym = reelSymbolVect[reel][row];
					symbolData& nextSymData = symbolVect[nextSym];

					if (nextSymData.isScatter && !nextSymData.isWild)
						break;
					if (symbolVect[symbolToMatch].isWild || nextSymData.isWild || nextSym == symbolToMatch)
					{
						number++;
						if (multiplierVect[reel][row] > 0)
							lineMultiplier *= multiplierVect[reel][row];
						if (symbolVect[symbolToMatch].isWild)
						{
							if (nextSymData.isWild)
							{
								consecWilds++;
								if (multiplierVect[reel][row] > 0)
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
					if (consecWilds >= lpPayTable->minWinSymbols)
					{           // Wilds make a payout. If wins are equal pay more symbols in preference to less wilds    
						if ((wildMultiplier * lpPayTable->getWin(startSymbol, consecWilds)) >
							(lineMultiplier * lpPayTable->getWin(symbolToMatch, number)))
						{       //  Wild payout > symbol payout e.g. W-W-W > W-W-W-sym. 
							symbolToMatch = startSymbol;
							number = consecWilds;
							lineMultiplier = wildMultiplier;
						}
					}

					int win = betMultiplier * lpPayTable->getWin(symbolToMatch, number);
					if (win)
					{
						totalWin += (win * lineMultiplier);
						//if (doMeters)
						//	lpPayTable->meterWin(symbolToMatch, number);

						// add additional line win data - possibly only needed for playable

						Win lineWin;
						lineWin.payLine = this->getPayLine(line);
						lineWin.payLineNum = line;
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
						lineWin.winType = "line";
						winVector.push_back(lineWin);
					}
				}
			}

		for (int scatterSymbol : this->scatterSymVect) {
			int symbolCount = this->getSymbolCount(scatterSymbol);
			if (symbolCount < lpPayTable->minPaySymbolsVect[scatterSymbol]) continue;
			int win = betMultiplier * lpPayLines->lineCount * lpPayTable->getWin(scatterSymbol, symbolCount);
			if (win) {
				totalWin += win;
				//if (doMeters)
				//	lpPayTable->meterWin(scatterSymbol, symbolCount);

				Win scatterWin;
				scatterWin.win = scatterWin.totalWin = win;
				scatterWin.oak = symbolCount;
				scatterWin.symbol = scatterSymbol;
				scatterWin.winType = "scatter";
				winVector.push_back(scatterWin);
			}
		}
		return totalWin;
	}


	// Identify winning symbols in the grid with the following: 
	//      set vector <vector <bool>> isWinVect   // [reel][row]
	//      set vector <int> winIndexVect          // undefined size, populate with index of winning positions
	// return the number of winning symbols, winIndexVect.size()
	int setWinningSymbols(void)
	{
		// check each active payline and set all winning positions in isWinVect
		for (Win& win : winVector)
		{
			if (win.winType == "scatter") {
				if (win.direction == WinDirections::left_right)
				{
					for (int reel = 0; reel < win.oak; reel++)
						isWinVect[reel][(*win.payLine)[reel]] = true;
				}
				else
				{
					for (int reel = maxReels - win.oak; reel < maxReels; reel++)
						isWinVect[reel][(*win.payLine)[reel]] = true;
				}
			}
			else {
				int& symbolId = win.symbol;
				int index = 0;
				for (int row = 0; row < maxRows; row++) {
					for (int reel = 0; reel < maxReels; reel++, index++) {
						if (this->getSymbol(index) == symbolId) {
							isWinVect[reel][row] = true;
						}
					}
				}
			}
		}
		winIndexVect.clear();
		int index = 0;
		for (int row = 0; row < maxRows; row++) {
			for (int reel = 0; reel < maxReels; reel++, index++) {
				if (isWinVect[reel][row])
					winIndexVect.push_back(index);
			}
		}

		return static_cast<int>(winIndexVect.size());
	}

	// count the number of active multipliers in the passed win
	int countWinMultipliers(Win& win)
	{
		int multiplierCount = 0;

		// set reels for win.direction == WinDirections::left_right
		int startReel = 0, endReel = win.oak;

		if (win.direction == WinDirections::right_left) {
			startReel = maxReels - win.oak;
			endReel = maxReels;
		}

		for (int reel = startReel; reel < endReel; reel++) {
			if ((multiplierVect[reel][(*win.payLine)[reel]]) > 0)
				multiplierCount++;
		}
		return multiplierCount;
	}

	// count the number of multipliers in multiplierVect that are involved in wins
	int countActiveWinMultipliers(void)
	{
		int activeMultipliers = 0, multiplierCount = 0;
		int maxMultipliers = static_cast<int>(multiplierVect.size());

		std::vector<std::vector<bool>> activeMultiplierVect(maxReels);
		for (int reel = 0; reel < maxReels; reel++)
			activeMultiplierVect[reel].resize(maxRows, false);

		for (size_t i = 0; i < winVector.size(); i++)
		{
			Win& win = winVector[i];
			int startReel = (win.direction == WinDirections::left_right) ? 0 : maxReels - win.oak;
			int endReel = (win.direction == WinDirections::left_right) ? win.oak : maxReels;
			for (int reel = startReel; multiplierVect.size() && reel < endReel; reel++)
			{
				int row = lpPayLines->getRow(win.payLine, reel);
				if (multiplierVect[reel][row] > 0)
					activeMultiplierVect[reel][row] = true;
			}
		}

		int activeCount = 0;
		for (int reel = 0; reel < maxReels; reel++) {
			for (int row = 0; row < maxRows; row++) {
				if (activeMultiplierVect[reel][row])
					activeCount++;
			}
		}

		return activeCount;
	}

	// Applies Grid.gridMultiplier (or optional passed, specialMultiplie) to all wins
	// Applies multiplier to each win and returns total win after multiplier is applied
	int applyGridMultiplier(MultipliersCombineType combineType, int specialMultiplier = 0)
	{
		int applyMultiplier = specialMultiplier ? specialMultiplier : this->gridMultiplier;
		int totalWin = 0;
		if (applyMultiplier > 1)
		{
			for (size_t i = 0; i < winVector.size(); i++)
				totalWin += winVector[i].applyMultiplier(applyMultiplier, combineType);
		}
		return totalWin;
	}

	// check each win to see if includes a value in 'multiplierVect'
	// if more than 1 symbol in a win contains a multiplier then they are combined using the passed combineType
	//      MultipliersCombineType::multiply or MultipliersCombineType::add
	// Returns the total win after multipliers are applied to all wins
	int applyWinMultipliers(MultipliersCombineType combineType)
	{
		int startReel = 0, endReel = 0;
		int totalWin = 0;
		for (Win& win : winVector) {
			int winMultiplier = win.multiplier;

			if (win.winType == "line") {
				startReel = 0;
				endReel = win.oak;
			}
			else if (win.direction == WinDirections::right_left) {
				startReel = maxReels - win.oak;
				endReel = maxReels;
			}

			for (int reel = startReel; reel < endReel; reel++) {
				int cellMultiplier = multiplierVect[reel][lpPayLines->getRow(win.payLine, reel)];
				if (cellMultiplier > 0)
				{
					if (combineType == MultipliersCombineType::multiply)
						winMultiplier *= cellMultiplier;
					else if (combineType == MultipliersCombineType::add)
						winMultiplier += cellMultiplier;
				}
			}
			totalWin += (winMultiplier > 1) ? win.applyMultiplier(winMultiplier, combineType)
				: win.totalWin;
		}
		return totalWin;
	}

	/****************************************/
	// Display methods
	/****************************************/

	// Clear any previous win data 
	// Show the winning line data to screen, 1 line per screen row
	int showWins(int startRow, int col, bool showTitle = true, bool showDirection = false)
	{
		if (!mTextMap.size())
		{
			reportError("LineGrid::showWins", "Call 'Grid::setTextMap()' before using this method");
			return 0;
		}

		if (showTitle && winVector.size())
		{
			gameConsole.setCursorPos(startRow++, col);
			gameConsole.coutStyle(szWinLineHeader, CONSOLE_INVERT);
		}

		int row = startRow;
		for (int i = 0; i < lpPayLines->lineCount; i++)
		{               // clear previous output
			gameConsole.setCursorPos(row++, col);
			std::cout << szBlankWin;
		}

		row = startRow;
		for (auto& win : winVector)
		{               // output current win lines
			if (win.winType == "line") {
				if (win.payLine)
				{
					gameConsole.setCursorPos(row++, col);
					std::cout << win.getWinText(mTextMap, showDirection);
				}
			}
			return row - startRow;
		}
	}

	// returns all winnning line data in vector <string> 
	std::vector<std::string> getWinTextVect(std::map<int, std::string>& symbolMap, bool showDirection = false)
	{
		std::vector<std::string> winTextVect;
		if (!mTextMap.size())
			reportError("LineGrid::getWinTextVect", "Call 'Grid::setTextMap()' before using this method");
		else
			for (auto win : winVector)
				winTextVect.push_back(win.getWinText(mTextMap));
		return winTextVect;
	}

	// return pay line at index
	std::vector<int>* getPayLine(int payLineNum) const {
		return &lpPayLines->lineVect[payLineNum];
	}
};

