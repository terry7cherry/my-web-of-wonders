#pragma once
#include "WeightTable.h"

class StringWeightTable : public WeightTable
{
public:
    const std::vector<std::string> stringVect;

    StringWeightTable(const std::vector<WeightData>& weightVector, const std::vector<std::string>& stringVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          stringVect(stringVect)
    {
    }

    StringWeightTable(const std::vector<int>& weightVector, const std::vector<std::string>& stringVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          stringVect(stringVect)
    {
    }

    std::string pick() const { return stringVect[WeightTable::pick()]; }
};
