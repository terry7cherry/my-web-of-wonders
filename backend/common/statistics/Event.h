#pragma once

// system includes
#include <string>
#include <cstdint>
#include <cstdio>
#include <map>
#include <iostream>

#include "MeterMap.h"
#include "Distribution.h"

// game globals (declared elsewhere)
extern Console gameConsole;

class Event
{
public:
    std::string szName = "";
    int nameLevel = 0;
    std::string szShort = "";

    bool doRtpReport = false;
    bool isActive = false;
    bool doMaxWin = false;
    Distribution* lpWinDist = nullptr;

    // Meters
    uint64_t hitCount = 0;
    uint64_t winCount = 0;
    uint64_t maxWinCount = 0;
    uint64_t totalWin = 0;
    uint64_t maxWin = 0;
    MeterMap initialSpinsMeter = MeterMap("InitialSpins");
    MeterMap finalSpinsMeter = MeterMap("FinalSpins");
    MeterMap initialSpinsWinsMeter = MeterMap("InitialSpinsWins");
    MeterMap finalSpinsWinsMeter = MeterMap("FinalSpinsWins");

    // local copy can be used for passing result to previous events
    int lastWin = 0;
    int initialSpins = 0, finalSpins = 0;

    // dummy constructor for vector initialisation
    Event() {}

    // level            : Used in playable to set row offset of event name
    // name             : Used in stats finalReport and playable
    // rtpReportName    : pass a string if this event to be shown in rtpReport
    // doMaxWin         : include event in maxWinReport
    // distName         :
    Event(int level, std::string name, std::string rtpReportName, bool doMaxWin, std::string distName = "")
    {
        nameLevel = level;
        szName = std::move(name);
        szShort = std::move(rtpReportName);
        if (szShort.size())
            doRtpReport = true;
        this->doMaxWin = doMaxWin;
        if (distName.size())
            lpWinDist = new Distribution(distName);
    }

    inline void reset(void)
    {
        hitCount = 0;
        winCount = 0;
        maxWinCount = 0;
        maxWin = 0;
        totalWin = 0;
        lastWin = 0;
        initialSpins = finalSpins = 0;
        initialSpinsMeter.reset();
        finalSpinsMeter.reset();
        initialSpinsWinsMeter.reset();
        finalSpinsWinsMeter.reset();
    }

    // Show event name offset from (baseRow, 0) by (event.nameLevel, event.nameLevel*2)
    // Optional inverse flash and clearline
    void showName(int baseRow, int flashCount = 0, bool clearLine = true)
    {
        int row = baseRow + nameLevel;
        int col = nameLevel * 2;

        if (clearLine)
            gameConsole.clearLine(row);
        if (flashCount)
            gameConsole.flashText(szName, row, col, 100, flashCount, true);
        else
        {
            gameConsole.setCursorPos(row, col);
            gameConsole.coutStyle(szName, CONSOLE_INVERT);
        }
    }

    // Show event name offset from (baseRow, 0) by (event.nameLevel, event.nameLevel*2)
    // Add szExtra after the name
    // Optional inverse flash and clearline
    void showName(int baseRow, std::string szExtra, bool clearLine = true)
    {
        showName(baseRow, 0, clearLine);
        std::cout << " " << szExtra;
    }

    // Call gameConsole.clearLine(baseRow + event.nameLevel)
    void clearName(int baseRow)
    {
        gameConsole.clearLine(baseRow + nameLevel);
    }

    // Show event name offset from (baseRow, 0) by (event.nameLevel, event.nameLevel*2)
    // Follow on line with, ": " + szSpin + " " + to_string(spin) + " of " + to_string(spinLimit) 
    void showNameSpins(int baseRow, std::string szSpin, int spin, int spinLimit, std::string szExtra = "")
    {
        showName(baseRow);
        std::string szSpinDetail = ": ";
        szSpinDetail += (szSpin + " " + std::to_string(spin) + " of " + std::to_string(spinLimit));
        if (szExtra.size())
            szSpinDetail += (" " + szExtra);
        std::cout << szSpinDetail;
    }

    inline void addHit(void)
    {
        lastWin = 0;
        hitCount++;
    }

    inline void addWin(int winVal)
    {
        if (lastWin = winVal)
        {
            totalWin += static_cast<uint64_t>(winVal);
            winCount++;
            if (doMaxWin)
            {
                if (static_cast<uint64_t>(winVal) > maxWin)
                {
                    maxWin = static_cast<uint64_t>(winVal);
                    maxWinCount = 1;
                }
                else if (static_cast<uint64_t>(winVal) == maxWin)
                    maxWinCount++;
            }
        }
        if (lpWinDist)
            lpWinDist->add(winVal);
    }

    inline int getLastWin(void) {
        return lastWin;
    }

    inline void setInitialSpins(int spinCount) {
        initialSpins = finalSpins = spinCount;
    }

    inline void addSpin(void) {
        finalSpins++;
    }

    // 
    inline void setFinalSpins(int spinCount, bool record = true) {
        finalSpins = spinCount;
        if (record)
            recordSpins();
    }

    inline void recordSpins(void) {
        initialSpinsMeter.add(initialSpins);
        initialSpinsWinsMeter.add(initialSpins, lastWin);
        finalSpinsMeter.add(finalSpins);
        finalSpinsWinsMeter.add(finalSpins, lastWin);
    }

    // Called mid simulation to show event rtp
    inline void rtpReport(uint64_t totBet)
    {
        if (doRtpReport)
            std::printf(", %s=%6.2f%%", szShort.c_str(), (float)totalWin * 100 / totBet);
    }

    //  Show header to screen: "Event", "RTP", "HitRate", "AvgWin", "WinChance", "WinRate"
    void finalReportHeader()
    {
        std::printf("%20s,%12s,%12s,%12s,%12s,%12s,%12s\n",
            "Event", "RTP", "HitRate", "AvgPay", "AvgWin", "WinChance", "WinRate");
    }

    void finalReport(uint64_t plays, uint64_t totBet)
    {
        uint64_t bet = totBet / plays;
        std::printf("%20s,", szName.c_str());                                  // Event
        std::printf("%11.2f%%,", (double)totalWin * 100.0 / totBet);                // RTP
        std::printf("%12.2f,", (double)plays / (hitCount ? hitCount : 1));       // HitRate
        std::printf("%12.2f,", hitCount ? (double)totalWin / hitCount / bet : 0);  // AvgPay
        std::printf("%12.2f,", winCount ? (double)totalWin / winCount / bet : 0);  // AvgWin
        std::printf("%11.2f%%,", hitCount ? (double)winCount * 100.0 / hitCount : 0); // WinChance
        std::printf("%12.2f\n", winCount ? (double)hitCount / winCount : 0);      // WinRate
    }

    //  Show header to screen: "Event", "MaxWin", "HitRate", "EventRate"
    void maxWinReportHeader(void)
    {
        std::printf("%20s,%12s,%12s,%12s\n",
            "Event", "MaxWin", "HitRate", "EventRate");
    }

    //  Show stats to screen: "Event", "MaxWin", "HitRate", "EventRate"
    void maxWinReport(uint64_t plays, uint64_t totBet)
    {
        if (!doMaxWin) return;

        uint64_t bet = totBet / plays;
        std::printf("%20s,", szName.c_str()); // Event
        std::printf("%12llu,", static_cast<unsigned long long>(maxWin / bet)); // MaxWin
        std::printf("%12llu,", maxWinCount ? static_cast<unsigned long long>(plays / maxWinCount) : 0ULL); // HitRate
        std::printf("%12llu\n", maxWinCount ? static_cast<unsigned long long>(hitCount / maxWinCount) : 0ULL); // EventRate
    }

    //  Show stats to screen: "Event", "InitilSpins", "FinalSpins"
    void spinReport(uint64_t totBet)
    {
        if (initialSpinsMeter.intMap.size() || finalSpinsMeter.intMap.size())
        {
            std::printf("\nEvent: %s\n", szName.c_str());
            if (initialSpinsMeter.intMap.size())
                hitAndWinReport(initialSpinsMeter, initialSpinsWinsMeter, totBet);
            std::printf("\n");
            if (finalSpinsMeter.intMap.size())
                hitAndWinReport(finalSpinsMeter, finalSpinsWinsMeter, totBet);
        }
    }

    void hitReport(MeterMap& meterMap, bool showSplits = true)
    {
        std::map<int, uint64_t>::iterator it;
        int64 mapTotal = 0;
        for (it = meterMap.intMap.begin(); showSplits && it != meterMap.intMap.end(); ++it)
            mapTotal += static_cast<int64>(it->second);

        if (showSplits)
        {
            std::printf("%20s, %10s, %10s\n", meterMap.szName.c_str(), "Hits", "Splits");
            for (it = meterMap.intMap.begin(); it != meterMap.intMap.end(); ++it)
            {
                std::printf("%20d, %10llu, %9.2f%%\n",
                    it->first,
                    static_cast<unsigned long long>(it->second),
                    mapTotal ? (double)it->second * 100.0 / mapTotal : 0.0);
            }
        }
        else
        {
            std::printf("%20s, %10s\n", meterMap.szName.c_str(), "Hits");
            for (it = meterMap.intMap.begin(); it != meterMap.intMap.end(); ++it)
            {
                std::printf("%20d, %10llu\n",
                    it->first,
                    static_cast<unsigned long long>(it->second));
            }
        }
    }

    void hitAndWinReport(MeterMap& hitMap, MeterMap& winMap, uint64_t totBet, bool showSplits = true)
    {
        std::map<int, uint64_t>::iterator itHits, itWins;
        int64 mapTotal = 0;

        if (showSplits)
        {
            for (itHits = hitMap.intMap.begin(); showSplits && itHits != hitMap.intMap.end(); ++itHits)
                mapTotal += static_cast<int64>(itHits->second);

            std::printf("%20s, %10s, %10s, %10s, %10s\n",
                hitMap.szName.c_str(), "Hits", "Splits", "Wins", "RTP");
            for (itHits = hitMap.intMap.begin(), itWins = winMap.intMap.begin();
                itHits != hitMap.intMap.end() && itWins != winMap.intMap.end();
                ++itHits, ++itWins)
            {
                std::printf("%20d, %10llu, %9.2f%%, %10llu, %9.2f%%\n",
                    itHits->first,
                    static_cast<unsigned long long>(itHits->second),
                    mapTotal ? (double)itHits->second * 100.0 / mapTotal : 0.0,
                    static_cast<unsigned long long>(itWins->second),
                    totBet ? (double)itWins->second * 100.0 / totBet : 0.0);
            }
        }
        else
        {
            std::printf("%20s, %10s, %10s, %10s\n",
                hitMap.szName.c_str(), "Hits", "Wins", "RTP");
            for (itHits = hitMap.intMap.begin(), itWins = winMap.intMap.begin();
                itHits != hitMap.intMap.end() && itWins != winMap.intMap.end();
                ++itHits, ++itWins)
            {
                std::printf("%20d, %10llu, %10llu, %9.2f%%\n",
                    itHits->first,
                    static_cast<unsigned long long>(itHits->second),
                    static_cast<unsigned long long>(itWins->second),
                    totBet ? (double)itWins->second * 100.0 / totBet : 0.0);
            }
        }
    }
};

