#pragma once

#include "../../headers/pch.h"

// Create a vector of this structure to initialise all instances of class ReelSet
class ReelSet
{
private:
	std::vector<std::vector<Symbol>> mSourceReelsVect;
	bool mDebugOutput = false;

public:
	std::string mSzName = "";
	std::vector<size_t> reelSizeVect;
	std::vector<std::vector<Symbol>> wrappedBandVect;

	/**********************************************/
	// constructors
	/**********************************************/

	// uses maxWindow to create wrapped bands
	ReelSet(const std::string& szName, size_t maxReelWindowHeight,
	        const std::vector<std::vector<Symbol>>& bandsVect);

	ReelSet(const std::string& szName, const std::vector<std::vector<Symbol>>& bandsVect);


	/**********************************************/
	// destructor
	/**********************************************/

	// frees memory in vectors
	~ReelSet();

	/**********************************************/
	// Debug methods
	/**********************************************/

	// Pass true to show stop index pick details in file sequence.txt
	void setDebugOutput(bool doDebugOutput) { mDebugOutput = doDebugOutput; }

	/**********************************************/
	// Initialise methods
	/**********************************************/
	// Call for each ReelSet during initialisation
	// Creates bands with wrap-around of "maxHeight" symbols
	void createWrappedBands(size_t maxHeight);

	// copy source to dest and add wrap-around of "maxHeight" symbols in dest
	static void createWrappedBand(std::vector<Symbol>& dest, const std::vector<Symbol>& source,
	                              size_t maxHeight);


	/**********************************************/
	// Functional methods
	/**********************************************/

	// Returns the symbol at step 'index' on the passed 'reel'
	// If 'index' < 0 or > reel length then the value is "looped" into the reel band range
	int getSymbol(size_t reel, size_t index) const;


	/**********************************************/
	// Non-maths, non-game methods
	/**********************************************/
	// creates a file containing the reel symbol text in columns, 1st col = reel 1 etc
	void fileOutput(std::map<int, std::string> symbolMap) const;

	// scan the reels for blocks of the same symbol
	// Parameters
	//		maxBlock - report to screen any symbol blocks greater than passed maxBlock
	//		noCheckSymbol -  a symbol that can be ignored during checking (-1 if no symbols can be ignored
	//		lpSymbolVect - optional pointer to a vector of string that describe the symbols ("H1","H2", "A", "K", "Q" etc.)
	int checkSymbolBlocks(int maxBlock, int noCheckSymbol,
	                      std::vector<std::string>* lpSymbolVect = nullptr) const;

	/**********************************************/
	// Game Display
	/**********************************************/

	// if showName then "Reelset: Name\t\tStops = [0,1,2,3,4...]"
	// else "ReelStops = [0,1,2,3,4...]"
	void showName(const int tabCount) const
	{
		showTabs(tabCount > 0 ? tabCount - 1 : 0, false);
		printf("%sReelset: %-20s\n", "\n", mSzName.c_str());
	}
};
