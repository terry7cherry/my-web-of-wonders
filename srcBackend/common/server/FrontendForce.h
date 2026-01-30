#pragma once
#include <string>
#include "../../thisTitle/state/GameCycle.h"

struct FrontendForce
{
    const std::string name;
    const std::function<bool(const GameCycle&)> condition;
    const std::vector<std::string> betTypes;
};
