#include <absl/container/flat_hash_map.h>
#include "PayTable.h"


PayTable::PayTable(
    const std::vector<std::vector<unsigned int>>& payData,
    const std::vector<Symbol>& paySymVect,
    const std::vector<Symbol>& scatterSymVect,
    const std::vector<Symbol>& wildSymVect,
    const std::vector<Symbol>& wildNoSubstVect,
    const std::vector<Symbol>& scatterWildSymVect,
    const std::vector<std::string>& symbolNames
)
    : payData(payData),
      numSymbols(static_cast<unsigned int>(payData.size())),
      paySymVect(paySymVect),
      scatterSymVect(scatterSymVect),
      wildSymVect(wildSymVect),
      wildNoSubstVect(wildNoSubstVect),
      scatterWildSymVect(scatterWildSymVect),
      symbolNames(symbolNames)
{
    minPaySymbolsVect.resize(payData.size(), SYMBOL_NONE);

    maxWinSymbols = payData[0].size() - 1;
    minWinSymbols = 0u;
    int iSym = 0;

    for (const auto& symbolPayData : payData)
    {
        for (Symbol i = 0; i < symbolPayData.size(); i++)
        {
            // find min pay count for this symbol
            if (symbolPayData[i] > 0)
            {
                minPaySymbolsVect[iSym] = i;
                break;
            }
        }

        if (minPaySymbolsVect[iSym] != 0 &&
            (minWinSymbols == 0 || minPaySymbolsVect[iSym] < minWinSymbols))
        {
            // got winning symbols and they are the lowest seen so far
            minWinSymbols = minPaySymbolsVect[iSym];
        }
        iSym++;
    }

    symbolsInit(numSymbols);
}

void PayTable::symbolsInit(const unsigned int symbolCount)
{
    wildMask = 0;
    symbolVect.resize(symbolCount);

    for (Symbol i = 0u; i < symbolCount; i++)
    {
        symbolVect[i].symbol = i;
        symbolVect[i].substWild = true; // assume wilds substitutes for all
        symbolVect[i].mask = (1 << i); // create a mask for all
        symbolVect[i].payMask = 0; // assume not a pay symbol
        symbolVect[i].isWild = false; // assume not wild
        symbolVect[i].isScatterWild = false;
        symbolVect[i].wildIndex = -1;
        symbolVect[i].isScatter = false; // assume not scatter
        symbolVect[i].scatterIndex = -1;
        symbolVect[i].name = symbolNames[i];
        symbolIndexMap[symbolNames[i]] = i;
    }

    // update for symbols that cannot use Wild
    for (Symbol symId : wildNoSubstVect)
        symbolVect[symId].substWild = false;


    // update for pay symbols and fill the wildMask
    for (Symbol symId : paySymVect)
    {
        symbolVect[symId].payMask = symbolVect[symId].mask;
        if (symbolVect[symId].substWild)
            wildMask |= symbolVect[symId].mask;
    }

    // apply the wildMask to all wild symbols
    int i = 0;
    for (Symbol symId : wildSymVect)
    {
        symbolVect[symId].isWild = true;
        symbolVect[symId].payMask = wildMask;
        symbolVect[symId].wildIndex = i++;
    }


    // update for scatter symbols
    i = 0;
    for (Symbol symId : scatterSymVect)
    {
        symbolVect[symId].isScatter = true;
        symbolVect[symId].scatterIndex = i++;
    }

    i = 0;
    for (Symbol symId : scatterWildSymVect)
    {
        symbolVect[symId].isScatterWild = true;
        symbolVect[symId].scatterWildIndex = i++;
    }
}

unsigned int PayTable::getWin(const Symbol symbolIndex, const unsigned int symbolCount,
                              const bool checkLimits) const
{
    if (checkLimits)
        if (symbolCount > maxWinSymbols)
            return payData[symbolIndex][maxWinSymbols];
    return payData[symbolIndex][symbolCount];
}

Json::Value PayTable::awardsToJson() const
{
    Json::Value json;
    for (size_t i = 0; i < payData.size(); i++)
    {
        const SymbolData symbol = symbolVect[i];
        json[symbolNames[i]] = Json::Value();
        json[symbolNames[i]]["func"] = symbol.isWild
                                           ? "wild"
                                           : symbol.isScatter
                                           ? "scatter"
                                           : "line";
        json[symbolNames[i]]["awards"] = Json::Value(Json::arrayValue);
        for (const unsigned int award : payData[i])
            json[symbolNames[i]]["awards"].append(award);
    }
    return json;
}

Json::Value PayTable::symbolNamesToJson() const
{
    Json::Value json(Json::arrayValue);
    for (const std::string& name : symbolNames)
        json.append(name);
    return json;
}


