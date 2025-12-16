#pragma once

// cpp-dev-library
#include "../headers/pch.h"

class PayLines {
public:
	int lineCount = 0;
	bool blnVerified = false;
	bool blnError = false;
	std::vector<std::vector<int> > lineVect;


	// default constructor
	PayLines() {
		blnVerified = blnError = false;
	}

	// Pass the games linee definition vector
	PayLines(std::vector<std::vector<int> > dataVect) {
		blnVerified = blnError = false;
		lineVect = dataVect; // create our local copy
		lineCount = (int) lineVect.size();
	}


	~PayLines() {
	}


	// Call to check the line data vector passed when created. return true if error 
	//		vector.size == numWinLines
	//		all lines defined for the correct number of reels and are unique
	bool checkData(const int numWinLines, const int reelCount) {
		std::string szMethod = "PayLines::checkData()";
		std::string szError = "";

		blnError = numWinLines != lineCount;
		if (blnError) {
			szError = "numWinLines != lineVect.size() : <";
			szError += (std::to_string(numWinLines) + " != " + std::to_string(lineCount) + ">");
			reportError(szMethod, szError);
		}

		for (int testLine = 0; testLine < lineCount; testLine++) {
			if ((int) lineVect[testLine].size() != reelCount) {
				szError = "Winline Definition Error: Line " + std::to_string(testLine) + " is too ";
				szError += (int) lineVect[testLine].size() > reelCount ? "long" : "short";
				reportError(szMethod, szError);
			}

			for (int lineIndex = testLine + 1; lineIndex < lineCount; lineIndex++) {
				// check all lines after this one
				int reelIndex = 0;
				while (reelIndex < reelCount && (lineVect[testLine][reelIndex] == lineVect[lineIndex][reelIndex]))
					reelIndex++;

				if (reelIndex == reelCount) {
					szError = "Winline Definition Error: <";
					szError += ("Line " + std::to_string(testLine) + " == " + std::to_string(lineIndex) + ">");
					reportError("PayLines::checkData()", szError);
					blnError = true;
				}
			}
		}

		blnVerified = true;
		return blnError;
	}

	static int getRow(const std::vector<int> *line, const size_t reel) {
		if (!line) reportError("PayLines::GetRow", "Line is null.");
		return (*line)[reel];
	}

	inline int getRow(const int lineNum, const int reel) const {
		return lineVect[lineNum][reel];
	}
};

