#include "PayTable.h"
#include "../statistics/MeterFile.h"


PayTable::PayTable(
    const std::vector<std::vector<size_t>>& payData,
    const std::vector<size_t>& paySymVect,
    const std::vector<size_t>& scatterSymVect,
    const std::vector<size_t>& wildSymVect,
    const std::vector<size_t>& wildNoSubstVect)
    : payData(payData),
      numSymbols(payData.size()),
      paySymVect(paySymVect),
      scatterSymVect(scatterSymVect),
      wildSymVect(wildSymVect),
      wildNoSubstVect(wildNoSubstVect)
{
    minPaySymbolsVect.resize(payData.size(), -1);

    maxWinSymbols = payData[0].size() - 1;
    minWinSymbols = -1;
    int iSym = 0;

    for (const auto& symbolPayData : payData)
    {
        for (size_t i = 0; i < symbolPayData.size(); i++)
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

void PayTable::symbolsInit(const int symbolCount)
{
    wildMask = 0;
    symbolVect.resize(symbolCount);

    for (int i = 0; i < symbolCount; i++)
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
    for (int symId : wildNoSubstVect)
        symbolVect[symId].substWild = false;


    // update for pay symbols and fill the wildMask
    for (int symId : paySymVect)
    {
        symbolVect[symId].payMask = symbolVect[symId].mask;
        if (symbolVect[symId].substWild)
            wildMask |= symbolVect[symId].mask;
    }

    // apply the wildMask to all wild symbols
    int i = 0;
    for (int symId : wildSymVect)
    {
        symbolVect[symId].isWild = true;
        symbolVect[symId].payMask = wildMask;
        symbolVect[symId].wildIndex = i++;
    }

    // update for scatter symbols
    i = 0;
    for (int symId : scatterSymVect)
    {
        symbolVect[symId].isScatter = true;
        symbolVect[symId].scatterIndex = i++;
        //scatterWinVect.push_back(new ScatterWin(symId, maxReels));
    }
}

size_t PayTable::getWin(const size_t symbolIndex, const size_t symbolCount, const bool checkLimits) const
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

