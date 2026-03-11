#pragma once
/**
 * @file StatsOutput.h
 * @brief Reporting helpers for simulation statistics (console and CSV files).
 */

#include "../../common/stats/ReportBuffer.h"
#include <cstdint>
#include <string>
#include <filesystem>
#include <cstdio>
#include <ostream>
#include <vector>
#include <map>

namespace Stats
{
    /**
     * @brief Buffered reporting class to collect tables/stats, then print to console and/or write CSV.
     * @details Usage:
     *   Stats::ReportBuffer rep;
     *   rep.addRow("GENERAL STATS", {"Phase","RTP"}, {"Base","96.12%"});
     *   rep.addStat("RUN SETTINGS", "Bet", "1");
     *   rep.printConsole();
     *   rep.writeCsv(csvStream);
     */

    /**
     * @brief Produce all statistics outputs (console and files) for a run.
     * @param simGames Number of simulated games.
     * @param betType Bet type label (e.g., "default").
     * @param rtp RTP value (e.g., 96).
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param simForce Name of sim force for run.
     * @param symbolNames Vector containing the readable name for each symbol.
     * @param currentWinCap The set win cap from the maths sheet, or 0 if not present.
     */
    void outputStats(uint64_t simGames, const std::string& betType, int rtp, int bet,
                     const std::string& simForce,
                     const std::vector<std::string>& symbolNames, unsigned int currentWinCap);

    /**
     * @brief Write reports that are for file output only (e.g. Distributions).
     * @param betType Bet type label (e.g., "default").
     * @param rtp RTP value (e.g., 96).
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param simGames Number of game cycles run in simulation.
     */
    void outputDistributionStats(const std::string& betType, int rtp, int bet, uint64_t simGames,
                                 const std::filesystem::path& outputFolder);

    /**
     * @brief Print a general (RTP, hit rates, averages) report to the console.
     * @param buffer Buffer to write stats to.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     */
    void addGeneralStatsReport(ReportBuffer& buffer, uint64_t simGames, int bet);

    /**
     * @brief Print a breakdown of RTP by paytable awards.
     * @param buffer Buffer to write stats to.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param symbolNames Vector containing the readable name for each symbol.
     */
    void addPayTableRTPReport(ReportBuffer& buffer, uint64_t simGames, int bet,
                              const std::vector<std::string>& symbolNames);

    /**
     * @brief Print maximum win report (value, frequency) to the console.
     * @param buffer Buffer to write stats to.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
   */
    void addMaxWinReport(ReportBuffer& buffer, uint64_t simGames, int bet);

    /**
     * @brief Print distributions for spins-related events.
     * @param buffer Buffer to write stats to.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     */
    void addSpinsReport(ReportBuffer& buffer, uint64_t simGames, int bet);


    /**
     * @brief Print table showing result of applying different win caps.
     * @param buffer Buffer to write stats to.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param currentWinCap The set win cap from the maths sheet, or 0 if not present.
     */
    void addWinCapReport(ReportBuffer& buffer,
                         uint64_t simGames,
                         int bet,
                         unsigned int currentWinCap);
}
