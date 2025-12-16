#pragma once
#include "ReelSet.h"
#include "WeightTable.h"

class ReelSetWeightTable : public WeightTable {
public:
    const std::vector<const ReelSet *> reelSetVect;

    ReelSetWeightTable(const std::vector<WeightData> &weightVector, const std::vector<const ReelSet *> &reelSetVect,
                       const std::string &sz = "")
        : WeightTable(weightVector, sz),
          reelSetVect(reelSetVect) {
    }

    ReelSetWeightTable(const std::vector<int> &weightVector, const std::vector<const ReelSet *> &reelSetVect,
                       const std::string &sz = "")
        : WeightTable(weightVector, sz),
          reelSetVect(reelSetVect) {
    }

    const ReelSet *pick() const { return reelSetVect[WeightTable::pick()]; }
};
