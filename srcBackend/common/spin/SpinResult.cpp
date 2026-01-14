#include "SpinResult.h"

#include <stdexcept>
#include "../gameMaths/mathElements/ReelSet.h"
#include "../rng/MathsRng.h"

SpinResult::SpinResult(const ReelSet& reelSet, const bool randomSpin)
    : reelSet(reelSet)
{
    stopIndexVect.resize(reelSet.reelSizeVect.size(), 0);
    if (randomSpin)
        for (size_t reelNum = 0; reelNum < stopIndexVect.size(); ++reelNum)
        {
            stopIndexVect[reelNum] = mathRng.randInt(
                static_cast<uint32>(reelSet.reelSizeVect[reelNum]));
        }
}

SpinResult::SpinResult(const ReelSet& reelSet, const std::vector<size_t>& spinVect)
    : reelSet(reelSet)
{
    stopIndexVect.resize(reelSet.reelSizeVect.size(), 0);
    for (const size_t reelNum : spinVect)
    {
        stopIndexVect[reelNum] = mathRng.
            randInt(static_cast<uint32>(reelSet.reelSizeVect[reelNum]));
    }
}

SpinResult::SpinResult(const ReelSet& reelSet, const std::vector<bool>& lockedVect)
    : reelSet(reelSet)
{
    stopIndexVect.resize(reelSet.reelSizeVect.size(), 0);

    const size_t reels = stopIndexVect.size();
    for (size_t reelNum = 0; reelNum < reels; ++reelNum)
    {
        const bool isLocked = (reelNum < lockedVect.size()) ? lockedVect[reelNum] : false;
        if (!isLocked)
        {
            stopIndexVect[reelNum] = mathRng.randInt(
                static_cast<uint32>(reelSet.reelSizeVect[reelNum]));
        }
    }
}

void SpinResult::setReelStops(const std::vector<size_t>& stopVect)
{
    stopIndexVect = stopVect;
}
