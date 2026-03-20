#pragma once

#include "../common/gameMaths/GameMaths.h"
#include "state/GameCycle.h"
#include "CollectorPositions.h"

namespace GameLogic
{
    unsigned int playGame(const GameMaths& maths, GameCycle& cycle);

    void updateMainCollectorPosition(const GameMaths& maths, GameCycle& cycle);

    unsigned int doBaseGame(const GameMaths& maths, GameCycle& cycle);

    unsigned int doFreeSpins(const GameMaths& maths, GameCycle& cycle);

    void activateInitialCollectors(const GameMaths& maths, GameCycle& cycle);

    unsigned int doOneFreeSpin(const GameMaths& maths, GameCycle& cycle);

    void spinReelsAndSetCoinValues(const GameMaths& maths, GameCycle& cycle,
                                   const std::string& weightTableName);

    void collectBonusSymbols(const GameMaths& maths, GameCycle& cycle);

    unsigned int analyseAndCascadeIfWin(const GameMaths& maths, GameCycle& cycle);

    void setMainCollectorMultiplier(const GameMaths& maths, const GameCycle& cycle);

    void collectRemainingCoins(const GameMaths& maths, GameCycle& cycle);

    void setCoinValues(const GameMaths& maths, GameCycle& cycle);

    unsigned int analysePayAnywhereAndCoinWinnings(const GameMaths& maths, GameCycle& cycle);

    void collectCoin(const GameMaths& maths, GameCycle& cycle, size_t reel, size_t row,
                     bool* collectorHasCollected);

    void activateCollector(const GameMaths& maths, GameCycle& cycle, CollectorPosition collectorPosition);

    void removeExcessScatters(const GameMaths& maths, GameCycle& cycle,
                              const std::vector<std::pair<size_t, size_t>>& removedSymbols);

    void recordFreeSpinsCompleteEvents(const GameMaths& maths, GameCycle& cycle);

    void handleFreeSpinsForces(const GameMaths& maths, GameCycle& cycle);

    void maximizeMultipliersForEnhancedFreeSpins(const GameMaths& maths, GameCycle& cycle);

    void updateDeadSpins(const GameMaths& maths, GameCycle& cycle);
}
