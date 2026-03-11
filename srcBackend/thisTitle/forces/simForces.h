#pragma once
#include <string>
#include <vector>

namespace Forces
{
    enum SimForce
    {
        ForceNone,
        ForceFreeSpins,
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
        "Free Spins",
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
