#pragma once
/**
 * @file StatsOutput.h
 * @brief Reporting helpers for simulation statistics (console and CSV files).
 */
#include <cstdint>
#include <string>
#include <filesystem>

namespace Stats
{
    /**
     * @brief Produce all statistics outputs (console and files) for a run.
     * @param simGames Number of simulated games.
     * @param betType Bet type label (e.g., "default").
     * @param rtp RTP value (e.g., 96).
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param symbolNames Vector containing the readable name for each symbol.
     * @param currentWinCap The set win cap from the maths sheet, or 0 if not present.
     */
    void outputStats(uint64_t simGames, const std::string& betType, int rtp, int bet,
                     const std::vector<std::string>& symbolNames, unsigned int currentWinCap);

    /**
     * @brief Print stats that are for console output only.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param symbolNames Vector containing the readable name for each symbol.
     * @param currentWinCap The set win cap from the maths sheet, or 0 if not present.
     */
    void outputConsoleStats(uint64_t simGames, int bet, const std::vector<std::string>& symbolNames,
                            unsigned int currentWinCap);

    /**
     * @brief Write reports that are for file output only (e.g. Distributions).
     * @param betType Bet type label (e.g., "default").
     * @param rtp RTP value (e.g., 96).
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param simGames Number of game cycles run in simulation.
     */
    void outputFileStats(const std::string& betType, int rtp, int bet, uint64_t simGames);

    /**
     * @brief Print a general (RTP, hit rates, averages) report to the console.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     */
    void outputGeneralStatsReport(uint64_t simGames, int bet);

    /**
     * @brief Print a breakdown of RTP by paytable awards.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param symbolNames Vector containing the readable name for each symbol.
     */
    void outputPayTableRTPReport(uint64_t simGames, int bet, const std::vector<std::string>& symbolNames);

    /**
     * @brief Print maximum win report (value, frequency) to the console.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
   */
    void outputMaxWinReport(uint64_t simGames, int bet);

    /**
     * @brief Print distributions for spins-related events.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     */
    void outputSpinsReport(uint64_t simGames, int bet);

    /**
     * @brief Print table showing result of applying different win caps.
     * @param simGames Number of simulated games.
     * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
     * @param currentWinCap The set win cap from the maths sheet, or 0 if not present.
     */
    void outputWinCappingReport(uint64_t simGames, int bet, unsigned int currentWinCap);

    std::filesystem::path createOutputFolder(const std::string& betType, int rtp, int bet, uint64_t simGames);
}
