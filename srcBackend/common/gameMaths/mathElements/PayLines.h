#pragma once

// cpp-dev-library
#include "../../headers/pch.h"

class PayLines
{
public:
	size_t lineCount = 0;
	bool blnVerified = false;
	bool blnError = false;
	std::vector<std::vector<size_t>> lineVect;


	// default constructor
	PayLines()
	{
		blnVerified = blnError = false;
	}

	// Pass the games linee definition vector
	PayLines(const std::vector<std::vector<size_t>>& dataVect)
	{
		blnVerified = blnError = false;
		lineVect = dataVect; // create our local copy
		lineCount = lineVect.size();
	}


	~PayLines()
	{
	}


	// Call to check the line data vector passed when created. return true if error 
	//		vector.size == numWinLines
	//		all lines defined for the correct number of reels and are unique
	bool checkData(size_t numWinLines, size_t reelCount);

	static size_t getRow(const std::vector<size_t>* line, size_t reel);

	size_t getRow(const size_t lineNum, const size_t reel) const
	{
		return lineVect[lineNum][reel];
	}
};

