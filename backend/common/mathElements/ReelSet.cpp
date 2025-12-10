#include "../headers/pch.h"

#include "ReelSet.h"
#include "../rng/MathsRng.h"

;

namespace MathLibrary
{

	static bool blnFolderSet = false;
	std::string szReelSetFolder = "reelSets";

	/*******************************/
	// Constructors
	/*******************************/

	ReelSet::ReelSet(std::string szName, std::vector < std::vector<int>> bandsVect)
	{
		mSzName = szName;
		mSourceReelsVect = bandsVect;

		stopIndexVect.resize(mSourceReelsVect.size(), 0);
		for (int i = 0; i < (int)mSourceReelsVect.size(); i++)
			reelSizeVect.push_back((int)mSourceReelsVect[i].size());

		stopIndexVect.resize(mSourceReelsVect.size(), 0);
		reelSizeVect.resize(mSourceReelsVect.size(), 0);
		didSpinVect.resize(mSourceReelsVect.size(), false);
		lockedVect.resize(mSourceReelsVect.size(), false);
	}

	ReelSet::ReelSet(std::string szName, int maxReelWindowHeight, std::vector < std::vector<int>> bandsVect) :
		ReelSet(szName, bandsVect)
	{
		createWrappedBands(maxReelWindowHeight);
	}


	ReelSet::~ReelSet()
	{
		for (int i = 0; i < mSourceReelsVect.size(); i++)
			mSourceReelsVect[i].clear();
		mSourceReelsVect.clear();
		for (int i = 0; i < wrappedBandVect.size(); i++)
			wrappedBandVect[i].clear();
		wrappedBandVect.clear();
	}

	// Call for each ReelSet during initialisation
	// Creates bands with wrap around of "maxHeight" symbols
	void ReelSet::createWrappedBands(int maxHeight)
	{
		if (mSourceReelsVect.size())
		{							// new method - old methods should have created, "mSourceReelsVect"
			wrappedBandVect.resize(mSourceReelsVect.size());
			for (int i = 0; i < (int)mSourceReelsVect.size(); i++)
				createWrappedBand(wrappedBandVect[i], mSourceReelsVect[i], maxHeight);
		}
		else
			reportError("ReelSet::createWrappedBands", "No data in 'mSourceReelsVect'");
	}

	void ReelSet::createWrappedBand(std::vector<int>& dest, std::vector<int>& source, int maxHeight)
	{
		dest = source;
		for (int i = 0; i < maxHeight; i++)
			dest.push_back(source[i]);
	}

	// spin all reels to random positions
	// Set flags in "didSpinVect" which is used in Grid::setSymbolsFromReels()
	void ReelSet::spin(void)
	{
		for (int i = 0; i < (int)mSourceReelsVect.size(); i++)
		{
			stopIndexVect[i] = mathRng.randInt(reelSizeVect[i]);
			didSpinVect[i] = true;
		}
	}

	// spin the reels in spinVect to random positions
	// Set flags in "didSpinVect" which is used in Grid::setSymbolsFromReels()
	void ReelSet::spin(std::vector <int> spinVect)
	{
		for (int i : spinVect)
		{
			if (i < (int)mSourceReelsVect.size())
			{
				stopIndexVect[i] = mathRng.randInt(reelSizeVect[i], mSzName.c_str());
				didSpinVect[i] = true;
			}
			else
				didSpinVect[i] = false;

		}
	}

	// Set 'stopIndexVect[]' to positions passed in 'forceVect'
	// Use -1 in 'forceVect' when not forcing a reel
	//	e.g. Passing {0,0,-1,-1,-1} forces r1,2 to position 0, r3,4,5 are not changed
	void ReelSet::force(std::vector <int> forceVect)
	{
		if (forceVect.size() > mSourceReelsVect.size())
		{
			printf("\r\nError: %s.force() - forceVect too big", mSzName.c_str());
			return;
		}

		int reel = 0;
		for (int stopPos : forceVect)
		{
			if (stopPos >= reelSizeVect[reel])
				printf("\r\nError: %s.force() - reel %d forceStop out of range (0-%d)\r\n", mSzName.c_str(), reel, reelSizeVect[reel] - 1);
			else if (stopPos != -1)
				stopIndexVect[reel] = stopPos;
			reel++;
		}
	}

	// spin reels that are not set in lockedVect
	void ReelSet::spinNotLocked(void)
	{
		for (int i = 0; i < (int)mSourceReelsVect.size(); i++)
		{
			if (!lockedVect[i])
			{
				stopIndexVect[i] = mathRng.randInt(reelSizeVect[i], mSzName.c_str());
				didSpinVect[i] = true;
			}
			else
				didSpinVect[i] = false;
		}
	}

	// Set lockedVect for each reel in noSpinVect 
	// noSpinVect is a list of reel indexes that are to be locked
	void ReelSet::setSpinLock(std::vector <int> noSpinVect)
	{
		clearSpinLock();
		if (isLockedSpin = noSpinVect.size())
		{
			for (int i : noSpinVect)
				lockedVect[i] = true;
		}
	}

	// Set lockedVect = spinStateVect 
	// spinStateVect contains a value for every reel 
	void ReelSet::setSpinLock(std::vector <bool> spinStateVect)
	{
		if (mSourceReelsVect.size() != spinStateVect.size())
		{
			reportError("ReelSet::setSpinLock", "Passed 'spinStateVect' is the wrong size");
			isLockedSpin = false;
			return;
		}
		isLockedSpin = true;
		lockedVect = spinStateVect;
	}


	void ReelSet::clearSpinLock(void)
	{
		isLockedSpin = false;
		lockedVect.resize(mSourceReelsVect.size(), false);
	}


	int* ReelSet::getReelStopPointer(int reel, int offset)
	{
		if (offset)
		{
			stopIndexVect[reel] += offset;
			while (stopIndexVect[reel] < 0)
				stopIndexVect[reel] += reelSizeVect[reel];
			while (stopIndexVect[reel] > reelSizeVect[reel])
				stopIndexVect[reel] -= reelSizeVect[reel];
		}
		return &wrappedBandVect[reel][stopIndexVect[reel]];
	}

	// Returns the symbol at step 'index' on the passed 'reel'
	// If 'index' < 0 or > reel length then the value is "looped" into the reel band range  
	int ReelSet::getSymbol(int reel, int index)
	{
		while (index > reelSizeVect[reel])
			index -= reelSizeVect[reel];
		while (index < 0)
			index += reelSizeVect[reel];
		return wrappedBandVect[reel][index];
	}

	// Returns the symbol 'row' steps below the current stop position on the passed 'reel'
	// If stop position + 'row' > reel length then the value is "looped" into the reel band range  
	int ReelSet::getStopSymbol(int reel, int row)
	{
		int index = stopIndexVect[reel] + row;
		while (index > wrappedBandVect[reel].size())
			index -= reelSizeVect[reel];
		return wrappedBandVect[reel][index];
	}

	// Steps up through the reel strip until 'symbol' is at the top of the reel window
	// Return values:
	//	true: symbol found, stopIndex is updated
	//	false: symbol not found. stopIndex is unchanged
	// parameter row is not used
	bool ReelSet::stepUpToSymbol(int reel, int symbol, int row)
	{
		int stepCount = 0;
		int stopIndex = stopIndexVect[reel];
		do {			// step up until symbol at top of reel window
			if (++stopIndex >= (int)mSourceReelsVect[reel].size())
				stopIndex = 0;
			stepCount++;
		} while (stepCount <= (int)mSourceReelsVect[reel].size() &&
			wrappedBandVect[reel][stopIndex] != symbol);

		if (wrappedBandVect[reel][stopIndex] == symbol)
		{
			stopIndexVect[reel] = stopIndex;
			return true;
		}
		return false;
	}

	// Steps up through the reel strip until symbol with 'overlay' is at the top of the reel window
	// Return values:
	//	true: symbol found, stopIndex is updated
	//	false: symbol not found. stopIndex is unchanged
	// parameter row is not used
	// DEL TEL - cannot handle 2 overlays - use overlay masks for this
	bool ReelSet::stepUpToOverlay(int reel, int overlay, int row)
	{
		int stepCount = 0;
		int stopIndex = stopIndexVect[reel];
		do {			// step up until symbol at top of reel window
			if (++stopIndex >= (int)mSourceReelsVect[reel].size())
				stopIndex = 0;
			stepCount++;
		} while (stepCount <= (int)mSourceReelsVect[reel].size() &&
			wrappedBandVect[reel][stopIndex] <= overlay);

		if (wrappedBandVect[reel][stopIndex] > overlay)
		{
			stopIndexVect[reel] = stopIndex;
			return true;
		}
		return false;
	}

	void ReelSet::stepDown(int reel, int count)
	{
		stopIndexVect[reel] -= count;
		while (stopIndexVect[reel] < 0)
			stopIndexVect[reel] += (int)mSourceReelsVect[reel].size();
	}

	void ReelSet::fileOutput(std::map <int, std::string> symbolMap)
	{
		std::ofstream outfile;

		if (!blnFolderSet)
		{
			if (makeSubFolder(szReelSetFolder))
			{
				std::cout << "\nCannot create ReelSet output folder: \\" << szReelSetFolder << std::endl;
				exit(1);
			}
			else
				std::cout << "\nReelSets output to folder: \\" << szReelSetFolder << std::endl;
			blnFolderSet = true;
		}

		std::string szFilename = szReelSetFolder;
		szFilename += ("\\" + mSzName + ".csv");
		outfile.open(szFilename);
		if (!outfile.is_open())
		{
			reportError("ReelSet.fileOutput", "Can't open file: " + mSzName + ". Output aborted", "Warning");
			return;
		}

		int maxReelLen = 0;
		for (int rl = 0; rl < (int)mSourceReelsVect.size(); rl++)
		{
			int rLen = (int)mSourceReelsVect[rl].size();
			if (rLen > maxReelLen)
				maxReelLen = rLen;
		}

		std::cout << "\t" << mSzName << std::endl;
		std::stringstream ss;
		if (symbolMap.size())
		{
			for (int i = 0; i < maxReelLen; i++)
			{
				ss.str("");
				for (std::vector<int> reel : mSourceReelsVect)
					ss << ((i < reel.size()) ? symbolMap[reel[i]] : "") << ",";
				outfile << ss.str() << std::endl;
				//fprintf_s(fp, "%s\n", ss.str().c_str());
			}
		}
		else
		{
			for (int i = 0; i < maxReelLen; i++)
			{
				ss.str("");
				for (std::vector<int> reel : mSourceReelsVect)
				{
					if (i < reel.size())
						ss << reel[i];
					ss << ",";
				}
				outfile << ss.str() << std::endl;
				//fprintf_s(fp, "%s\n", ss.str().c_str());
			}
		}
		outfile.close();
	}


	// scan the reels for blocks of the same symbol
	// Parameters
	//		maxBlock - report to screen any symbol blocks greater than passed maxBlock
	//		noCheckSymbol -  a symbol that can be ignored during checking (-1 if no symbols can be ignored
	//		lpSymbolVect - optional pointer to a vector of string that describe the symbols ("H1","H2", "A", "K", "Q" etc.)
	int ReelSet::checkSymbolBlocks(int maxBlock, int noCheckSymbol, std::vector <std::string>* lpSymbolVect)
	{
		struct blockError
		{
			int reel;
			int pos;
			int blockSize;
			int symbolId;
			std::string szSymbol;
		};
		std::vector <blockError> errVect;

		int maxReelLen = 0;
		for (int rl = 0; rl < (int)mSourceReelsVect.size(); rl++)
		{
			int rLen = (int)mSourceReelsVect[rl].size();
			int i = 0;
			while (i < rLen)
			{
				int sym = wrappedBandVect[rl][i];
				int block = 1;
				int j = i == (rLen - 1) ? 0 : i + 1;
				while (i < rLen && wrappedBandVect[rl][j] == sym)
				{
					block++;
					j++;
				}
				if (sym != noCheckSymbol && block > maxBlock)
				{
					errVect.push_back({ rl, i, block, sym, "" });
					i += block;
				}
				else
					i++;
			}
		}

		if (errVect.size())
		{
			printf("Reelset %s FAILED\r\n", mSzName.c_str());
			for (auto err : errVect)
			{
				if (err.szSymbol.length())
					printf("\tReel %d, Pos %3d, Symbol %s, Block Size %d\r\n", err.reel, err.pos, err.szSymbol.c_str(), err.blockSize);
				else
					printf("\tReel %d, Pos %3d, Symbol %2d, Block Size %d\r\n", err.reel, err.pos, err.symbolId, err.blockSize);
			}
		}

		return (int)errVect.size();
	}



}