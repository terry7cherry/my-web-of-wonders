#pragma once
/**
 * @file StatsOutput.h
 * @brief Reporting helpers for simulation statistics (console and CSV files).
 */
#include <cstdint>
#include <string>

namespace Stats
{
 /**
  * @brief Produce all statistics outputs (console and files) for a run.
  * @param simGames Number of simulated games.
  * @param betType Bet type label (e.g., "default").
  * @param rtp RTP value (e.g., 96).
  * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
  */
 void outputStats(uint64_t simGames, const std::string& betType, int rtp, int bet);

 /**
  * @brief Print stats that are for console output only.
  * @param simGames Number of simulated games.
  * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
  */
 void outputConsoleStats(uint64_t simGames, int bet);

 /**
  * @brief Write reports that are for file output only (e.g. Distributions).
  * @param betType Bet type label (e.g., "default").
  * @param rtp RTP value (e.g., 96).
  * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
  */
 void outputFileStats(const std::string& betType, int rtp, int bet);

 /**
  * @brief Print a general (RTP, hit rates, averages) report to the console.
  * @param simGames Number of simulated games.
  * @param bet Bet amount per game (in coins, same unit as minBet and paytable payouts).
  */
 void outputGeneralStatsReport(uint64_t simGames, int bet);

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
}
