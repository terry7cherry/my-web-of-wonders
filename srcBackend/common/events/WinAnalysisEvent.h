#pragma once

#include "eventTemplates/BasicEvent.h"

#include <vector>

#include "../grids/LineGrid.h"

static constexpr int WinAnalysisEventBufferSize = 4;

class WinAnalysisEvent : public BasicEvent
{
public:
	explicit WinAnalysisEvent(const LineGrid& grid, const std::string& analysisStage);

	static void updateMeters(const LineGrid& grid [[maybe_unused]],
	                         std::string analysisStage [[maybe_unused]])
	{
	}

protected:
	Json::Value toJson() const override
	{
		Json::Value eventDetails;
		eventDetails["eventType"] = "winAnalysis";
		eventDetails["totalWin"] = totalWin;
		eventDetails["analysisStage"] = std::string(analysisStage);
		eventDetails["wins"] = getWinsAsJson();
		return eventDetails;
	}

private:
	Json::Value getWinsAsJson() const;
	static Json::Value convertWinToJson(const LineWin& win);

	unsigned int totalWin = 0;
	unsigned int gridMultiplier = 1;
	std::array<LineWin, WinAnalysisEventBufferSize> bufferedWins;
	std::vector<LineWin> excessWins;
	std::string analysisStage;
	size_t numWins;
};
