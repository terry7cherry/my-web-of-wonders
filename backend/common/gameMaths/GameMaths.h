#pragma once
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <string>
#include <unordered_map>
#include "../mathElements/WeightTables.h"
#include "../mathElements/PayLines.h"
#include "../mathElements/PayTable.h"
#include "../mathElements/ThresholdTable.h"
#include <absl/container/flat_hash_map.h>
#include "caching/TypeConfig.h"

//template <typename T>
//using TypeConfig = absl::flat_hash_map<std::string, T>;
//using TypeConfig = std::unordered_map<std::string, T>;

class GameMaths
{
public:
    TypeConfig<int> ints{{"int"}};
    TypeConfig<double> doubles{{"double"}};
    TypeConfig<std::string> strings{{"string"}};
    TypeConfig<bool> bools{{"bool"}};
    TypeConfig<std::unordered_map<std::string, int>> intMaps{{"intMap"}};
    TypeConfig<std::unordered_map<std::string, double>> doubleMaps{{"doubleMap"}};
    TypeConfig<std::unordered_map<std::string, std::string>> stringMaps{{"stringMap"}};
    TypeConfig<std::unordered_map<std::string, bool>> boolMaps{{"boolMap"}};
    TypeConfig<WeightTable> intWeightTables{{"intWeightTable", "weightTable"}};
    TypeConfig<WeightTable>& weightTables = intWeightTables;
    TypeConfig<StringWeightTable> stringWeightTables{{"stringWeightTable"}};
    TypeConfig<DoubleWeightTable> doubleWeightTables{{"doubleWeightTable"}};
    TypeConfig<BoolWeightTable> boolWeightTables{{"boolWeightTable"}};
    TypeConfig<ReelSet> reelSets{{"reelSet"}};
    TypeConfig<ReelSetWeightTable> reelSetWeights{{"reelSetWeights"}};
    TypeConfig<PayTable> payTables{{"payTable"}};
    TypeConfig<PayLines> payLines{{"payLines"}};
    TypeConfig<ThresholdTable<int>> intThresholdTables{{"intThresholdTable", "thresholdTable", "awardTable"}};
    TypeConfig<ThresholdTable<int>>& thresholdTables = intThresholdTables;
    TypeConfig<ThresholdTable<int>>& awardTables = intThresholdTables;
    TypeConfig<ThresholdTable<int>>& awards = intThresholdTables;
};
