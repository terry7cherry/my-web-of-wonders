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
        NumForces,
    };

    inline std::vector<std::string> simForceNames{
        "None",
        "Free Spins"
        // Add names of additional forces here.
    };
}
