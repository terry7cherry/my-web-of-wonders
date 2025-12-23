#pragma once

#include <cstddef>
#include <vector>
#include "../mathElements/ReelSet.h"
#include "../mathElements/ReelSetWeightTable.h"

/**
 * @brief Determines reelstops for spin and stores the result.
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
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSet& reelSet, bool randomSpin = true);

    /**
     * @brief Spin only the reels listed in @p spinVect.
     *
     * Reels not present in @p spinVect have stops set to 0.
     *
     * @param reelSet ReelSet to use for reel lengths and RNG range.
     * @param spinVect Reel indexes to spin.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSet& reelSet, const std::vector<size_t>& spinVect);

    /**
     * @brief Spin only the reels not set to true in lockedVect.
     *
     * If a reel is locked, it has its stop set to 0.
     *
     * @param reelSet ReelSet to use for reel lengths and RNG range.
     * @param lockedVect Lock state per reel.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSet& reelSet, const std::vector<bool>& lockedVect);

    /**
     * @brief Choose a reelset, then spin all reels.
     * @param weightTablePtr ReelSet weight table to use.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSetWeightTable& weightTablePtr)
        : SpinResult(*weightTablePtr.pick())
    {
    }

    /**
     * @brief Choose a reelset, then spin only the reels listed in @p spinVect.
     *
     * Reels not present in @p spinVect have stops set to 0.
     *
     * @param weightTablePtr Reel set weight table to use.
     * @param spinVect Reel indexes to spin.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSetWeightTable& weightTablePtr, const std::vector<size_t>& spinVect)
        : SpinResult(*weightTablePtr.pick(), spinVect)
    {
    }

    /**
     * @brief Choose a reelset, then spin the reels not set to true in lockedVect.
     *
     * If a reel is locked, it has its stop set to 0.
     *
     * @param weightTablePtr Reel set weight table to use.
     * @param lockedVect Lock state per reel.
     * @throws std::runtime_error if @p reelSet is null.
     */
    SpinResult(const ReelSetWeightTable& weightTablePtr, const std::vector<bool>& lockedVect)
        : SpinResult(*weightTablePtr.pick(), lockedVect)
    {
    }

    /**
     * @brief Sets reel stop positions for all reels.
     *
     * Replaces the current reel stops with the provided vector of stop positions.
     *
     * @param stopVect Vector containing the stop positions for each reel.
     */
    void setReelStops(const std::vector<size_t>& stopVect);
};
