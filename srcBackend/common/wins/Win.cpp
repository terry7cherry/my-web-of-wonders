#include "../headers/pch.h"
#include "Win.h"

unsigned int Win::applyMultiplier(const unsigned int newMultiplier,
                                  const MultipliersCombineType combineType)
{
    if (newMultiplier <= 0)
        reportError("Win.applyMultiplier", "Passed multiplier <= 0");
    else if (multiplier <= 0)
        reportError("Win.applyMultiplier", "Win multiplier <= 0");
    else
    {
        // update the win multiplier and win
        if (combineType == MultipliersCombineType::add)
            multiplier = (multiplier <= 1)
                             ? newMultiplier
                             : multiplier + newMultiplier;
        else if (combineType == MultipliersCombineType::multiply)
            multiplier *= newMultiplier;
        totalWin = win * multiplier;
    }
    return totalWin;
}
