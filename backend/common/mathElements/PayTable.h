#pragma once

// cpp-dev-library
#include "../statistics/MeterFile.h"


/************************************/
// Symbols
/************************************/
typedef struct
{
    int index;
    bool isWild;
    int wildIndex;
    bool substWild;

    bool isScatter;
    int scatterIndex;

    bool isOverlay;
    int overlayIndex;

    uint32 mask;
    uint32 payMask;
} SymbolData;

class PayTable
{
public:
    // Data
    std::vector<std::vector<size_t>> payData; // copy of passed pay data
    size_t numSymbols;
    std::vector<int> minPaySymbolsVect;
    std::vector<size_t> paySymVect; // symbols that have a payout
    std::vector<size_t> scatterSymVect; // symbols that are scatters
    std::vector<size_t> wildSymVect; // symbols that are wild
    std::vector<size_t> wildNoSubstVect; // symbols that wilds do not match
    std::vector<SymbolData> symbolVect;
    uint32 wildMask = 0;
    int minWinSymbols = -1;
    size_t maxWinSymbols = 0;

    //std::vector<MeterMap*> symbolWinMeterVect;
    //std::vector<MeterMap*> symbolHitMeterVect;


    PayTable(
        const std::vector<std::vector<size_t>>& payData,
        const std::vector<size_t>& paySymVect,
        const std::vector<size_t>& scatterSymVect,
        const std::vector<size_t>& wildSymVect,
        const std::vector<size_t>& wildNoSubstVect);

    ~PayTable() = default;

    void symbolsInit(int symbolCount);

    size_t getWin(size_t symbolIndex, size_t symbolCount, bool checkLimits = false) const;

    bool isWild(const int symbolId) const
    {
        return symbolVect[symbolId].isWild;
    }

    bool wildSubstitutes(const int symbolId) const
    {
        return symbolVect[symbolId].substWild;
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

