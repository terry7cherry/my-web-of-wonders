#pragma once
#include "../headers/pch.h"
#include "Rng.h"
#include "CSVIterator.h"
#include "../utils/utils.h"

;

namespace MathLibrary
{
	class RngData : public Rng
	{
	private:
		int mDataIndex = 0;
		CSVRow mDataRow;
		int mDataGame = -1;
		int mDataPlay = -1;
		int mDataBet = -1;
		int mDataWin = -1;
		int mDataWinMultiplier = 1;
		std::ifstream mDataFile;
		bool doSequenceInput = true;
		bool rnStreamOutput = false;
		std::vector <uint32> rnSpinStream;
		//MTRand* lpMtRng = nullptr;

		// get the next number from the active sequence
		// returns -1 if passed the end of the current sequence 
		uint32 getIntFromSequence(bool isRN);

	public:
		bool rnStreamDone = false;
		bool rnStreamError = false;
		bool matchRandoms = false;
		CSVIterator mRngDataCSVIt;

		uint32 randInt(uint32 range, std::string sz = "") override;

		/***************************************************************/
		// Input sequence methods
		/***************************************************************/

		// Open CSV files containing RNG stream
		// Return: the streams bet level in cent. Zero if error
		int openDataFile(std::string szDataFilename, int mathsBet);

		// Returns the streams bet level in cent. Zero if error
		int readHeader(void);

		// set the sequence of RNG results to use for the next game
		// returns the 1st entry from the sequence, game number
		int startGameSequence(int debugGame);

		inline void printGame(void)
		{
			if (rnStreamOutput)
				std::cout << mDataGame;
		}

		inline void printStream(void)
		{
			if (rnStreamOutput)
				mDataRow.print("RNstream", 4);
		}

		int startSpinSequence(void);

		// get the next float from the active sequence
		// returns -1 if passed the end of the current sequence 
		double getDoubleFromSequence(void);

		inline int getFromSequenceGame(void) {
			return mDataGame;
		}

		// Check end of sequence was reached, reports errors and returns true if error
		bool endSpinSequence(int mathsWin);

		// Stop using the current sequence
		// Check end of sequence was reached and return the win multiplier
		// pass game bet and mathsWin as cash 
		bool endGameSequence(int bet, int mathsWin);

		inline int getGameCount(void) {
			return mDataGame;
		}
	};

	extern RngData rnStream;
}


