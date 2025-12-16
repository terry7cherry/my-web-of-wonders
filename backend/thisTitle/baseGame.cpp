#include "__GameMaths.h"
#include "../common/events/ReelSpinEvent.h"
#include "../common/events/WinAnalysisEvent.h"
/*
;

static MeterMap fsMultiplierMap("fsFinalMultiplier");
static MeterMap fsWildEventsMap("fsWildEvents");

int __GameMaths::doBaseGame(int force)
{
	int totalWin = 0, spinWin = 0;

	Event& bgEvent = eventVect[EventIDBaseGame];

	bgEvent.addHit();

	gameGrid.gridMultiplier = 1;
	if (force == 1)
	{			// force free spins
		this->lpActiveReelSet = &(this->reelsetVect[ReelsIDBaseGame1]);
		this->lpActiveReelSet->spin();		// random spin before forcing
		this->lpActiveReelSet->stepUpToSymbol(0, SC);
		this->lpActiveReelSet->stepUpToSymbol(2, SC);
		this->lpActiveReelSet->stepUpToSymbol(4, SC);
	}
	else
	{			// random play - chose reels and spin them
		getBgReelsetId(rtpIndex);
		this->lpActiveReelSet->spin();				// random spin
	}
	gameGrid.reset();								// clear grid and apply reel spin 
	gameGrid.setSymbolsFromReels(this->lpActiveReelSet);
	events.push_back(std::make_unique<ReelSpinEvent>(gameGrid));

#if PLAY_MODE
	bgEvent.showName(LineOut::phaseName0, "", true);
#endif	

	int wildCount = gameGrid.getSymbolCount(W1);
	this->scatterCount = gameGrid.getSymbolCount(SC);


	if (scatterCount >= 3) this->fsTrigger = true;

	spinWin = gameGrid.evaluateWins(betMultiplier, PAYTABLE_METERS);
	events.push_back(std::make_unique<WinAnalysisEvent>(gameGrid, "base game"));

	totalWin += spinWin;

	bgEvent.addWin(totalWin);
	return totalWin;
}


*/
