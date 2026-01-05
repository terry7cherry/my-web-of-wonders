/*
#pragma once



#include "stdio.h"

// MathLibrary includes
#include "../common/headers/cpp-math-library.h"

// cpp-dev-library includes
#include "../common/grids/LineGrid.h"
#include "../common/mathElements/PayTable.h"
#include "../common/console/Console.h"

// Game includes
#include "simConfig.h"
#include "../common/statistics/Event.h"
#include "../common/events/BasicEvent.h"

;

/**************************************************************************************************/
// This file contains 
//		Maths Data - definitions of ReelSet, WeightTable, etc. 
//		Basic inline methods for accessing data
//		Prototypes for more complicated methods
//			Look in GameMaths.cpp for the code for these methods
/**************************************************************************************************/

/*****************************************/
// Game Definitions
/*****************************************/
/*
#define MAX_REELS		5
#define MAX_ROWS		3
#define MAX_PAY_LINES	25

// Symbol definitions
#define W1 0
#define H1 1
#define H2 2
#define H3 3
#define H4 4
#define L1 5
#define L2 6
#define L3 7
#define L4 8
#define SC 9
#define SYM_COUNT 10


/*****************************************/
// Playable Definitions
/*****************************************/
/*
#define LINE_WINS_COLUMN 44

enum ReelsID {
	ReelsIDBaseGame1,
	ReelsIDFreeSpins1,
	ReelsIDCount
};

enum EventID {
	EventIDGame,
	EventIDBaseGame,
	EventIDFreeSpins,
	EventIDBaseGameStd,
	EventIDFreeSpinsStd,
	EventIDCount
};


class __GameMaths {
public:
	/*****************************************/
// Initialize
/*****************************************/
/*
	const std::string gameName = "Mini Project";
	const std::string version = "1.0.0";
	const int minBet = 25;
	int bet = minBet;
	int betMultiplier = 1;
	int rtpIndex = -1;
	std::vector<std::string> rtpOptionVect = {"96"};
	std::vector<int> rtpIntVect = {96};
	int reelsetIndex = -1;
	LineGrid gameGrid = LineGrid(MAX_REELS, MAX_ROWS);
	std::vector<Event> eventVect;

	ReelSet *lpActiveReelSet = nullptr;
	std::vector<std::string> szReelsetVect;

	int fsStartSpins[MAX_REELS + 1]{0, 0, 0, 10, 20, 50};
	int fsRetriggerSpins[MAX_REELS + 1]{0, 0, 5, 10, 20, 50};
	bool fsTrigger = false;
	int scatterCount = 0;
	bool fsActive = false;
	int fsStartMultiplier = 0;
	int fsSpin = 0;
	int fsTotalSpins = 0;
	int fsTotalWin = 0;
	std::vector<std::unique_ptr<BasicEvent> > events;

	void initialise(void);

	void reset(int bet, std::string szMode = "");

	void showWin(int win, int totalWin = -1) const {
		gameConsole.clearLine(LineOut::result);
		if (win)
			printf(" Reel Win = %6d (x%4.2f)\n", win, static_cast<float>(win) / bet);
		else
			printf(" Reel Win = %6d\n", win);

		if (totalWin >= 0 && totalWin > win) {
			if (totalWin)
				printf("Total Win = %6d (x%4.2f)\n", totalWin, static_cast<float>(totalWin) / bet);
			else
				printf("Total Win = %6d\n", totalWin);
		} else
			printf("                             \n");

		if (fsActive) {
			printf("\n");
			if (this->fsTotalWin)
				printf("   FS Win = %6d (x%4.2f)\n", this->fsTotalWin, static_cast<float>(this->fsTotalWin) / bet);
			else
				printf("  FS Win = %6d\n", this->fsTotalWin);
		} else
			printf("\n");
	}

	// Show reels and wins
	// Pass win = -1 to not show win text
	void showSpinResult(int win, int totalWin = -1) {
		gameConsole.setCursorPos(LineOut::reelset);
		this->lpActiveReelSet->showName(1, false);

		gameConsole.setCursorPos(LineOut::grid);
		gameGrid.showReels(1);
		if (win != -1)
			showWin(win, totalWin);
		// Show win details to right of the grid
		gameGrid.showWins(LineOut::phaseName0, LINE_WINS_COLUMN, true, true);
	}

	std::vector<std::string> &getReelList(void) {
		if (this->szReelsetVect.size() == 0) {
			for (int i = 0; i < (int) ReelsIDCount; i++)
				szReelsetVect.push_back(reelsetVect[i].mSzName);
		}
		return this->szReelsetVect;
	}

	/*****************************************/
// Console Playable
/*****************************************/
/*
	bool fullScreenBest = true;

	enum PlayableOptions { none, allGames, bonusGames };;

	int playableMode = PlayableOptions::allGames;
	std::vector<std::string> playableModeOptions = {"All games", "Bonus Games"};

	enum LineOut {
		phaseName0 = 4, phaseName1, phaseName2, reelset, grid,
		result = 13, fsSummary = 17, press = 19
	};

	int getPressToPlayLine(void) {
		return LineOut::press;
	}

	inline bool clearScreen(void) {
		bool doIt = true;
		return doIt;
	}

	/*****************************************/
// Meters
/*****************************************/
/*
uint64_t mTotalWin = 0, mBgWin = 0;
uint64_t mTrailHit = 0, mWinHit = 0, mWinSpinHit = 0;


inline void metersReset(void)
{
	for (auto& event : eventVect)
		event.reset();

	mTotalWin = mBgWin = 0;
	mTrailHit = mWinHit = mWinSpinHit = 0;
}

inline void rtpReport(uint64 plays, uint64 totBet, bool newLine)
{
	printf("%s%10I64d spins. RTP ", newLine ? "\n" : "\r", plays);

	for (auto& event : eventVect)
		event.rtpReport(totBet);
}

void inline finalReport(uint64 plays, uint64 totBet)
{
	printf("\n\nEvent Stats Report\n");
	eventVect[0].finalReportHeader();
	for (auto& event : eventVect)
		event.finalReport(plays, totBet);

	fsStatsReport();

	printf("\n\nMaxWin Report\n");
	eventVect[0].maxWinReportHeader();
	for (auto& event : eventVect)
		event.maxWinReport(plays, totBet);

	printf("\n\nSpins Report\n");
	for (auto& event : eventVect)
		event.spinReport(totBet);
	printf("\n\n");
}

/*****************************************/
// Distributions
/*****************************************/
/*
	void distributionsReset(int rtp, int bet, std::string szMode = "");

	void distributionsSave(void);

	Distribution baseSpinDist = Distribution("BaseSpin");
	Distribution winSpinDist = Distribution("BgWinSpin");

	/**************************************/
// sim mode methods
/**************************************/
/*
int reelTest(int reelset);

int playGame(int force = 0);

/**************************************/
// game phase methods
/**************************************/
/*

int doBaseGame(int force);

int doFreeSpins(void);

void fsStatsReport(void);

/**************************************/
// generic methods
/**************************************/
/*
Console *lpConsole = nullptr;

void setConsole(Console *ref) {
	lpConsole = ref;
}

// if valid option set return string
// else return returns "??"
std::string getSzRtp(void) {
	if (rtpIndex >= 0 && rtpIndex<rtpOptionVect.size())
		return rtpOptionVect[rtpIndex];
	else
		return "??";
}

int getIntRtp(void) const {
	if (rtpIndex >= 0 && rtpIndex<rtpIntVect.size())
		return rtpIntVect[rtpIndex];
	else
		return -1;
}

inline int getRtpIndex(void) const {
	return rtpIndex;
}

// if passed index is valid then sets gameMaths.rtpIndex and returns the associated RTP value e.g. 96
int rtpSetOption(int index);


// return gameName version RN_STREAM_TEST
std::string getGameName(bool addVersion = true) {
	return addVersion ? (gameName + " v" + version) : gameName;
}


/***********************************/
// Pay Lines
/***********************************/
/*
PayLines payLines = PayLines(
	{
		{1, 1, 1, 1, 1},
		{0, 0, 0, 0, 0},
		{2, 2, 2, 2, 2},
		{0, 1, 2, 1, 0},
		{2, 1, 0, 1, 2},
		{1, 0, 1, 0, 1},
		{1, 2, 1, 2, 1},
		{0, 1, 0, 1, 0},
		{2, 1, 2, 1, 2},
		{0, 1, 1, 1, 0},
		{2, 1, 1, 1, 2},
		{1, 0, 0, 0, 1},
		{1, 2, 2, 2, 1},
		{0, 0, 1, 0, 0},
		{2, 2, 1, 2, 2},
		{1, 1, 0, 1, 1},
		{1, 1, 2, 1, 1},
		{2, 0, 0, 0, 2},
		{0, 2, 2, 2, 0},
		{2, 2, 0, 2, 2},
		{0, 0, 2, 0, 0},
		{2, 0, 1, 0, 2},
		{0, 2, 1, 2, 0},
		{2, 0, 2, 0, 2},
		{0, 2, 0, 2, 0},
	}
);


/********************************************/
// Pay Table
//		Order must match symbol definitons
//		Add wilds as an entry
/********************************************/
/*
PayTable stdPayTable = PayTable({
	{0, 0, 0, 25, 100, 1000, 0}, // W1
	{0, 0, 0, 20, 75, 500, 0}, // H1
	{0, 0, 0, 20, 50, 200, 0}, // H2
	{0, 0, 0, 15, 30, 100, 0}, // H3
	{0, 0, 0, 15, 30, 100, 0}, // H4
	{0, 0, 0, 10, 25, 75, 0}, // L1
	{0, 0, 0, 10, 25, 75, 0}, // L2
	{0, 0, 0, 5, 20, 50, 0}, // L3
	{0, 0, 0, 5, 20, 50, 0}, // L4
	{0, 0, 0, 5, 20, 100}, // SC
});


std::vector<ReelSet> reelsetVect = {
	//****************************
	// Base Game
	//****************************
	// MORE FREE SPINS
	{
		"baseReelSet", MAX_REELS, {
			{0, 9, 8, 3, 4, 0, 5, 4, 9, 2, 7, 5, 1, 2, 9, 4, 6, 5, 9, 8, 1, 4, 6, 2, 8, 3, 7, 8},
			{
				8, 9, 3, 2, 8, 6, 0, 3, 9, 1, 2, 6, 8, 7, 3, 6, 1, 7, 9, 6, 3, 4, 1, 3, 2, 8, 3, 4, 8, 5, 8, 4, 6,
				9, 8, 4, 8, 5, 4, 8
			},
			{3, 9, 5, 7, 0, 3, 6, 8, 9, 7, 1, 8, 2, 7, 3, 6, 7, 2, 9, 6, 4, 7, 2, 6, 7, 4, 6, 1, 2, 9, 5, 6, 2, 0},
			{
				7, 9, 3, 5, 9, 1, 5, 7, 9, 8, 0, 5, 8, 2, 8, 3, 6, 1, 9, 5, 3, 6, 8, 2, 8, 6, 3, 4, 8, 2, 6, 8, 4,
				6, 3
			},
			{9, 3, 2, 1, 4, 3, 0, 2, 9, 7, 2, 6, 3, 1, 5, 8, 4, 3, 9, 6, 0, 1, 2, 7, 4, 5, 1, 8, 2, 6, 4, 1},
		}
	},
	{
		"freeSpinsReelSet", MAX_REELS, {
			{0, 5, 8, 3, 4, 0, 5, 4, 3, 2, 7, 5, 1, 2, 9, 4, 6, 5, 3, 8, 1, 4, 6, 2, 8, 3, 7, 8},
			{
				8, 7, 3, 2, 8, 6, 0, 3, 7, 1, 2, 6, 8, 7, 3, 6, 1, 7, 4, 6, 3, 4, 1, 3, 2, 8, 3, 4, 8, 5, 8, 4, 6,
				9, 8, 4, 8, 5, 4, 8
			},
			{3, 7, 5, 7, 0, 3, 6, 8, 2, 7, 1, 8, 2, 7, 3, 6, 7, 2, 1, 6, 4, 7, 2, 6, 7, 4, 6, 1, 2, 9, 5, 6, 2, 0},
			{
				7, 6, 3, 5, 9, 1, 5, 7, 3, 8, 0, 5, 8, 2, 8, 3, 6, 1, 6, 5, 3, 6, 8, 2, 8, 6, 3, 4, 8, 2, 6, 8, 4,
				6, 3
			},
			{9, 3, 2, 1, 4, 3, 0, 2, 1, 7, 2, 6, 3, 1, 5, 8, 4, 3, 2, 6, 0, 1, 2, 7, 4, 5, 1, 8, 2, 6, 4, 1},
		}
	},
};


inline ReelSet *getReelSet(int index) {
	return &(reelsetVect[index]);
}

//********************************************************
// Weight Tables and access methods
//********************************************************

//********************************************************
// Free Spins
//********************************************************

std::vector<WeightTable> fsReelWeightVect = {
	{{WeightData({ReelsIDFreeSpins1, 100})}, "fsStdReels96"}
};

WeightTable fsMultiplierWeights = {
	{
		{2, 100},
		{3, 100},
		{4, 100},
		{5, 100}
	},
	"fsMultiplier"
};


inline int getFsReelsetId(int rtpOption) {
	int rsId = -1;
	rsId = fsReelWeightVect[0].pick();
	this->lpActiveReelSet = &reelsetVect[rsId];
	return rsId;
}

//********************************************************
// Base Game
//********************************************************

std::vector<WeightTable> bgReelWeightVect = {
	{{WeightData({ReelsIDBaseGame1, 100})}, "bgStdReels96"}
};


// pick base gem reels and set this->lpActiveReelSet
// returns reelSetId
int getBgReelsetId(int rtpOption, int bbOption = -1) {
	int rsId = -1;
	if (bbOption == -1) {
		rsId = bgReelWeightVect[0].pick();
	}
	this->lpActiveReelSet = &reelsetVect[rsId];
	return rsId;
}

std::map<int, std::string> symbolTextMap = {
	{-1, "  "},
	{H1, "H1"},
	{H2, "H2"},
	{H3, "H3"},
	{H4, "H4"},
	{L1, "L1"},
	{L2, "L2"},
	{L3, "L3"},
	{L4, "L4"},
	{W1, "W1"},
	{SC, "SC"}
};

std::map<int, int> symbolColourMap = {
	{W1, TEXT_BLACK | BG_YELLOW},
	{SC, TEXT_BLACK | BG_RED}
};
};


*/





















