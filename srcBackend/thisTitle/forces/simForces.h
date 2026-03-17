#pragma once
#include <string>
#include <vector>

namespace Forces
{
    enum SimForce
    {
        ForceNone,
        ForceStandardFreeSpins,
        ForceEnhancedFreeSpins1,
        ForceEnhancedFreeSpins2,
        ForceEnhancedFreeSpins3,
        ForceEnhancedFreeSpinsRandom,
        // Add additional forces here.
        ForceBaseGameA,
        ForceBaseGameB,
        ForceBaseGameC,
        ForceBaseGameD,
        ForceBaseGameE,
        ForceBaseGameF,
        ForceBaseGameG,
        ForceBaseGameH,
        NumForces,
    };

    inline std::vector<std::string> simForceNames{
        "None",
        "Standard Free Spins",
        "Enhanced Free Spins 1",
        "Enhanced Free Spins 2",
        "Enhanced Free Spins 3",
        "Enhanced Free Spins Random",
        "Base Reelset A",
        "Base Reelset B",
        "Base Reelset C",
        "Base Reelset D",
        "Base Reelset E",
        "Base Reelset F",
        "Base Reelset G",
        "Base Reelset H",
        // Add names of additional forces here.
    };
}
