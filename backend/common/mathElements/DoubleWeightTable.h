#pragma once
#include "ReelSet.h"
#include "WeightTable.h"

class DoubleWeightTable : public WeightTable
{
public:
    const std::vector<double> doubleVect;

    DoubleWeightTable(const std::vector<WeightData>& weightVector, const std::vector<double>& doubleVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          doubleVect(doubleVect)
    {
    }

    DoubleWeightTable(const std::vector<int>& weightVector, const std::vector<double>& doubleVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          doubleVect(doubleVect)
    {
    }

    double pick() const { return doubleVect[WeightTable::pick()]; }
};
