#include "ReelSet.h"
#include "../../rng/MathsRng.h"
#include "../../grids/Grid.h"

#ifdef _WIN32
#define MEMCPY_SAFE(dest, destSz, src, count) memcpy_s(dest, destSz, src, count)
#else
#define MEMCPY_SAFE(dest, destsz, src, count)                                  \
do {                                                                       \
size_t _d = static_cast<size_t>(destsz);                               \
size_t _c = static_cast<size_t>(count);                                \
assert(_c <= _d && "MEMCPY_SAFE: count > dest size");                  \
std::memcpy((dest), (src), _c);                                        \
} while (0)

#endif


static bool blnFolderSet = false;
std::string szReelSetFolder = "reelSets";

/*******************************/
// Constructors
/*******************************/

ReelSet::ReelSet(const std::string& szName, const std::vector<std::vector<Symbol>>& bandsVect)
{
	mSzName = szName;
	mSourceReelsVect = bandsVect;

	for (const auto& i : mSourceReelsVect)
		reelSizeVect.push_back(i.size());

	reelSizeVect.resize(mSourceReelsVect.size(), 0);
}

ReelSet::ReelSet(const std::string& szName, const size_t maxReelWindowHeight,
                 const std::vector<std::vector<Symbol>>& bandsVect)
	: ReelSet(szName, bandsVect)
{
	createWrappedBands(maxReelWindowHeight);
}

ReelSet::~ReelSet()
{
	for (auto& i : mSourceReelsVect)
		i.clear();
	mSourceReelsVect.clear();
	for (auto& i : wrappedBandVect)
		i.clear();
	wrappedBandVect.clear();
}

// Call for each ReelSet during initialisation
// Creates bands with wrap around of "maxHeight" symbols
void ReelSet::createWrappedBands(const size_t maxHeight)
{
	if (!mSourceReelsVect.empty())
	{
		// new method - old methods should have created, "mSourceReelsVect"
		wrappedBandVect.resize(mSourceReelsVect.size());
		for (size_t i = 0; i < mSourceReelsVect.size(); i++)
			createWrappedBand(wrappedBandVect[i], mSourceReelsVect[i], maxHeight);
	}
	else
		reportError("ReelSet::createWrappedBands", "No data in 'mSourceReelsVect'");
}

void ReelSet::createWrappedBand(std::vector<Symbol>& dest, const std::vector<Symbol>& source,
                                const size_t maxHeight)
{
	dest = source;
	for (size_t i = 0; i < maxHeight; i++)
		dest.push_back(source[i]);
}

// Returns the symbol at step 'index' on the passed 'reel'
// If 'index' < 0 or > reel length, then the value is "looped" into the reel band range
int ReelSet::getSymbol(const size_t reel, size_t index) const
{
	while (index > reelSizeVect[reel])
		index -= reelSizeVect[reel];
	while (index < 0)
		index += reelSizeVect[reel];
	return wrappedBandVect[reel][index];
}


void ReelSet::fileOutput(std::map<int, std::string> symbolMap) const
{
	std::ofstream outfile;

	if (!blnFolderSet)
	{
		if (makeSubFolder(szReelSetFolder))
		{
			std::cout << "\nCannot create ReelSet output folder: \\" << szReelSetFolder <<
				std::endl;
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
		reportError("ReelSet.fileOutput", "Can't open file: " + mSzName + ". Output aborted",
		            "Warning");
		return;
	}

	size_t maxReelLen = 0;
	for (const auto& rl : mSourceReelsVect)
	{
		size_t rLen = rl.size();
		if (rLen > maxReelLen)
			maxReelLen = rLen;
	}

	std::cout << "\t" << mSzName << std::endl;
	std::stringstream ss;
	if (!symbolMap.empty())
	{
		for (int i = 0; i < maxReelLen; i++)
		{
			ss.str("");
			for (std::vector<Symbol> reel : mSourceReelsVect)
				ss << ((i < reel.size()) ? symbolMap[reel[i]] : "") << ",";
			outfile << ss.str() << std::endl;
			//printf_s(fp, "%s\n", ss.str().c_str());
		}
	}
	else
	{
		for (int i = 0; i < maxReelLen; i++)
		{
			ss.str("");
			for (std::vector<Symbol> reel : mSourceReelsVect)
			{
				if (i < reel.size())
					ss << reel[i];
				ss << ",";
			}
			outfile << ss.str() << std::endl;
			//printf_s(fp, "%s\n", ss.str().c_str());
		}
	}
	outfile.close();
}

int ReelSet::checkSymbolBlocks(const int maxBlock, const int noCheckSymbol,
                               const std::vector<std::string>* lpSymbolVect) const
{
	struct blockError
	{
		int reel;
		int pos;
		int blockSize;
		Symbol symbolId;
		std::string szSymbol;
	};
	std::vector<blockError> errVect;

	for (size_t rl = 0; rl < mSourceReelsVect.size(); rl++)
	{
		const size_t rLen = mSourceReelsVect[rl].size();
		int i = 0;
		while (i < rLen)
		{
			const Symbol sym = wrappedBandVect[rl][i];
			int block = 1;
			int j = i == (rLen - 1) ? 0 : i + 1;
			while (i < rLen && wrappedBandVect[rl][j] == sym)
			{
				block++;
				j++;
			}
			if (sym != noCheckSymbol && block > maxBlock)
			{
				errVect.push_back({
					static_cast<int>(rl), i, block, sym,
					(lpSymbolVect && lpSymbolVect->size() > sym) ? lpSymbolVect->at(sym) : ""
				});
				i += block;
			}
			else
				i++;
		}
	}

	if (!errVect.empty())
	{
		printf("Reelset %s FAILED\r\n", mSzName.c_str());
		for (const auto& [reel, pos, blockSize, symbolId, szSymbol] : errVect)
		{
			if (!szSymbol.empty())
				printf("\tReel %d, Pos %3d, Symbol %s, Block Size %d\r\n", reel, pos,
				       szSymbol.c_str(),
				       blockSize);
			else
				printf("\tReel %d, Pos %3d, Symbol %2d, Block Size %d\r\n", reel, pos, symbolId,
				       blockSize);
		}
	}

	return static_cast<int>(errVect.size());
}

Json::Value ReelSet::toJson(const std::vector<std::string>& symbolNames) const
{
	Json::Value reels(Json::arrayValue);
	for (const auto& reel : mSourceReelsVect)
	{
		Json::Value jsonReel(Json::arrayValue);
		for (const auto& j : reel) jsonReel.append(symbolNames[j]);
		reels.append(jsonReel);
	}
	return reels;
}


