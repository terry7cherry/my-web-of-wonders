#pragma once

#include "PayGrid.h"

class HoldAndSpinGrid : public PayGrid
{
public:
    HoldAndSpinGrid(size_t reels, size_t rows, const PayTable& payTable);

    explicit HoldAndSpinGrid(const GameMaths& maths);

    unsigned int numUnlockedPositions;

    void reset();
};
