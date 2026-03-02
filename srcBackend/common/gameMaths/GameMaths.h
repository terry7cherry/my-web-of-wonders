#pragma once
/**
 * @file GameMaths.h
 * @brief Aggregated configuration for a single maths variant (betType, RTP).
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <string>
#include <unordered_map>
#include <json/json.h>
#include "mathElements/WeightTables.h"
#include "./mathElements/PayLines.h"
#include "./mathElements/PayTable.h"
#include "./mathElements/ThresholdTable.h"
#include <absl/container/flat_hash_map.h>
#include "TypeConfig.h"

//template <typename T>
//using TypeConfig = absl::flat_hash_map<std::string, T>;
//using TypeConfig = std::unordered_map<std::string, T>;

/**
 * @brief Holds typed config maps (ints, doubles, symbols, reels, tables) for a maths variant.
 */
class GameMaths
{
public:
    std::string gameTitle;
    double betTypeCost;
    int minBetCost;
    double targetRTP;
    int rtp;
    int minBet;
    std::string betType;
    TypeConfig<int> ints{{"int"}};
    TypeConfig<Symbol> symbols{{"symbol"}};
    TypeConfig<std::vector<Symbol>> symbolVectors{{"symbols"}};
    TypeConfig<double> doubles{{"double"}};
    TypeConfig<std::string> strings{{"string"}};
    TypeConfig<bool> bools{{"bool"}};
    TypeConfig<std::unordered_map<std::string, int>> intMaps{{"intMap"}};
    TypeConfig<std::unordered_map<std::string, double>> doubleMaps{{"doubleMap"}};
    TypeConfig<std::unordered_map<std::string, Symbol>> symbolMaps{{"symbolMap"}};
    TypeConfig<std::unordered_map<std::string, std::string>> stringMaps{{"stringMap"}};
    TypeConfig<std::unordered_map<std::string, bool>> boolMaps{{"boolMap"}};
    TypeConfig<WeightTable> intWeightTables{{"intWeightTable", "weightTable"}};
    TypeConfig<WeightTable>& weightTables = intWeightTables;
    TypeConfig<StringWeightTable> stringWeightTables{{"stringWeightTable"}};
    TypeConfig<DoubleWeightTable> doubleWeightTables{{"doubleWeightTable"}};
    TypeConfig<SymbolWeightTable> symbolWeightTables{{"symbolWeightTable"}};
    TypeConfig<BoolWeightTable> boolWeightTables{{"boolWeightTable"}};
    TypeConfig<std::vector<std::vector<int>>> intGrids{{"intGrid"}};
    TypeConfig<ReelSet> reelSets{{"reelSet"}};
    TypeConfig<ReelSetWeights> reelSetWeights{{"reelSetWeights"}};
    TypeConfig<PayTable> payTables{{"payTable"}};
    TypeConfig<PayLines> payLines{{"payLines"}};
    TypeConfig<ThresholdTable<int>> intThresholdTables{{"intThresholdTable", "thresholdTable", "awardTable"}};
    TypeConfig<ThresholdTable<int>>& thresholdTables = intThresholdTables;
    TypeConfig<ThresholdTable<int>>& awardTables = intThresholdTables;
    TypeConfig<ThresholdTable<int>>& awards = intThresholdTables;

    Json::Value toJson() const;
    static std::string getFrontendMathsSheetName();
};
