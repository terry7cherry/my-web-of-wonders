#pragma once

// cpp-dev-library
#include "Win.h"
#include "../headers/types.h"
#include "../gameMaths/mathElements/PayTable.h"


/*******************************************************************/
// This class was updated in January 2023 after discussions about how 
// WaysWin should handle wilds on the 1st reel
/*******************************************************************/
class WaysWin : public Win
{
public:
	// grid data - used for sizing win vectors
	size_t maxReels;
	size_t maxRows;

	// Win data
	int ways = 1; // how many ways for this 'oak' of 'symbol'
	bool isWildWin = false;
	int wildWays = 1;
	bool analysisComplete = false;
	std::vector<int> reelCountVect; // how many 'symbol' on each reel
	std::vector<int> wildCountVect; // how many wilds on each reel
	std::vector<std::pair<size_t, size_t>> positions{};

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
	//		winSymbol: the winning symbol
	//		maxReelCount: the number of reels in the grid
	//		maxRowCount: the number of rows in the grid
	WaysWin(Symbol winSymbol, size_t maxReelCount, size_t maxRowCount);

	WaysWin(); // default constructor for creating arrays

	~WaysWin() override
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
	void addPosition(size_t reel, size_t row, bool isWild);

	// Calculates the number of winning ways and returns that value
	// Call once, after all reels have been checked
	int setWays(void);

	// Calculates the total win and returns it
	// Needs the following code sequence
	//		if (this.setWays())
	//		{
	//			if(winVal = payTable.getWin(this.symbol, this.oak))
	//				this.setWin(winVal * betMultiplier)
	int setWin(const PayTable& payTable, int betMultiplier);

	// Generates text that describes the WayWin
	// returns string in the format, "3 H1(100) x2 ways [x2 multiplier ] = 400"
	std::string getWinText(std::string szSymbol) const;

	// Generates text that describes the WayWin
	// returns string in the format, "3 H1(100) x2 ways [x2 multiplier ] = 400"
	std::string getWinText(const std::map<int, std::string>& symbolMap) const;

	Json::Value toJson(const std::vector<std::string>& symbolNames) const override;

	std::unique_ptr<Win> clone() const override
	{
		return std::make_unique<WaysWin>(*this);
	}
};

