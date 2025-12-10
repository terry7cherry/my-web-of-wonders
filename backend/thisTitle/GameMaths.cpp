#include "GameMaths.h"

;

void GameMaths::initialise(void)
{
	eventVect.resize(EventIDCount);
	eventVect[EventIDGame] = Event(0, "FullGame", "TOT", true, "KitchenChaos");
	eventVect[EventIDBaseGame] = Event(0, "BaseGame", "BG", true, "BaseGame");
	eventVect[EventIDFreeSpins] = Event(0, "FreeSpins", "FS", true, "FreeSpins");

	eventVect[EventIDBaseGameStd] = Event(2, "bgStd", "", false);

	eventVect[EventIDFreeSpinsStd] = Event(2, "fsStd", "", false);

	// Free Spin sub Events

	if (payLines.checkData(MAX_PAY_LINES, MAX_REELS))
		reportError("GameMaths::initialise(void)", "payLines.checkData() - FAILED");
	gameGrid.setPayLines(payLines);
	gameGrid.setPayTable(stdPayTable);
	gameGrid.setTextMap(symbolTextMap);
	gameGrid.setColourMap(symbolColourMap);
	gameGrid.setConsole(&gameConsole);

	// add symbol Data to the grid
	gameGrid.paySymVect = {
		H1,H2,H3,H4,
		L1,L2,L3,L4
	};
	gameGrid.scatterSymVect = { SC };
	gameGrid.wildSymVect = { W1 };
	gameGrid.symbolsInit(SYM_COUNT);
}

// set new distribution titles
// reset any stats variables1
void GameMaths::reset(int gameBet, std::string szMode)
{
	bet = gameBet;
	betMultiplier = bet / minBet;
	distributionsReset(getIntRtp(), gameBet, szMode);
	this->fsTrigger = false;

	metersReset();
	events.reserve(512);
}

// if passed index is valid then sets gameMaths.rtpIndex and returns the associated RTP value e.g. 96
// returns 0 if index is not valid
int GameMaths::rtpSetOption(int index)
{
	std::string szErrDetail = "";
	if (index >= rtpOptionVect.size())
		szErrDetail = "Passed index #" + std::to_string(index) +
		" >= rtpOptionVect.size() #" + std::to_string(rtpOptionVect.size());
	else if (index < 0)
		szErrDetail = "Passed index #" + std::to_string(index) + "< 0";
	if (szErrDetail.size())
	{
		reportError("GameMaths::rtpSetOption(int index)", szErrDetail);
		return 0;
	}
	rtpIndex = index;
	return rtpIntVect[rtpIndex];
}

void GameMaths::distributionsReset(int rtp, int bet, std::string szMode)
{
	for (auto& event : eventVect) {
		if (event.lpWinDist)
			event.lpWinDist->reset(rtp, bet, szMode);
	}

	baseSpinDist.reset(rtp, bet, szMode);
	winSpinDist.reset(rtp, bet, szMode);
}

void GameMaths::distributionsSave(void)
{
	for (auto& event : eventVect) {
		if (event.lpWinDist)
			event.lpWinDist->fileOutput(true);
	}
	baseSpinDist.fileOutput(true);
	winSpinDist.fileOutput(true);
}

int GameMaths::playGame(int force)
{
	events.clear();
	bool doSpin = true;
	Event& gameEvent = eventVect[EventIDGame];
	gameEvent.addHit();

	int win = doBaseGame(force);
	if (DO_FREE_SPINS && this->fsTrigger)
		win += this->doFreeSpins();
	gameEvent.addWin(win);
	return win;
}

int GameMaths::reelTest(int reelset) {
	Event& gameEvent = eventVect[EventIDGame];
	gameEvent.addHit();

	this->lpActiveReelSet = &(this->reelsetVect[reelset]);
	this->lpActiveReelSet->spin();				// random spin
	gameGrid.reset();							// clear grid and apply reel spin 
	gameGrid.setSymbolsFromReels(this->lpActiveReelSet);

	int spinWin = gameGrid.evaluateWins(betMultiplier, PAYTABLE_METERS);

	if (gameGrid.getSymbolCount(SC) >= 3)
		eventVect[EventIDFreeSpins].addHit();

	gameEvent.addWin(spinWin);
	return spinWin;
}






