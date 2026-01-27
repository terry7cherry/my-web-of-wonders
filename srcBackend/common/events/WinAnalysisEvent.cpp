#include "WinAnalysisEvent.h"
#include "../grids/PayGrid.h"

WinAnalysisEvent::WinAnalysisEvent(const PayGrid& grid, const std::string& analysisStage)
    : wins(grid.winVector),
      analysisStage(analysisStage)
{
    for (const auto& win : wins)
    {
        totalWin += win.totalWin;
    }
}

void WinAnalysisEvent::updateMeters(const PayGrid& grid [[maybe_unused]],
                                    std::string analysisStage [[maybe_unused]])
{
    for (const auto& win : grid.winVector)
    {
        payTableHits.add(win.symbol, win.oak);
        payTableWinnings.add(win.symbol, win.oak, win.totalWin);
    }
}

Json::Value WinAnalysisEvent::getWinsAsJson() const
{
    Json::Value winsAsJson(Json::arrayValue);
    for (const Win& win : wins)
    {
        winsAsJson.append(convertWinToJson(win));
    }
    return winsAsJson;
}

Json::Value WinAnalysisEvent::toJson() const
{
    Json::Value eventDetails;
    eventDetails["eventType"] = "winAnalysis";
    eventDetails["totalWin"] = totalWin;
    eventDetails["analysisStage"] = std::string(analysisStage);
    eventDetails["wins"] = getWinsAsJson();
    return eventDetails;
}

Json::Value WinAnalysisEvent::convertWinToJson(const Win& win)
{
    Json::Value winAsJson;
    winAsJson["symbol"] = win.symbol;
    winAsJson["oak"] = win.oak;
    winAsJson["wildCount"] = win.wildCount;
    winAsJson["multiplier"] = win.multiplier;
    winAsJson["win"] = win.win;
    winAsJson["totalWin"] = win.totalWin;
    winAsJson["direction"] = static_cast<int>(win.direction);
    winAsJson["winType"] = win.winType;
    if (win.winType == "line") winAsJson["payLine"] = BasicEvent::toJsonArray(*win.payLine);
    return winAsJson;
}

void WinAnalysisEvent::consoleReportRTP(uint64_t simGames, int bet,
                                        const std::vector<std::string>& symbolNames)
{
    size_t maxOAK = 0;
    size_t minOAK = SIZE_MAX;
    for (int symbolId = 0; symbolId < symbolNames.size(); symbolId++)
    {
        for (int oak = 1; oak <= PayTableMeter::MAX_OAK; oak++)
        {
            if (oak > maxOAK)
                if (payTableWinnings.global[symbolId][oak - 1] > 0)
                    maxOAK = oak;
            if (oak < minOAK)
                if (payTableWinnings.global[symbolId][oak - 1] > 0)
                    minOAK = oak;
        }
    }

    printf("\n\nPAYTABLE RTP REPORT\n");
    printf("%10s", "Symbol");
    for (size_t oak = minOAK; oak <= maxOAK; oak++)
        printf("\t\t%llu OAK", oak);
    for (size_t symbolId = 0; symbolId < symbolNames.size(); symbolId++)
    {
        printf("\n%10s", symbolNames[symbolId].c_str());
        for (size_t oak = minOAK; oak <= maxOAK; oak++)
        {
            printf("\t%12.2f%%",
                   static_cast<double>(100 * static_cast<long double>(
                           payTableWinnings.global[symbolId][oak - 1]) / simGames / bet
                   ));
        }
    }
}
