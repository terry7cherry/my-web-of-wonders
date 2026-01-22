#pragma once
/**
 * @file PayLines.h
 * @brief Holds and validates payline definitions and provides access helpers.
 */

// cpp-dev-library
#include "../../headers/pch.h"

/**
 * @brief Container for payline definitions with validation utilities.
 */
class PayLines
{
public:
	size_t lineCount = 0;
	bool blnVerified = false;
	bool blnError = false;
	std::vector<std::vector<size_t>> lineVect;


	/**
	 * @brief Default constructor. Creates an empty, unverified payline set.
	 */
	PayLines()
	{
		blnVerified = blnError = false;
	}

	/**
	 * @brief Construct from a vector of line definitions.
	 * @param dataVect Each inner vector lists the apertures included in a line for each row.
	 */
	PayLines(const std::vector<std::vector<size_t>>& dataVect)
	{
		blnVerified = blnError = false;
		lineVect = dataVect; // create our local copy
		lineCount = lineVect.size();
	}


	/**
	 * @brief Destructor.
	 */
	~PayLines()
	{
	}


	/**
	 * @brief Validate line definitions.
	 * @param numWinLines Expected number of lines.
	 * @param reelCount Expected number of reels for each line.
	 * @return true if an error is detected; false if valid.
	 */
	bool checkData(size_t numWinLines, size_t reelCount);

	/**
	 * @brief Get the aperture for a given reel from a line definition.
	 * @param line Pointer to a line vector.
	 * @param reel Reel index.
	 */
	static size_t getRow(const std::vector<size_t>* line, size_t reel);

	/**
	 * @brief Get the aperture for a given line and reel.
	 * @param lineNum Line index.
	 * @param reel Reel index.
	 * @return Row index for that reel on the specified line.
	 */
	size_t getRow(const size_t lineNum, const size_t reel) const
	{
		return lineVect[lineNum][reel];
	}
};

