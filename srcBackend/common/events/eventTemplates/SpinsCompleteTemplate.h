#pragma once

#include "./PhaseCompleteTemplate.h"

template <typename T>
class SpinsCompleteTemplate : public PhaseCompleteTemplate<T>
{
public:
    inline static DistributionMeter initialSpinsMeter;
    inline static DistributionMeter finalSpinsMeter;
    inline static DistributionMeter initialSpinsWinsMeter;
    inline static DistributionMeter finalSpinsWinsMeter;

    explicit SpinsCompleteTemplate(const int totalWin,
                                   const int initialSpins[[maybe_unused]],
                                   const int finalSpins[[maybe_unused]])
        : PhaseCompleteTemplate<T>(totalWin)
    {
    }

    static void updateMeters(const int totalWin, const int initialSpins, const int finalSpins)
    {
        PhaseCompleteTemplate<T>::updateMeters(totalWin);
        initialSpinsMeter.add(initialSpins);
        initialSpinsWinsMeter.add(initialSpins, totalWin);
        finalSpinsMeter.add(finalSpins);
        finalSpinsWinsMeter.add(finalSpins, totalWin);
    }


    inline static std::string phaseName = "Template Spins Phase";

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

        std::printf("%20s, %10s, %10s, %10s, %10s\n",
                    phaseName.c_str(), "Hits", "Splits", "Wins", "RTP");
        for (itHits = hitsMeter.begin(), itWins = winsMeter.begin();
             itHits != hitsMeter.end() && itWins != winsMeter.end();
             ++itHits, ++itWins
        )
        {
            std::printf("%20d, %10llu, %9.2f%%, %10llu, %9.2f%%\n",
                        itHits->first,
                        static_cast<unsigned long long>(itHits->second),
                        mapTotal ? static_cast<double>(itHits->second) * 100.0 / mapTotal : 0.0,
                        static_cast<unsigned long long>(itWins->second),
                        totBet ? static_cast<double>(itWins->second) * 100.0 / totBet : 0.0);
        }
    }

    static void outputInitialSpinsReport(const uint64_t totalBet)
    {
        std::printf("\nInitial spins:\n");
        const auto& hitsMeter = initialSpinsMeter.getSortedData();
        const auto& winsMeter = initialSpinsWinsMeter.getSortedData();
        outputHitAndWinReport(hitsMeter, winsMeter, totalBet);
    }

    static void outputFinalSpinsReport(const uint64_t totalBet)
    {
        std::printf("\nFinal spins:\n");
        const auto& hitsMeter = finalSpinsMeter.getSortedData();
        const auto& winsMeter = finalSpinsWinsMeter.getSortedData();
        outputHitAndWinReport(hitsMeter, winsMeter, totalBet);
    }

protected
:
    std::string_view getEventType() const override { return "spinsPhaseEnd"; }
};
