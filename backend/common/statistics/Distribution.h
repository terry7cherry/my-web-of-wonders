#pragma once

#include <fstream>

#include "../headers/pch.h"


class Distribution {
private:
	std::string szDistributionFolder = "Distributions"; // destinaton folder for output files

	// game name + Distribution type
	// e.g. PhantomTides, PhantomTidesBB, PhantomTidesBonus, PhantomTidesBase
	std::string szBaseFilename = "";

	// current sim configuration. Set using 'setFilename()'
	// format is: szBaseFilename-szMode_rtp_bet
	std::string szFilename = "";

	// szFilename + "_
	std::string szFilePath = "";

	std::map<int, int64> intMap; // distribution data <item, count>

public:
	std::string szExtension = ".csv"; // output file extension
	std::string szSep = "_"; // text seperator used by 'setFilename()'


	// Construct instance with passed BaseFilename, szBaseName
	Distribution(std::string szBaseName) {
		szBaseFilename = szFilename = szBaseName;
	}

	// checks if the passed item alread exits in the distribution
	inline bool exists(int item) {
		return intMap.find(item) != intMap.end();
	}

	// Clear the distribution data
	inline void reset(void) {
		intMap.clear();
	}

	// Clear the distribution data.
	// Rename the output file and check it can be opened
	inline void reset(int rtp, int bet, std::string szMode = "") {
		reset(std::to_string(rtp), std::to_string(bet), szMode);
	}

	// Clear the distribution data.
	// Rename the output file and check it can be opened
	inline void reset(std::string rtp, std::string bet, std::string szMode = "") {
		intMap.clear();
		setFilename(rtp, bet, szMode);
	}

	// Check subfolder exists, create if not
	// Check file can be opened, report issue if not
	void checkFile(void);

	// Create fullFilename suitable for tableau, "baseFileName-Extension_rtp_bet"
	// Check file can be opened
	inline void setFilename(int rtp, int bet, std::string szMode = "") {
		setFilename(std::to_string(rtp), std::to_string(bet), szMode);
	}

	// Create fullFilename suitable for tableau, "baseFileName-Extension_rtp_bet"
	// Check file can be opened
	void setFilename(std::string szRtp, std::string szBet, std::string szMode = "");

	// add an item to the distribution if it does not already exist
	// then increment its count by pased value
	void add(int item, int value = 1);

	// output the distribution to a csv file as <value, count> pair on each line
	// Parameter hiFirst setss the output order, true = high to low
	// Pass doConsoleStatus = true if you want to see the tile of the written file on the console
	void fileOutput(bool hiFirst, bool doConOut = false);

	// get the filename without its path
	inline std::string filenameGet() { return szFilename; }
};


