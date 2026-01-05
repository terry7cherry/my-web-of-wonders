#include "WinAnalysisEvent.h"

WinAnalysisEvent::WinAnalysisEvent(const LineGrid& grid, const std::string& analysisStage)
    : analysisStage(analysisStage)
{
    const std::vector<LineWin>& sourceWins = grid.winVector;
    numWins = sourceWins.size();

    for (const auto& win : sourceWins)
    {
        totalWin += win.totalWin;
    }

    for (size_t i = 0; i < numWins && i < WinAnalysisEventBufferSize; ++i)
    {
        bufferedWins[i] = sourceWins[i];
    }

    if (numWins > WinAnalysisEventBufferSize)
    {
        excessWins.reserve(numWins - WinAnalysisEventBufferSize);
        excessWins.assign(sourceWins.begin() + WinAnalysisEventBufferSize, sourceWins.end());
    }
}

Json::Value WinAnalysisEvent::getDetailsAsJson() const
{
    Json::Value eventDetails;
    eventDetails["totalWin"] = totalWin;
    eventDetails["analysisStage"] = std::string(analysisStage);
    Json::Value winsAsJson(Json::arrayValue);
    for (size_t i = 0; i < numWins && i < WinAnalysisEventBufferSize; ++i)
    {
        winsAsJson.append(convertWinToJson(bufferedWins[i]));
    }

    if (numWins > WinAnalysisEventBufferSize)
    {
        for (const LineWin& win : excessWins)
        {
            winsAsJson.append(convertWinToJson(win));
        }
    }
    eventDetails["wins"] = winsAsJson;
    return eventDetails;
}

Json::Value WinAnalysisEvent::convertWinToJson(const LineWin& win)
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
    if (win.winType == "line") winAsJson["payLine"] = toJsonArray(*win.payLine);
    return winAsJson;
}
