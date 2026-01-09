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
    std::vector<std::vector<unsigned int>> payData; // copy of passed pay data
    unsigned int numSymbols;
    std::vector<unsigned int> minPaySymbolsVect;
    std::vector<uint8_t> paySymVect; // symbols that have a payout
    std::vector<uint8_t> scatterSymVect; // symbols that are scatters
    std::vector<uint8_t> wildSymVect; // symbols that are wild
    std::vector<uint8_t> wildNoSubstVect; // symbols that wilds do not match
    std::vector<SymbolData> symbolVect;
    uint32 wildMask = 0;
    unsigned int minWinSymbols = 0u;
    size_t maxWinSymbols = 0;

    //std::vector<MeterMap*> symbolWinMeterVect;
    //std::vector<MeterMap*> symbolHitMeterVect;


    PayTable(
        const std::vector<std::vector<unsigned int>>& payData,
        const std::vector<uint8_t>& paySymVect,
        const std::vector<uint8_t>& scatterSymVect,
        const std::vector<uint8_t>& wildSymVect,
        const std::vector<uint8_t>& wildNoSubstVect);

    ~PayTable() = default;

    void symbolsInit(unsigned int symbolCount);

    unsigned int getWin(uint8_t symbolIndex, unsigned int symbolCount, bool checkLimits = false) const;

    bool isWild(const uint8_t symbolId) const
    {
        return symbolVect[symbolId].isWild;
    }

    bool wildSubstitutes(const uint8_t symbolId) const
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

