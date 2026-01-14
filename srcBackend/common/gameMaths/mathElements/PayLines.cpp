#include "PayLines.h"

bool PayLines::checkData(const size_t numWinLines, const size_t reelCount)
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


size_t PayLines::getRow(const std::vector<size_t>* line, const size_t reel)
{
	if (!line)
	{
		reportError("PayLines::GetRow", "Line is null.");
		throw;
	}
	return (*line)[reel];
}

