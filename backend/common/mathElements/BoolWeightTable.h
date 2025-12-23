#pragma once
#include "WeightTable.h"

class BoolWeightTable : public WeightTable
{
public:
    const std::vector<bool> boolVect;

    BoolWeightTable(const std::vector<WeightData>& weightVector, const std::vector<bool>& boolVect,
                    const std::string& sz = "")
        : WeightTable(weightVector, sz),
          boolVect(boolVect)
    {
    }

    BoolWeightTable(const std::vector<int>& weightVector, const std::vector<bool>& boolVect,
                    const std::string& sz = "")
        : WeightTable(weightVector, sz),
          boolVect(boolVect)
    {
    }

    bool pick() const { return boolVect[WeightTable::pick()]; }
};
