#pragma once
/**
 * @file ReelSet.h
 * @brief Represents a set of reels and provides band wrapping and access utilities.
 */

#include "../../headers/pch.h"
#include "json/json.h"

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

	/**
	 * @brief Construct with source bands and create wrapped bands using window height.
	 * @param szName Name of this reel set.
	 * @param maxReelWindowHeight Visible window height used to wrap.
	 * @param bandsVect Source bands per reel.
	 */
	ReelSet(const std::string& szName, size_t maxReelWindowHeight,
	        const std::vector<std::vector<Symbol>>& bandsVect);

	/**
	 * @brief Construct with source bands only; wrapping can be done later.
	 */
	ReelSet(const std::string& szName, const std::vector<std::vector<Symbol>>& bandsVect);


	/**********************************************/
	// destructor
	/**********************************************/

	/**
	 * @brief Destructor.
	 */
	~ReelSet();

	/**********************************************/
	// Debug methods
	/**********************************************/

	/**
	 * @brief Enable/disable debug output of stop picks to sequence.txt.
	 */
	void setDebugOutput(bool doDebugOutput) { mDebugOutput = doDebugOutput; }

	/**********************************************/
	// Initialise methods
	/**********************************************/
	/**
	 * @brief Create wrapped bands, duplicating top symbols to allow simple fetching of grid symbols.
	 * @param maxHeight Number of rows in the visible window.
	 */
	void createWrappedBands(size_t maxHeight);

	/**
	 * @brief Create wrapped bands in place.
	 * @param dest Destination band to write.
	 * @param source Source band to copy from.
	 * @param maxHeight Visible window height used for wrap size.
	 */
	static void createWrappedBand(std::vector<Symbol>& dest, const std::vector<Symbol>& source,
	                              size_t maxHeight);


	/**********************************************/
	// Functional methods
	/**********************************************/

	/**
	 * @brief Get the symbol at a stop index for a given reel, wrapping indices as needed.
	 * @param reel Reel index.
	 * @param index Stop position (wraps within the band).
	 * @return Symbol index at that position.
	 */
	int getSymbol(size_t reel, size_t index) const;


	/**********************************************/
	// Non-maths, non-game methods
	/**********************************************/
	/**
	 * @brief Output reel symbols to a CSV-like file, columns per reel.
	 * @param symbolMap Map from symbol to text for output.
	 */
	void fileOutput(std::map<int, std::string> symbolMap) const;

	/**
	 * @brief Scan reels for contiguous blocks of the same symbol and report exceeding a threshold.
	 * @param maxBlock Max contiguous length before reporting.
	 * @param noCheckSymbol Symbol to ignore (-1 for none).
	 * @param lpSymbolVect Optional names for symbols.
	 * @return Number of blocks found/examined.
	 */
	int checkSymbolBlocks(int maxBlock, int noCheckSymbol,
	                      const std::vector<std::string>* lpSymbolVect = nullptr) const;

	Json::Value toJson(const std::vector<std::string>& symbolNames) const;
};
