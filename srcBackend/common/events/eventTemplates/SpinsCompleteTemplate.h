#pragma once
/**
 * @file SpinsCompleteTemplate.h
 * @brief Template for completion events that report spins-related statistics.
 */

#include "./PhaseCompleteTemplate.h"
#include <inttypes.h>
#include "../../../thisTitle/stats/StatsOutput.h"
#include "../../stats/ReportBuffer.h"

/**
 * @brief Completion event base for phases involving a number of awarded spins that may or may not be retriggered.
 * @details Only necessary when you have more than one phase of this type. Otherwise, use existing FreeSpinsCompleteEvent.
 * @attention When extending this class, you MUST pass the child class name as a template parameter to get separate stats. You must also set phaseName (use FreeSpinsCompleteEvent as a guide).
 * @tparam T Derived event type providing `phaseName`.
 */
template <typename T>
class SpinsCompleteTemplate : public PhaseCompleteTemplate<T>
{
public:
    // These meters are threadsafe and performant.
    inline static DistributionMeter initialSpinsMeter;
    inline static DistributionMeter finalSpinsMeter;
    inline static DistributionMeter initialSpinsWinsMeter;
    inline static DistributionMeter finalSpinsWinsMeter;

    /**
     * @brief Construct with total win and spins info.
     * @param totalWin Total win for the phase.
     * @param initialSpins Spins awarded from base game.
     * @param finalSpins Total spins played, including retriggers.
     * @param repeatEvent A new hit will not be stored if this is true.
     */
    explicit SpinsCompleteTemplate(const unsigned int totalWin,
                                   const unsigned int initialSpins[[maybe_unused]],
                                   const unsigned int finalSpins[[maybe_unused]],
                                   const bool repeatEvent [[maybe_unused]] = false)
        : PhaseCompleteTemplate<T>(totalWin)
    {
    }

    /**
     * @brief Update distribution and hit meters for spins-based phases.
     * @param totalWin Total win for the phase.
     * @param initialSpins Spins awarded from base game.
     * @param finalSpins Total spins played, including retriggers.
     * @param repeatEvent A new hit will not be stored if this is true.
     */
    static void updateMeters(const unsigned int totalWin,
                             const unsigned int initialSpins,
                             const unsigned int finalSpins,
                             const bool repeatEvent = false)
    {
        PhaseCompleteTemplate<T>::updateMeters(totalWin, repeatEvent);
        initialSpinsMeter.add(initialSpins);
        initialSpinsWinsMeter.add(initialSpins, totalWin);
        finalSpinsMeter.add(finalSpins);
        finalSpinsWinsMeter.add(finalSpins, totalWin);
    }

    /** @brief Name for stats output. Override this in derived class. **/
    inline static std::string phaseName = "Template Spins Phase";

    /**
     * @brief Print a CSV-like table of hits/splits/wins and RTP to stdout.
     * @param hitsMeter Sorted hit-count tuples (value,count).
     * @param winsMeter Sorted win-sum tuples (value,sumWins).
     * @param totBet Total bet amount for RTP calculation.
     */
    static void outputHitAndWinReport(
        const std::vector<std::pair<int, uint64_t>>& hitsMeter,
        const std::vector<std::pair<int, uint64_t>>& winsMeter,
        const uint64_t totBet
    )
    {
        std::vector<std::pair<int, uint64_t>>::const_iterator itHits, itWins;
        int64 mapTotal = 0;

        for (itHits = hitsMeter.begin(); itHits != hitsMeter.end(); ++itHits)
            mapTotal += static_cast<int64>(itHits->second);

        std::printf("%30s, %10s, %10s, %10s, %10s\n",
                    "Spins", "Hits", "Splits", "Wins", "RTP");
        for (itHits = hitsMeter.begin(), itWins = winsMeter.begin();
             itHits != hitsMeter.end() && itWins != winsMeter.end();
             ++itHits, ++itWins
        )
        {
            std::printf("%30d, %10" PRIu64 ", %9.2f%%, %10" PRIu64 ", %9.2f%%\n",
                        itHits->first,
                        static_cast<uint64_t>(itHits->second),
                        mapTotal > 0 ? static_cast<long double>(itHits->second) * 100.0 / mapTotal : 0.0,
                        static_cast<uint64_t>(itWins->second),
                        totBet > 0 ? static_cast<long double>(itWins->second) * 100.0 / totBet : 0.0);
        }
    }

    /**
     * @brief Print initial-spins distributions.
     * @param totalBet Total staked in entire simulation (cycles * bet).
     */
    static void outputInitialSpinsReport(const uint64_t totalBet)
    {
        std::printf("\n\tInitial spins:\n");
        const auto& hitsMeter = initialSpinsMeter.getSortedData();
        const auto& winsMeter = initialSpinsWinsMeter.getSortedData();
        outputHitAndWinReport(hitsMeter, winsMeter, totalBet);
    }

    /**
     * @brief Print final-spins distributions.
     * @param totalBet Total staked in simulation (cycles * bet).
     */
    static void outputFinalSpinsReport(const uint64_t totalBet)
    {
        std::printf("\n\tFinal spins:\n");
        const auto& hitsMeter = finalSpinsMeter.getSortedData();
        const auto& winsMeter = finalSpinsWinsMeter.getSortedData();
        outputHitAndWinReport(hitsMeter, winsMeter, totalBet);
    }

    // Buffered report appenders
    static void appendInitialSpinsReport(ReportBuffer& buffer, const uint64_t totalBet)
    {
        const auto& hits = initialSpinsMeter.getSortedData();
        const auto& wins = initialSpinsWinsMeter.getSortedData();
        appendSpins(buffer, "SPINS REPORT - " + T::phaseName + " - Initial", hits, wins, totalBet);
    }

    static void appendFinalSpinsReport(ReportBuffer& buffer, const uint64_t totalBet)
    {
        const auto& hits = finalSpinsMeter.getSortedData();
        const auto& wins = finalSpinsWinsMeter.getSortedData();
        appendSpins(buffer, "SPINS REPORT - " + T::phaseName + " - Final", hits, wins, totalBet);
    }

private:
    static void appendSpins(
        ReportBuffer& buffer,
        const std::string& title,
        const std::vector<std::pair<int, uint64_t>>& hits,
        const std::vector<std::pair<int, uint64_t>>& wins,
        const uint64_t totBet)
    {
        const std::vector<std::string> headers = {"Spins", "Hits", "Splits", "Wins", "RTP"};
        long long mapTotal = 0;
        for (const auto& p : hits) mapTotal += static_cast<long long>(p.second);
        auto pct2 = [](long double v)
        {
            char s[64];
            std::snprintf(s, sizeof(s), "%.2Lf%%", v);
            return std::string(s);
        };
        for (size_t i = 0; i < hits.size() && i < wins.size(); ++i)
        {
            const auto splitsPct = mapTotal > 0
                                       ? static_cast<long double>(hits[i].second) * 100.0L / mapTotal
                                       : 0.0L;
            const auto rtpPct = totBet > 0
                                    ? static_cast<long double>(wins[i].second) * 100.0L / totBet
                                    : 0.0L;
            buffer.addRow(title, headers,
                          {
                              std::to_string(hits[i].first),
                              std::to_string(static_cast<unsigned long long>(hits[i].second)),
                              pct2(splitsPct),
                              std::to_string(static_cast<unsigned long long>(wins[i].second)),
                              pct2(rtpPct)
                          });
        }
    }
};
