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
            stopIndexVect[reelNum] = mathsRng.randInt(
                static_cast<uint32>(reelSet.reelSizeVect[reelNum]),
                "reel stop - reel " + std::to_string(reelNum));
        }
}

SpinResult::SpinResult(const ReelSet& reelSet, const std::vector<size_t>& spinVect)
    : reelSet(reelSet)
{
    stopIndexVect.resize(reelSet.reelSizeVect.size(), 0);
    for (const size_t reelNum : spinVect)
    {
        stopIndexVect[reelNum] = mathsRng.
            randInt(static_cast<uint32>(reelSet.reelSizeVect[reelNum]),
                    "reel stop - reel " + std::to_string(reelNum) + " - spun reels specified");
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
            stopIndexVect[reelNum] = mathsRng.randInt(
                static_cast<uint32>(reelSet.reelSizeVect[reelNum]), "reel stop - reel " + std::to_string(reelNum) + " - locked reels specified");
        }
    }
}

void SpinResult::setReelStops(const std::vector<size_t>& stopVect)
{
    stopIndexVect = stopVect;
}

void SpinResult::setReelStop(const size_t reel, const size_t stop)
{
    stopIndexVect[reel] = stop;
}
