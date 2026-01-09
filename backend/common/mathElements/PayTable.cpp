#include "PayTable.h"
#include "../statistics/MeterFile.h"


PayTable::PayTable(
    const std::vector<std::vector<unsigned int>>& payData,
    const std::vector<uint8_t>& paySymVect,
    const std::vector<uint8_t>& scatterSymVect,
    const std::vector<uint8_t>& wildSymVect,
    const std::vector<uint8_t>& wildNoSubstVect)
    : payData(payData),
      numSymbols(static_cast<unsigned int>(payData.size())),
      paySymVect(paySymVect),
      scatterSymVect(scatterSymVect),
      wildSymVect(wildSymVect),
      wildNoSubstVect(wildNoSubstVect)
{
    minPaySymbolsVect.resize(payData.size(), SYMBOL_NONE);

    maxWinSymbols = payData[0].size() - 1;
    minWinSymbols = 0u;
    int iSym = 0;

    for (const auto& symbolPayData : payData)
    {
        for (uint8_t i = 0; i < symbolPayData.size(); i++)
        {
            // find min pay count for this symbol
            if (symbolPayData[i] > 0)
            {
                minPaySymbolsVect[iSym] = i;
                break;
            }
        }

        if (minPaySymbolsVect[iSym] != -1 &&
            (minWinSymbols == -1 || minPaySymbolsVect[iSym] < minWinSymbols))
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

    for (unsigned int i = 0u; i < symbolCount; i++)
    {
        symbolVect[i].index = i;
        symbolVect[i].substWild = true; // assume wilds substitutes for all
        symbolVect[i].mask = (1 << i); // create a mask for all
        symbolVect[i].payMask = 0; // assume not a pay symbol
        symbolVect[i].isWild = false; // assume not wild
        symbolVect[i].wildIndex = -1;
        symbolVect[i].isScatter = false; // assume not scatter
        symbolVect[i].scatterIndex = -1;
    }

    // update for symbols that cannot use Wild
    for (uint8_t symId : wildNoSubstVect)
        symbolVect[symId].substWild = false;


    // update for pay symbols and fill the wildMask
    for (uint8_t symId : paySymVect)
    {
        symbolVect[symId].payMask = symbolVect[symId].mask;
        if (symbolVect[symId].substWild)
            wildMask |= symbolVect[symId].mask;
    }

    // apply the wildMask to all wild symbols
    int i = 0;
    for (uint8_t symId : wildSymVect)
    {
        symbolVect[symId].isWild = true;
        symbolVect[symId].payMask = wildMask;
        symbolVect[symId].wildIndex = i++;
    }

    // update for scatter symbols
    i = 0;
    for (uint8_t symId : scatterSymVect)
    {
        symbolVect[symId].isScatter = true;
        symbolVect[symId].scatterIndex = i++;
        //scatterWinVect.push_back(new ScatterWin(symId, maxReels));
    }
}

unsigned int PayTable::getWin(const uint8_t symbolIndex, const unsigned int symbolCount,
                              const bool checkLimits) const
{
    if (checkLimits)
    {
        if (symbolIndex < 0 || symbolIndex >= static_cast<int>(payData.size()))
            std::printf("\r\nError: PayTable::getWin() - symbolIndex (%d) out of bounds", symbolIndex);

        if (symbolCount < minPaySymbolsVect[symbolIndex])
            std::printf("\r\nError: PayTable::getWin() - symbolCount (%d) has no payout", symbolCount);
        else if (symbolCount > maxWinSymbols)
            std::printf("\r\nError: PayTable::getWin() - symbolCount (%d) > maxWinSymbols (%d)",
                        symbolCount, static_cast<int>(maxWinSymbols));
    }
    return payData[symbolIndex][symbolCount];
}

