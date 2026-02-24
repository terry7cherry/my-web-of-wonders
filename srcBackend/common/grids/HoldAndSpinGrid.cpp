#include "HoldAndSpinGrid.h"

HoldAndSpinGrid::HoldAndSpinGrid(const size_t reels, const size_t rows, const PayTable& payTable)
    : PayGrid(reels, rows, payTable)
{
}

HoldAndSpinGrid::HoldAndSpinGrid(const GameMaths& maths)
    : HoldAndSpinGrid(maths.ints.at("numReels"), maths.ints.at("numRows"), maths.payTables.at("default")),
      numUnlockedPositions(maxReels * maxRows)
{
}


