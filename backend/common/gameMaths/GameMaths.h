#pragma once
#include <string>
#include <unordered_map>
#include "../mathElements/WeightTables.h"
#include "../mathElements/PayLines.h"
#include "../mathElements/PayTable.h"
#include "../mathElements/ThresholdTable.h"
//#include "caching/TypeConfig.h"

template <typename T>
using TypeConfig = std::unordered_map<std::string, T>;

class GameMathsNew
{
public:
    TypeConfig<int> ints;
    TypeConfig<double> doubles;
    TypeConfig<std::string> strings;
    TypeConfig<bool> bools;
    TypeConfig<std::unordered_map<std::string, int>> intMaps;
    TypeConfig<std::unordered_map<std::string, double>> doubleMaps;
    TypeConfig<std::unordered_map<std::string, std::string>> stringMaps;
    TypeConfig<std::unordered_map<std::string, bool>> boolMaps;
    TypeConfig<WeightTable> intWeightTables;
    TypeConfig<StringWeightTable> stringWeightTables;
    TypeConfig<DoubleWeightTable> doubleWeightTables;
    TypeConfig<BoolWeightTable> boolWeightTables;
    TypeConfig<ReelSet> reelSets;
    TypeConfig<ReelSetWeightTable> reelSetWeights;
    TypeConfig<PayTable> payTables;
    TypeConfig<PayLines> payLines;
    TypeConfig<ThresholdTable<int>> intThresholdTables;
};
