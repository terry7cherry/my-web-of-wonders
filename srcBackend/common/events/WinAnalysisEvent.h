#pragma once
/**
 * @file WinAnalysisEvent.h
 * @brief Event that captures line/ways wins for analysis/visualization.
 */


#include <vector>
#include <memory>

#include "eventTemplates/BasicEvent.h"
#include "../grids/PayGrid.h"
#include "threadSafeMeters/PayTableMeter.h"
// Buffered reporting API
#include "../stats/ReportBuffer.h"

/**
 * @brief Serializes a subset of line wins along with totals for UI/debug output.
 */
class WinAnalysisEvent : public BasicEvent
{
public:
	unsigned int totalWin = 0; // total of all wins in this analysis stage
	unsigned int gridMultiplier = 1;
	// Store polymorphic wins without slicing; Event owns its own copies
	std::vector<std::unique_ptr<Win>> wins;
	const std::vector<std::string>& symbolNames;
	std::string analysisStage; // e.g., "base game", "free spins"

	/**
	 * @param grid Grid from which to take wins.
     * @param maths Game maths object for symbol names.
	 * @param analysisStage Name of analysis stage, for display in frontend ticker.
	 */
	explicit WinAnalysisEvent(const PayGrid& grid,
	                          const GameMaths& maths,
	                          const std::string& analysisStage);

	/**
	 * @brief Optional metrics hook.
	 * @details Note this is required because updateMeters arguments must match constructor.
	 * @param grid Grid from which to take wins.
     * @param maths Game maths object for symbol names.
	 * @param analysisStage Name of analysis stage, for display in frontend ticker.
	 */
	static void updateMeters(const PayGrid& grid [[maybe_unused]],
	                         const GameMaths& maths [[maybe_unused]],
	                         const std::string& analysisStage [[maybe_unused]]);

	static void consoleReportRTP(uint64_t simGames, int bet, const std::vector<std::string>& symbolNames);

	// Buffered report version (adds PAYTABLE RTP REPORT table)
	static void reportRTP(ReportBuffer& buffer,
	                      uint64_t simGames,
	                      int bet,
	                      const std::vector<std::string>& symbolNames);

	inline static PayTableMeter payTableHits;
	inline static PayTableMeter payTableWinnings;

	std::string prettyPrint(const std::string& baseIndent) override;

protected:
	Json::Value toJson() const override;

private:
	/**
	 * @brief Convert buffered/excess wins into a compact JSON array.
	 */
	Json::Value getWinsAsJson() const;
};
