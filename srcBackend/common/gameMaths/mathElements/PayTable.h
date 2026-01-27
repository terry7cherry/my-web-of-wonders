#pragma once
/**
 * @file PayTable.h
 * @brief Symbol properties and payout lookup table.
 */

#include <absl/container/flat_hash_map.h>
//#include "../../statistics/MeterFile.h"


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
    std::string name;
} SymbolData;

/**
 * @brief Holds symbol metadata and pay values; provides win lookup helpers.
 */
class PayTable
{
public:
    // Data
    std::vector<std::vector<unsigned int>> payData; // copy of passed pay data
    unsigned int numSymbols;
    std::vector<unsigned int> minPaySymbolsVect;
    std::vector<Symbol> paySymVect; // symbols that have a payout
    std::vector<Symbol> scatterSymVect; // symbols that are scatters
    std::vector<Symbol> wildSymVect; // symbols that are wild
    std::vector<Symbol> wildNoSubstVect; // symbols that wilds do not match
    std::vector<SymbolData> symbolVect;
    uint32 wildMask = 0;
    unsigned int minWinSymbols = 0u;
    size_t maxWinSymbols = 0;
    std::vector<std::string> symbolNames;
    std::unordered_map<std::string, Symbol> symbolIndexMap;

    //std::vector<MeterMap*> symbolWinMeterVect;
    //std::vector<MeterMap*> symbolHitMeterVect;


    /**
     * @brief Construct a pay table and initialise symbol metadata.
     * @param payData 2D pay values indexed by symbol then of-a-kind count. Pay values are in coins (in other words, the actual payout when bet is minBet).
     * @param paySymVect Symbols that pay on lines/ways.
     * @param scatterSymVect Symbols that pay as scatters.
     * @param wildSymVect Symbols that act as wilds.
     * @param wildNoSubstVect Symbols that wilds do not substitute for.
     * @param symbolNames Human-readable names per symbol index.
     */
    PayTable(
        const std::vector<std::vector<unsigned int>>& payData,
        const std::vector<Symbol>& paySymVect,
        const std::vector<Symbol>& scatterSymVect,
        const std::vector<Symbol>& wildSymVect,
        const std::vector<Symbol>& wildNoSubstVect,
        const std::vector<std::string>& symbolNames
    );

    ~PayTable() = default;

    /**
     * @brief Initialise symbol structures and masks.
     * @param symbolCount Total number of distinct symbols.
     */
    void symbolsInit(unsigned int symbolCount);

    /**
     * @brief Get the payout for a symbol with a given of-a-kind count.
     * @param symbolIndex Symbol index.
     * @param symbolCount Number of symbols in the win (of-a-kind).
     * @param checkLimits If true, bounds-checks and may clamp invalid counts.
     * @return Win amount for that combination.
     */
    unsigned int getWin(Symbol symbolIndex, unsigned int symbolCount, bool checkLimits = false) const;

    bool isWild(const Symbol symbolId) const
    {
        return symbolVect[symbolId].isWild;
    }

    bool wildSubstitutes(const Symbol symbolId) const
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