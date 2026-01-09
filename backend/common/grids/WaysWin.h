#pragma once

// cpp-dev-library
#include "../headers/pch.h"
#include "../mathElements/PayTable.h"


/*******************************************************************/
// This class was updated in January 2023 after discussions about how 
// WaysWin should handle wilds on the 1st reel
/*******************************************************************/
class WaysWin
{
public:
	// grid data - used for sizing win vectors
	int maxReels;
	int maxRows;

	// Win data
	uint8_t symbol = SYMBOL_NONE; // winnning symbol
	int oak = 0; // 'of a kind' - how many symbols in the win
	int winVal = 0; // win for 1 way of this 'oak' of 'symbol'
	int winWays = 1; // how many ways for this 'oak' of 'symbol'
	int totalWinVal = 0; // win for '1'winWays' of this 'oak' of 'symbol'
	int multiplier = 1;
	bool isWildWin = false;
	int wildWays = 1;
	std::vector<int> reelCountVect; // how many 'symbol' on each reel
	std::vector<int> wildCountVect; // how many wild on each reel

	// winning symbol location data
	uint64 winMask = 0; // bit map of winning positions, 1 << (reel + (row * maxReels))

	// array of winning positions, winPosVect[reel][row]			
	// 0= no win, 1= winning symbol, 2= winning wild
	std::vector<std::vector<int>> winPosVect;

	/*****************************/
	// Constructors
	/*****************************/

	// basic constructor with no starting win positions
	// Parameters:
	//		winSymbol		the winning symbol 
	//		maxReelCount	the number of reels in the grid
	WaysWin(uint8_t winSymbol, int maxReelCount, int maxRowCount);

	WaysWin(); // default constructor for creating arrays

	~WaysWin()
	{
		reelCountVect.clear();
		wildCountVect.clear();
		for (size_t i = 0; i < winPosVect.size(); i++)
			winPosVect[i].clear();
	}

	void resizeVectors(void);

	// Identifies a win as all wilds 
	// Call once if 1st symbol is wild
	void setWildWin(bool isWildWin);

	// Add new position (reel, row) for the winning symbol. 
	// Parameter isWild identifies the position as a symbol or a wild
	void addPosition(int reel, int row, bool isWild);

	// Calculates the number of winning ways and returns that value
	// Call once, after all reels have been checked
	int setWays(void);

	// Calculates the total win and returns it
	// Needs the following code sequence
	//		if (this.setWays())
	//		{
	//			if(winVal = lpPayTable->getWin(this.symbol, this.oak))
	//				this.setWin(winVal * betMultiplier)
	int setWin(PayTable* lpPayTable, int betMultiplier);

	// Generates text that describes the WayWin
	// returns string in the format, "3 H1(100) x2 ways [x2 multiplier ] = 400"
	std::string getWinText(std::string szSymbol);

	// Generates text that describes the WayWin
	// returns string in the format, "3 H1(100) x2 ways [x2 multiplier ] = 400"
	std::string getWinText(std::map<int, std::string>& symbolMap);

private:
	// a method used by all constructors to the size of the ways vectors
	void init(int maxReelCount);
};

