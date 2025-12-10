#pragma once

// cpp-dev-library
#include "../headers/pch.h"
#include "../statistics/MeterMap.h"
#include "../statistics/MeterFile.h"

#include <cstdio>   // for std::snprintf

class PayTable
{
public:
    // Data
    std::vector<std::vector<int>> payData;  // copy of passed pay data
    std::vector<int> minPaySymbolsVect;
    int minWinSymbols = -1;
    int maxWinSymbols = 0;

    std::vector<MeterMap*> symbolWinMeterVect;
    std::vector<MeterMap*> symbolHitMeterVect;

    PayTable(std::vector<std::vector<int>> data)
    {
        payData = std::move(data);
        minPaySymbolsVect.resize(payData.size(), -1);

        maxWinSymbols = static_cast<int>(payData[0].size()) - 1;
        minWinSymbols = -1;
        int iSym = 0;

        for (const auto& symbolPayData : payData)
        {
            for (int i = 0; i < static_cast<int>(symbolPayData.size()); i++)
            {   // find min pay count for this symbol
                if (symbolPayData[i] > 0)
                {
                    minPaySymbolsVect[iSym] = i;
                    break;
                }
            }

            if (minPaySymbolsVect[iSym] != -1 &&
                (minWinSymbols == -1 || minPaySymbolsVect[iSym] < minWinSymbols))
            {   // got winning symbols and they are the lowest seen so far
                minWinSymbols = minPaySymbolsVect[iSym];
            }
            iSym++;
        }
    }

    ~PayTable() = default;

    int getWin(int symbolIndex, int symbolCount, bool checkLimits = false)
    {
        if (checkLimits)
        {
            if (symbolIndex < 0 || symbolIndex >= static_cast<int>(payData.size()))
                std::printf("\r\nError: PayTable::getWin() - symbolIndex (%d) out of bounds", symbolIndex);

            if (symbolCount < minPaySymbolsVect[symbolIndex])
                std::printf("\r\nError: PayTable::getWin() - symbolCount (%d) has no payout", symbolCount);
            else if (symbolCount > maxWinSymbols)
                std::printf("\r\nError: PayTable::getWin() - symbolCount (%d) > maxWinSymbols (%d)",
                    symbolCount, maxWinSymbols);
        }
        return payData[symbolIndex][symbolCount];
    }

    /*
    // Add meters to record win values and hit counts for symbol type and count
    void addMeters(MeterFile* lpFile, std::string szTableName) {
        addMeters(lpFile, szTableName.c_str());
    }

    // Add meters to record win values and hit counts for symbol type and count
    void addMeters(MeterFile* lpFile, const char* szTableName)
    {
        addWinMeters(lpFile, szTableName);

        for (int i = 0; i < static_cast<int>(payData.size()); i++)
        {
            char* buf = new char[METER_TEXT_LEN];
            if (szTableName)
                std::snprintf(buf, METER_TEXT_LEN, "%s SYM%d HITS", szTableName, i);
            else
                std::snprintf(buf, METER_TEXT_LEN, "SYM%d HITS", i);

            symbolHitMeterVect.push_back(new MeterMap(buf));
            lpFile->addMeters(symbolHitMeterVect.back());
        }
    }

    // Add meters to record win values by symbol type and count
    void addWinMeters(MeterFile* lpFile, std::string szTableName) {
        addWinMeters(lpFile, szTableName.c_str());
    }

    // Add meters to record win values by symbol type and count
    void addWinMeters(MeterFile* lpFile, const char* szTableName)
    {
        for (int i = 0; i < static_cast<int>(payData.size()); i++)
        {
            char* buf = new char[METER_TEXT_LEN];
            if (szTableName)
                std::snprintf(buf, METER_TEXT_LEN, "%s SYM%d WINS", szTableName, i);
            else
                std::snprintf(buf, METER_TEXT_LEN, "SYM%d WINS", i);

            symbolWinMeterVect.push_back(new MeterMap(buf));
            lpFile->addMeters(symbolWinMeterVect.back());
        }
    }

    // Record the the win value for the oak of symbolIndex 
    void meterWin(int symbolIndex, int oak, int ways = 1)
    {
        if (symbolIndex < static_cast<int>(symbolWinMeterVect.size()))
            symbolWinMeterVect[symbolIndex]->add(oak, payData[symbolIndex][oak] * ways);
        if (symbolIndex < static_cast<int>(symbolHitMeterVect.size()))
            symbolHitMeterVect[symbolIndex]->add(oak, ways);
    }
    */
};

