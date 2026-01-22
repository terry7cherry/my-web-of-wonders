#pragma once
#include <string>
#include "../../thisTitle/state/GameCycle.h"

struct FrontendForce
{
    const std::string name;
    std::function<bool(const GameCycle&)> condition;
};
