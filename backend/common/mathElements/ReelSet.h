#pragma once

#include "../headers/pch.h"
namespace MathLibrary
{
	// Create a vector of this structure to initialise all instances of class ReelSet
	class ReelSet
	{
	private:
		std::vector < std::vector<int>> mSourceReelsVect;
		bool mDebugOutput = false;

	public:
		std::string mSzName = "";
		std::vector <int> stopIndexVect;
		std::vector <int> reelSizeVect;
		std::vector <std::vector<int>> wrappedBandVect;
		std::vector <bool>didSpinVect;
		std::vector <bool>lockedVect;
		bool isLockedSpin = false;

		/**********************************************/
		// constructors
		/**********************************************/

		// uses maxWindow to create wrapped bands
		ReelSet(std::string szName, int maxReelWindowHeight, std::vector < std::vector<int>> bandsVect);

		ReelSet(std::string szName, std::vector < std::vector<int>> bandsVect);


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
		void createWrappedBands(int maxHeight);

		// copy source to dest and add wrap-around of "maxHeight" symbols in dest
		void createWrappedBand(std::vector<int>& dest, std::vector<int>& source, int maxHeight);


		/**********************************************/
		// Functional methods
		/**********************************************/
		// spin all reels to random positions
		void spin(void);

		// spin reels in spinVect to random positions
		void spin(std::vector <int> spinVect);

		// set stopIndexVect[] to passed positions
		// use -1 if not forcing a reel
		void force(std::vector <int> forceVect);

		// spin reels not in noSpinVect to random positions
		void spinNot(std::vector <int> noSpinVect);

		// Set lockedVect for each reel in noSpinVect 
		// noSpinVect is a list of reel indexes that are to be locked
		void setSpinLock(std::vector <int> noSpinVect);

		// Set lockedVect = noSpinVect 
		// spinStateVect contains a value for every reel 
		void setSpinLock(std::vector <bool> spinStateVect);

		// spin reels that are not set in lockedVect
		void spinNotLocked(void);

		// remove all locked reels
		void clearSpinLock(void);

		int* getReelStopPointer(int reel, int offset = 0);

		// Returns the symbol 'row' steps below the current stop position on the passed 'reel'
		// If stop position + 'row' > reel length then the value is "looped" into the reel band range  
		int getStopSymbol(int reel, int row);

		// Returns the symbol at step 'index' on the passed 'reel'
		// If 'index' < 0 or > reel length then the value is "looped" into the reel band range  
		int getSymbol(int reel, int index);

		/**********************************************/
		// Spin manipulators for forcing
		/**********************************************/
		// Steps up through the reel strip until 'symbol' is at the top of the reel window
		// Return values:
		//	true: symbol found, stopIndex is updated
		//	false: symbol not found. stopIndex is unchanged
		// parameter row is not used
		bool stepUpToSymbol(int reel, int symbol, int row = 0);

		// Steps up through the reel strip until symbol with 'overlay' is at the top of the reel window
		// Return values:
		//	true: symbol found, stopIndex is updated
		//	false: symbol not found. stopIndex is unchanged
		// parameter row is not used
		bool stepUpToOverlay(int reel, int overlay, int row = 0);

		// step passed reel down by count symbols
		void stepDown(int reel, int count);


		/**********************************************/
		// Non-maths, non-game methods
		/**********************************************/
		// creates a file containing the reel symbol text in columns, 1st col = reel 1 etc
		void fileOutput(std::map <int, std::string> symbolMap);

		// scan the reels for blocks of the same symbol
		// Parameters
		//		maxBlock - report to screen any symbol blocks greater than passed maxBlock
		//		noCheckSymbol -  a symbol that can be ignored during checking (-1 if no symbols can be ignored
		//		lpSymbolVect - optional pointer to a vector of string that describe the symbols ("H1","H2", "A", "K", "Q" etc.)
		int checkSymbolBlocks(int maxBlock, int noCheckSymbol, std::vector <std::string>* lpSymbolVect = nullptr);

		/**********************************************/
		// Game Display
		/**********************************************/

		// if showName then "Reelset: Name\t\tStops = [0,1,2,3,4...]"
		// else "ReelStops = [0,1,2,3,4...]"
		void showStops(int tabCount)
		{
			std::cout << "Stops =";
			showTabs(tabCount, false);
			int i = 0;
			printf(" [%4d,", stopIndexVect[i++]);
			while (i < stopIndexVect.size() - 1)
				printf("%4d,", stopIndexVect[i++]);
			printf("%4d]", stopIndexVect[i]);
		}

		// if showName then "Reelset: Name\t\tStops = [0,1,2,3,4...]"
		// else "ReelStops = [0,1,2,3,4...]"
		void showName(int tabCount, bool doShowStops = false)
		{
			showTabs(tabCount > 0 ? tabCount - 1 : 0, false);
			printf("%sReelset: %-20s\n", doShowStops ? "" : "\n", mSzName.c_str());
			if (doShowStops)
				showStops(tabCount);
		}
	};
}