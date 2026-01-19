#pragma once
/**
 * @file WinAnalysisEvent.h
 * @brief Event that captures line/ways wins for analysis/visualization.
 */

#include "eventTemplates/BasicEvent.h"

#include <vector>

#include "../grids/LineGrid.h"

static constexpr int WinAnalysisEventBufferSize = 4;

/**
 * @brief Serializes a subset of line wins along with totals for UI/debug output.
 */
class WinAnalysisEvent : public BasicEvent
{
public:
	/**
	 * @param grid Grid from which to take wins.
	 * @param analysisStage Name of analysis stage, for display in frontend ticker.
	 */
	explicit WinAnalysisEvent(const LineGrid& grid, const std::string& analysisStage);

	/**
	 * @brief Optional metrics hook.
	 * @details Note this is required because updateMeters arguments must match constructor.
	 * @param grid Grid from which to take wins.
	 * @param analysisStage Name of analysis stage, for display in frontend ticker.
	 */
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
	/**
	 * @brief Convert buffered/excess wins into a compact JSON array.
	 */
	Json::Value getWinsAsJson() const;
	/**
	 * @brief Convert a single win to JSON.
	 */
	static Json::Value convertWinToJson(const LineWin& win);

	unsigned int totalWin = 0; // total of all wins in this analysis stage
	unsigned int gridMultiplier = 1;
	std::array<LineWin, WinAnalysisEventBufferSize> bufferedWins;
	std::vector<LineWin> excessWins;
	std::string analysisStage; // e.g., "preMultiplier", "postMultiplier"
	size_t numWins;
};
