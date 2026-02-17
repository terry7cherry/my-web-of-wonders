#include "WinAnalysisEvent.h"
#include "../grids/PayGrid.h"

WinAnalysisEvent::WinAnalysisEvent(const PayGrid& grid,
                                   const GameMaths& maths,
                                   const std::string& analysisStage) // NOLINT(*-pass-by-value)
    : symbolNames(maths.payTables["default"].symbolNames),
      analysisStage(analysisStage)
{
    wins.reserve(grid.winVector.size());
    for (const auto& winPtr : grid.winVector)
    {
        // Deep copy to preserve any derived data if present in future
        wins.push_back(winPtr->clone());
        totalWin += winPtr->totalWin;
    }
}

void WinAnalysisEvent::updateMeters(const PayGrid& grid [[maybe_unused]],
                                    const GameMaths& maths [[maybe_unused]],
                                    const std::string& analysisStage [[maybe_unused]])
{
    for (const auto& winPtr : grid.winVector)
    {
        payTableHits.add(winPtr->symbol, winPtr->oak);
        payTableWinnings.add(winPtr->symbol, winPtr->oak, winPtr->totalWin);
    }
}

Json::Value WinAnalysisEvent::getWinsAsJson() const
{
    Json::Value winsAsJson(Json::arrayValue);
    for (const auto& winPtr : wins)
    {
        winsAsJson.append(winPtr->toJson(symbolNames));
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

void WinAnalysisEvent::consoleReportRTP(const uint64_t simGames,
                                        const int bet,
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
                   static_cast<double>(100
                       * static_cast<long double>(payTableWinnings.global[symbolId][oak - 1])
                       / static_cast<long double>(simGames)
                       / bet
                   ));
        }
    }
}

std::string WinAnalysisEvent::prettyPrint(const std::string& baseIndent)
{
    Json::Value jsonEvent = toJson();
    for (auto& win : jsonEvent["wins"])
    {
        std::string prettyPayLine = "";
        for (const auto& aperture : win["payLine"])
            prettyPayLine += aperture.asString() + " ";
        win["payLine"] = prettyPayLine;
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  "; // two spaces (default is 3)

    std::string json = Json::writeString(builder, jsonEvent);

    std::ostringstream out;
    std::istringstream in(json);
    std::string line;

    while (std::getline(in, line))
    {
        out << baseIndent << line << '\n';
    }

    return out.str();
}
