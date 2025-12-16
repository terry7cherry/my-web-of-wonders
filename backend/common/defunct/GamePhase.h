#pragma once

// cpp-dev-library
#include "../../pch.h"
#include "../statistics/MeterMap.h"

class GamePhase {
private:
    bool reportedNoWinDistribution = false;

public:
    std::string szPhaseName = "";
    int id = -1;

    std::map<int, std::vector<int> > scatterPayMap;

    std::vector<int> triggerSpinVect = {1}; // default 1 spin 
    std::vector<int> extraSpinVect;


    // Game components
    int reelsetIndex = -1;
    WeightTable *lpReelsPicker = nullptr;

    /***************************/
    // constructors
    /***************************/
    GamePhase() {
    }

    inline GamePhase(int phaseId, std::string name, bool addWinDist = true) {
        id = phaseId;
        szPhaseName = name;
        if (addWinDist)
            addDistribution();
    }

    /***************************/
    // Meter setup
    /***************************/
    bool hasMeters = false;
    int meterIndex = -1;
    std::string szMetersName = "";
    std::vector<int> meterSplitVect;

    inline void addMeters(string name, std::vector<int> splitVect = {}) {
        hasMeters = true;
        szMetersName = name;
        if (splitVect.size())
            meterSplitVect = splitVect;
    }

    /***************************/
    // Win Distribution
    /***************************/
    Distribution *lpWinDist = nullptr;

    inline void addDistribution(void) {
        lpWinDist = new Distribution(szPhaseName);
    }

    // Create fullFilename suitable for tableau, "baseFileName"+"szMode"+"_rtp_bet" 
    inline void initDistribution(int rtp, int bet, std::string szMode = "") {
        lpWinDist->reset(rtp, bet, szMode);
    }

    // Create fullFilename suitable for tableau, "baseFileName"+"szMode"+"_rtp_bet" 
    // Check file can be opened
    inline void initDistribution(std::string szRtp, std::string szBet, std::string szMode) {
        lpWinDist->reset(szRtp, szBet, szMode);
    }


    inline void addWinToDistribution(int win) {
        if (lpWinDist)
            lpWinDist->add(win);
#if _DEBUG
		else if (!reportedNoWinDistribution) {
            reportError("GamePhase.addWinToDistribution",
                        "Phase '" + szPhaseName + "': winDist not set", "WARNING");
            reportedNoWinDistribution = true;
        }
#endif
    }

    /***************************/
    // Spins Meters
    /***************************/
    MeterMap *lpSpinsMeters = nullptr;

    inline void addSpinMeters(std::string szMeter = "SpinCount") {
        std::string szName = szPhaseName + " " + szMeter;
        lpSpinsMeters = new MeterMap(szName);
    }

    /***************************/
    // Spin setup
    /***************************/
    inline void setTriggerSpinOptions(std::vector<int> options) {
        if (options.size())
            triggerSpinVect = options;
    }

    inline void setExtraSpinOptions(std::vector<int> options) {
        if (options.size())
            extraSpinVect = options;
    }


    /***************************/
    // Scatter Wins
    /***************************/

    // pass the winVect associated with symbol
    // winVect contains bet multiples 
    inline void addScatterWin(int symbol, std::vector<int> winVect) {
        auto result = scatterPayMap.emplace(symbol, winVect);

#if _DEBUG
        if (result.second == false)
            reportError("GamePhase.addScatterWin", "Symbol " + to_string(symbol) + " already in \"scatterPayMap\"");
#endif
    }

    inline int getScatterWin(int symbol, int count) {
        int win = 0;
        if (scatterPayMap.size()) {
            auto it = scatterPayMap.find(symbol);
            if (it != scatterPayMap.end()) {
                if (count < it->second.size())
                    win = it->second[count];
            }
#if _DEBUG
			else
            reportError("GamePhase.getScatterWin", "Symbol " + to_string(symbol) + " not in \"scatterPayMap\"");
#endif
        }
#if _DEBUG
		else
        reportError("GamePhase.getScatterWin", "No entries in \"scatterPayMap\"");
#endif
        return win;
    }

    /***************************/
    // Play configutraton
    /***************************/


    /***************************/
    // Play configutraton
    /***************************/
    inline void setReelset(int rsIndex) { reelsetIndex = rsIndex; }

    inline void setReelsetPicker(std::vector<WeightData> weightVector) {
        lpReelsPicker = new WeightTable(weightVector);
    }

    /***************************/
    // Play actions
    /***************************/
    inline int getReelsetIndex(void) {
        if (lpReelsPicker)
            reelsetIndex = lpReelsPicker->pick();
        else {
            if (reelsetIndex == -1)
                reportError("GamePhase.getReelsetIndex", "reelsetIndex not set");
            return reelsetIndex;
        }
    }
};