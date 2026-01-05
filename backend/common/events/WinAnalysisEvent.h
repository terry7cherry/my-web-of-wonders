#pragma once

#include "BasicEvent.h"

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
	static constexpr std::string_view EventType = "winAnalysis";

	std::string_view getEventType() const override
	{
		return EventType;
	}

	Json::Value getDetailsAsJson() const override;

private:
	static Json::Value convertWinToJson(const LineWin& win);

	size_t totalWin = 0;
	size_t gridMultiplier = 1;
	std::array<LineWin, WinAnalysisEventBufferSize> bufferedWins;
	std::vector<LineWin> excessWins;
	std::string analysisStage;
	size_t numWins;
};
