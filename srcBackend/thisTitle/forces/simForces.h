#pragma once
#include <string>

namespace SimForces
{
    enum Force
    {
        ForceNone,
        ForceFreeSpins,
    };

    inline std::vector<std::string> forceNames{
        "None",
        "Free Spins"
    };
}
