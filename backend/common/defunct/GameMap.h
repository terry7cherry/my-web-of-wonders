#pragma once
#include <map>

// cpp-dev-library
#include "../headers/pch.h"
#include "../../utils.h"
#include "GamePhase.h"

class GameMap
{
public:
	std::map<int, GamePhase> phaseMap;
	std::map<int, GamePhase>::iterator itActive = phaseMap.end();
	std::map<int, GamePhase>::iterator itNext = phaseMap.end();

	GamePhase *lpActivePhase=nullptr;
	GamePhase nextPhase;

	GameMap(void)
	{
	}

	GamePhase* addPhase(GamePhase newPhase)
	{
		if (phaseMap.find(newPhase.id) == phaseMap.end())
		{				// phase with this id does not exist
			phaseMap[newPhase.id] = newPhase;
			return &phaseMap[newPhase.id];
		}
		else
		{
			reportError("GamePhase.addPhase", "Phase already registered with id " + newPhase.id);
			return nullptr;
		}
	}

	/********************************/
	// Distribution processing
	/********************************/

	// Output all active win distributions, hi first
	inline void saveDistributions(void)
	{
		itNext = phaseMap.begin();
		do {
			if (itNext->second.lpWinDist)
				itNext->second.lpWinDist->fileOutput(true);
		} while (++itNext != phaseMap.end());
	}

	inline void resetDistributions(int rtp, int bet, std::string szMode = "")
	{
		itNext = phaseMap.begin();
		do {
			if (itNext->second.lpWinDist)
				itNext->second.lpWinDist->reset(rtp, bet, szMode);
		} while (++itNext != phaseMap.end());

	}

	// Finds the passed gpid in phaseMap and sets the associated GamePhase as the active one
	// Returns a pointer to the active GamePhase 
	GamePhase * setActive(int gpid)
	{
		itActive = phaseMap.find(gpid);
#if _DEBUG	
		if (itActive == phaseMap.end())
			reportError("GamePhase.setActive", "Passed GamePhaseId is not in the map");
#endif
		return &(itActive->second);
	}

	// Returns a pointer to the active GamePhase 
	GamePhase* getActive(void)
	{
#if _DEBUG	
		if (itActive == phaseMap.end())
			reportError("GamePhase.getActive", "Active phase is not set");
#endif
		return &(itActive->second);
	}


	GamePhase* setNext(int gpid)
	{
		std::map<int, GamePhase>::iterator it = phaseMap.find(gpid);

		if (it != phaseMap.end())
			itNext = it;
		else
			reportError("GamePhase.setNext", "Passed GamePhaseId is not in the map");
		return &(itNext->second);
	}

	GamePhase* setNextToActive(void)
	{
		if (itNext != phaseMap.end())
		{
			itActive = itNext;
			itNext = phaseMap.end();
		}
	}

	GamePhase* getNext(void)
	{
		if (itNext == phaseMap.end())
			reportError("GamePhase.getNext", "Next phase is not set");
		return &(itNext->second);
	}

	GamePhase* find(int id)
	{
		std::map<int, GamePhase>::iterator it = phaseMap.find(id);
		if (it == phaseMap.end())
		{
			reportError("GamePhase.find()", "Phase not found");
			return nullptr;
		}
		return &(it->second);
	}

	void startGame(void)
	{
		itActive = itNext = phaseMap.end();
	}

	int size(void) { return (int)phaseMap.size(); }
	bool exists(void) { return phaseMap.size() != 0; }

};

