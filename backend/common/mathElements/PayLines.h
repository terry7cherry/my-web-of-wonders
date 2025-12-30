#pragma once

// cpp-dev-library
#include "../headers/pch.h"

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
	PayLines(std::vector<std::vector<size_t>> dataVect)
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
	bool checkData(const size_t numWinLines, const size_t reelCount)
	{
		std::string szMethod = "PayLines::checkData()";
		std::string szError = "";

		blnError = numWinLines != lineCount;
		if (blnError)
		{
			szError = "numWinLines != lineVect.size() : <";
			szError += (std::to_string(numWinLines) + " != " + std::to_string(lineCount) + ">");
			reportError(szMethod, szError);
		}

		for (int testLine = 0; testLine < lineCount; testLine++)
		{
			if (lineVect[testLine].size() != reelCount)
			{
				szError = "Winline Definition Error: Line " + std::to_string(testLine) + " is too ";
				szError += lineVect[testLine].size() > reelCount ? "long" : "short";
				reportError(szMethod, szError);
			}

			for (int lineIndex = testLine + 1; lineIndex < lineCount; lineIndex++)
			{
				// check all lines after this one
				int reelIndex = 0;
				while (reelIndex < reelCount && (lineVect[testLine][reelIndex] == lineVect[lineIndex][
					reelIndex]))
					reelIndex++;

				if (reelIndex == reelCount)
				{
					szError = "Winline Definition Error: <";
					szError += ("Line " + std::to_string(testLine) + " == " + std::to_string(lineIndex) +
						">");
					reportError("PayLines::checkData()", szError);
					blnError = true;
				}
			}
		}

		blnVerified = true;
		return blnError;
	}

	static int getRow(const std::vector<size_t>* line, const size_t reel)
	{
		if (!line) reportError("PayLines::GetRow", "Line is null.");
		return (*line)[reel];
	}

	inline int getRow(const size_t lineNum, const size_t reel) const
	{
		return lineVect[lineNum][reel];
	}
};

