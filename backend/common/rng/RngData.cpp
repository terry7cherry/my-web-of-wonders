#include "../headers/pch.h"
#include "RngData.h"
#include <cmath>

;

namespace MathLibrary
{
	/**********************************************/
	// Implement the Rng virtual methods
	/**********************************************/

	uint32 RngData::randInt(uint32 range, std::string sz)
	{
		uint32 value = getIntFromSequence(true);
		if (value != -1 && value > range)
		{
			std::string szMsg;
			szMsg = "Game " + std::to_string(mDataGame);
			if (sz.size())
				szMsg += (" : " + sz);
			szMsg += " gave a value outside of 'range' " + std::to_string(int(value));

			reportError("RngWrapper.randInt()", szMsg);
			rnStreamError = true;
		}
		return value;
	}

	/**********************************************/
	// Implement the dataStream methods
	/**********************************************/

	// get the next number from the active sequence
	// returns -1 if passed the end of the current sequence 
	uint32 RngData::getIntFromSequence(bool isRN)
	{
		int val;
		if (mDataIndex >= (int)mDataRow.size())
		{
			if (mDataIndex != 0)
			{
				std::string buffer;
				buffer = "Game " + std::to_string(mDataGame) +
					" Spin " + std::to_string(mDataPlay) + " did not have enough data";
				reportError("getIntFromSequence", buffer);
				rnStreamError = true;
			}
			val = -1;
		}
		else
		{
			val = mDataRow.getUint32Data(mDataIndex++);
			if(isRN)
				rnSpinStream.push_back(val);
		}
		return val;
	}

	/***************************************************************/
	// Input sequence methods
	/***************************************************************/

	// Open CSV files containing RNG stream
	// Return: the streams bet level in cent. Zero if error
	int RngData::openDataFile(std::string szDataFilename, int mathsBet)
	{
		std::string buffer = "";
		std::string szMethod = "RngData.openDataFile";
		doSequenceInput = true;

		// Load RNG file
		mDataFile = std::ifstream(szDataFilename);
		if (!mDataFile)
		{
			doSequenceInput = false;
			reportError(szMethod, "Cannot open sequence file <" + szDataFilename + ">");
		}
		else
		{
			mRngDataCSVIt = CSVIterator(mDataFile);
			if (mRngDataCSVIt == CSVIterator())
			{
				doSequenceInput = false;
				reportError(szMethod, "Sequence file <" + szDataFilename + "> is empty");
			}
		}
		int streamBet = readHeader();
		mDataWinMultiplier = 100/mathsBet;
		return streamBet;
	}

	// Returns the streams bet level in cent. Zero if error
	int RngData::readHeader(void)
	{
		std::string szMethod = "RngData.readHeader";
		bool headerSizeError = false;
		std::string szRow1 = "bet";
		std::vector<std::string> headerVect = { "job", "betInJob", "betAsGrowingTotal", "play", "payout", "frig" };

		mDataRow = *mRngDataCSVIt;

		// row 1 - bet
		mDataIndex = 0;
		if (mDataRow.getString(mDataIndex++) != szRow1)
		{
			// just a warning for now as not doing anything with mDataBet
			reportError(szMethod, "DataFile - Row1[0] != '" + szRow1 + "'");
			return 0;
		}
		mDataBet = getIntFromSequence(false);
		mDataBet *= 100;

		// row 2 - headers
		mDataRow = *(++mRngDataCSVIt);
		if (mDataRow.size() != headerVect.size())
		{
			headerSizeError = true;
			if (mDataRow.size() > headerVect.size())
				reportError(szMethod, "DataFile - More headers than expected", "Warning");
			else
				reportError(szMethod, "DataFile - Less headers than expected", "Warning");
			// Output header entries
			for (mDataIndex = 0; mDataIndex < (int)headerVect.size(); mDataIndex++)
				std::cout << mDataRow.getString(mDataIndex) << ((mDataIndex + 1) < (int)headerVect.size()) ? ", " : "\r\n";
			++mRngDataCSVIt;
		}

		if (++mRngDataCSVIt == CSVIterator())
		{
			reportError("endGameSequence", "No data after Header");
			rnStreamDone = true;
			mDataFile.close();
			return 0;
		}
		return mDataBet;
	}

	// set the sequence of RNG results to use for the next game
	// returns the 1st entry from the sequence, game number
	int RngData::startGameSequence(int debugGame)
	{
		// DEL TEL - To Do
		// remember where game started and return here automatically to debug
		int game = startSpinSequence();
		if (game == mDataGame)
		{
			reportError("RngData.startGameSequence", "New game started before game stream complete");
		}
		else
		{
			mDataGame = game;
			rnStreamOutput = false;
			if (game == debugGame)
				rnStreamOutput = true;
		}
		return game;
	}

	int RngData::startSpinSequence(void)
	{
		int job = -1, jobGameCount = -1, totalGameCount = -1;
		if (doSequenceInput)
		{
			mDataRow = *mRngDataCSVIt;
			mDataIndex = 0;

			// Read 1st 5 data entries: job, betInJob, betAsGrowingTotal, play, payout
			job = getIntFromSequence(false);
			jobGameCount = getIntFromSequence(false);
			int game = getIntFromSequence(false);	// get game number
			mDataPlay = getIntFromSequence(false);	// get game spin number
			double dataWin = getDoubleFromSequence();
			mDataWin = (int)((100 * dataWin) + 0.5);
			return game;
		}
		else
			return -1;
	}




	// get the next float from the active sequence
	// returns -1 if passed the end of the current sequence 
	double RngData::getDoubleFromSequence(void)
	{
		double val;
		if (mDataIndex >= (int)mDataRow.size())
		{
			if (mDataIndex != 0)
			{
				std::string buffer;
				buffer = "Game " + std::to_string(mDataGame) +
					" Spin " + std::to_string(mDataPlay) + " did not have enough data";
				reportError("RngData.getDoubleFromSequence", buffer);
				rnStreamError = true;
			}
			val = -1;
		}
		else
			val = mDataRow.getDoubleData(mDataIndex++);
		return val;
	}

	// Check end of sequence was reached, reports errors and returns true if error
	bool RngData::endSpinSequence(int mathsWin)
	{
		if (!doSequenceInput)
			return true;

		std::string method = "RngData.endSpinSequence";
		bool winMatch = false;
		if (mDataIndex < (int)mDataRow.size())
		{
			std::string buffer;
			buffer = "Game " + std::to_string(mDataGame) + " Spin " + std::to_string(mDataPlay) +
				" did not reach the end of data";
			reportError("RngData.endSpinSequence", buffer);
			rnStreamError = true;
		}
		else
		{
			mathsWin *= mDataWinMultiplier;
			winMatch = (mDataWin == mathsWin);
			if (!winMatch)
			{
				std::string buffer;
				buffer = "Game " + std::to_string(mDataGame) + " Spin " + std::to_string(mDataPlay) +
					" mDataWin != mathsWin (" + std::to_string(mDataWin) + " != " + std::to_string(mathsWin) + ")";
				reportError(method, buffer);
				std::cout << "streamWin = " << std::to_string(mDataWin) << ", mathsWin = " << std::to_string(mathsWin) <<
					", Diff = " << std::to_string(mathsWin > mDataWin ? mathsWin - mDataWin : mDataWin - mathsWin);
			}
		}

		if (++mRngDataCSVIt == CSVIterator())
		{
			reportError(method, "End of RNG data", "Report");
			rnStreamDone = true;
			mDataFile.close();
		}
		else
			startSpinSequence();

		return rnStreamError || !winMatch || rnStreamDone;
	}

	// Stop using the current sequence
	// Check end of sequence was reached and return the win multiplier
	// pass game bet and mathsWin as cash 
	bool RngData::endGameSequence(int bet, int mathsWin)
	{
		std::string method = "RngData.endSpinSequence";

		bool winMatch = false;
		if (doSequenceInput)
		{
			if (mDataIndex < (int)mDataRow.size())
			{
				std::string buffer;
				buffer = "Game " + std::to_string(mDataGame) + " did not reach the end of data";
				reportError(method, buffer);
				rnStreamError = true;
			}
			else
			{
				// This code has failed in Maxiplier fd maths error gave 139.99999999 => 139
				// int streamWin = mDataWin * bet;
				double temp = mDataWin * bet;
				int streamWin = (int)std::round(temp);

				winMatch = (streamWin == mathsWin);
				if (!winMatch)
				{
					std::string buffer;
					buffer = "Game " + std::to_string(mDataGame) + " streamWin != mathsWin (" +
						std::to_string(streamWin) + " != " + std::to_string(mathsWin) + ")";
					reportError(method, buffer);
					std::cout << "streamWin = " << std::to_string(streamWin) << ", mathsWin = " << std::to_string(mathsWin) <<
						", Diff = " << std::to_string(mathsWin > streamWin ? mathsWin - mDataWin : mDataWin - mathsWin);
				}
			}

			if (++mRngDataCSVIt == CSVIterator())
			{
				reportError(method, "End of RNG data", "Report");
				rnStreamDone = true;
				mDataFile.close();
			}
		}
		return winMatch;
	}
}