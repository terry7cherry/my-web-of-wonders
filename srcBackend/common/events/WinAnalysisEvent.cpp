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

void WinAnalysisEvent::reportRTP(ReportBuffer& buffer,
                                 const uint64_t simGames,
                                 const int bet,
                                 const std::vector<std::string>& symbolNames)
{
    // Determine OAK range present
    std::size_t maxOAK = 0;
    std::size_t minOAK = PayTableMeter::MAX_OAK;
    for (int symbolId = 0; symbolId < static_cast<int>(symbolNames.size()); ++symbolId)
    {
        for (std::size_t oak = 1; oak <= PayTableMeter::MAX_OAK; ++oak)
        {
            if (payTableWinnings.global[symbolId][oak - 1] > 0)
            {
                if (oak > maxOAK) maxOAK = oak;
                if (oak < minOAK) minOAK = oak;
            }
        }
    }
    if (maxOAK == 0)
    {
        minOAK = 1;
        maxOAK = 1;
    }

    std::vector<std::string> headers;
    headers.emplace_back("Symbol");
    for (std::size_t oak = minOAK; oak <= maxOAK; ++oak)
    {
        headers.emplace_back(std::to_string(static_cast<unsigned long long>(oak)) + " OAK");
    }


    for (std::size_t symbolId = 0; symbolId < symbolNames.size(); ++symbolId)
    {
        std::vector<std::string> row;
        row.push_back(symbolNames[symbolId]);
        for (std::size_t oak = minOAK; oak <= maxOAK; ++oak)
        {
            const auto winnings = payTableWinnings.global[symbolId][oak - 1];
            long double pct = 0.0L;
            if (simGames > 0 && bet > 0)
                pct = static_cast<long double>(winnings) /
                    (static_cast<long double>(simGames) * bet);
            row.push_back(buffer.toPercent(pct, 2));
        }
        buffer.addRow("PAYTABLE RTP REPORT", headers, row);
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
