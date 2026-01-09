#include "__GameMaths.h"
#include "../events/FreeSpinsTriggerEvent.h"
#include "../events/FreeSpinsRetriggerEvent.h"
#include "../events/NewFreeSpinEvent.h"
#include "../events/ReelSpinEvent.h"
#include "../events/GridUpdateEvent.h"
#include "../events/WinAnalysisEvent.h"
#include "../events/FreeSpinsCompleteEvent.h"

/*
;

static MeterMap fsMultiplierMap("fsFinalMultiplier");
static MeterMap fsAnyEventMap("fsAnyEvent");

int __GameMaths::doFreeSpins(void) {
	Event &fsEvent = eventVect[EventIDFreeSpins];

	fsEvent.addHit();
	int initialSpins = this->fsStartSpins[this->scatterCount];
	fsEvent.setInitialSpins(initialSpins);
	events.push_back(std::make_unique<FreeSpinsTriggerEvent>(initialSpins));
#if PLAY_MODE
	if (playableMode == PlayableOptions::allGames) {
		fsEvent.showName(LineOut::phaseName0, FEATURE_FLASH_COUNT);
		gameConsole.waitAnyKeyText("TO PLAY FREE SPINS", LINE_WINS_COLUMN, LINE_OUT::press);
	}
#endif

	this->fsActive = true;
	this->fsTotalWin = 0;
	this->fsTotalSpins = initialSpins;
	for (this->fsSpin = 0; this->fsSpin < this->fsTotalSpins; this->fsSpin++) {
		int fsMultiplier = this->fsMultiplierWeights.pick();
		events.push_back(std::make_unique<NewFreeSpinEvent>(fsMultiplier));
		getFsReelsetId(rtpIndex);
		this->lpActiveReelSet->spin(); // random spin
		gameGrid.reset(); // clear grid and apply reel spin
		gameGrid.setSymbolsFromReels(this->lpActiveReelSet);
		events.push_back(std::make_unique<ReelSpinEvent>(gameGrid));

		unsigned int numWildReelsExpanded = 0;
		for (size_t reelNum = 0; reelNum < gameGrid.maxReels; ++reelNum) {
			if (gameGrid.getReelSymbolCount(reelNum, 0)) {
				numWildReelsExpanded++;
				gameGrid.setReelSymbols(reelNum, 0);
			}
		}
		if (numWildReelsExpanded) {
			const std::string message = numWildReelsExpanded > 1 ? "Wilds expanded!" : "Wild expanded!";
			events.push_back(std::make_unique<GridUpdateEvent>(gameGrid, message));
		}


#if PLAY_MODE
		if (this->fsSpin)
			gameConsole.waitAnyKeyText("FOR NEXT FREE SPIN", LINE_WINS_COLUMN, LINE_OUT::press);
		fsEvent.showNameSpins(LineOut::phaseName0, "Spin", this->fsSpin + 1, this->fsTotalSpins,
		                      "Multiplier = x" + to_string(gameGrid.gridMultiplier));
#endif

		int spinWin = 0, finalWin = 0;

		gameGrid.gridMultiplier = fsMultiplier;
		spinWin = gameGrid.evaluateWins(betMultiplier, PAYTABLE_METERS);
		if (spinWin) {
			if (gameGrid.gridMultiplier > 1)
				finalWin = gameGrid.applyGridMultiplier(MultipliersCombineType::add);
			else
				finalWin = spinWin;
			this->fsTotalWin += (finalWin);
		}
		events.push_back(std::make_unique<WinAnalysisEvent>(gameGrid, "free spin"));

		this->scatterCount = gameGrid.getSymbolCount(SC);
		int retriggerSpins = this->fsRetriggerSpins[scatterCount];
		this->fsTotalSpins += retriggerSpins;
		if (retriggerSpins > 0) events.push_back(std::make_unique<FreeSpinsRetriggerEvent>(retriggerSpins));

		eventVect[EventIDFreeSpinsStd].addHit();
		eventVect[EventIDFreeSpinsStd].addWin(finalWin);

#if PLAY_MODE
		this->showWin(spinWin, finalWin);
		gameGrid.showWins(LineOut::phaseName0, LINE_WINS_COLUMN, true, true);
#endif

		fsEvent.addSpin();
	}
	fsEvent.setFinalSpins(this->fsSpin);
	fsEvent.addWin(this->fsTotalWin);
	events.push_back(std::make_unique<FreeSpinsEndEvent>(fsTotalWin));
	fsActive = false;

	gameGrid.gridMultiplier = 0;
	return this->fsTotalWin;
}

void __GameMaths::fsStatsReport(void) {
	std::cout << "\n\nFree Spins Report\n";
	fsMultiplierMap.consoleOutH(true);
	fsAnyEventMap.consoleOutH(true);
}
*/
