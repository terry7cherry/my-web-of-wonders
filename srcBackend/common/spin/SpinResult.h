#pragma once
/**
 * @file SpinResult.h
 * @brief Determines reel stops (and optionally reel set) for a spin and stores the result for grid application.
 */

#include <vector>
#include "../gameMaths/mathElements/ReelSet.h"
#include "../gameMaths/mathElements/WeightTables.h"

/**
 * @brief Determines reelstops (and optionally reel set) for spin and stores the result.
 */
struct SpinResult
{
    /** @brief Stop index per reel. */
    std::vector<size_t> stopIndexVect;

    /** @brief ReelSet to be used. */
    const ReelSet& reelSet;

    /**
     * @brief Spin all reels.
     * @param reelSet ReelSet to use.
     * @param randomSpin Set to false to prevent random stops being set if you plan to set them manually.
     */
    SpinResult(const ReelSet& reelSet, bool randomSpin = true);

    /**
     * @brief Spin only the reels listed in @p spinVect.
     * @details Reels not present in @p spinVect have stops set to 0.
     * @param reelSet ReelSet to use for reel lengths and RNG range.
     * @param spinVect Reel indexes to spin.
     */
    SpinResult(const ReelSet& reelSet, const std::vector<size_t>& spinVect);

    /**
     * @brief Spin only the reels NOT set to true in lockedVect.
     * @details If a reel is locked, it has its stop set to 0.
     * @param reelSet ReelSet to use for reel lengths and RNG range.
     * @param lockedVect Lock state per reel.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSet& reelSet, const std::vector<bool>& lockedVect);

    /**
     * @brief Choose a reelset, then spin all reels.
     * @param weightTable ReelSet weight table to use.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSetWeights& weightTable)
        : SpinResult(weightTable.pick())
    {
    }

    /**
     * @brief Choose a reelset, then spin only the reels listed in @p spinVect.
     * @details Reels not present in @p spinVect have stops set to 0.
     * @param weightTable Reel set weight table to use.
     * @param spinVect Reel indexes to spin.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSetWeights& weightTable, const std::vector<size_t>& spinVect)
        : SpinResult(weightTable.pick(), spinVect)
    {
    }

    /**
     * @brief Choose a reelset, then spin the reels not set to true in lockedVect.
     * @details If a reel is locked, it has its stop set to 0.
     * @param weightTable Reel set weight table to use.
     * @param lockedVect Lock state per reel.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSetWeights& weightTable, const std::vector<bool>& lockedVect)
        : SpinResult(weightTable.pick(), lockedVect)
    {
    }

    /**
     * @brief Sets reel stop positions for all reels.
     * @details Replaces the current reel stops with the provided vector of stop positions.
     * @param stopVect Vector containing the stop positions for each reel.
     */
    void setReelStops(const std::vector<size_t>& stopVect);

    /**
     * @brief Sets reel stop position for reel.
     * @param reel Reel index.
     * @param stop Reel stop index.
     */
    void setReelStop(size_t reel, size_t stop);
};
