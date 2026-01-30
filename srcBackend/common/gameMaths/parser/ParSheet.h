#pragma once
/**
 * @file ParSheet.h
 * @brief Parses a spreadsheet par sheet into tagged ranges and typed game maths structures.
 *
 * `ParSheet` loads an Excel workbook, discovers tagged ranges (see `TaggedRange`), sorts them by
 * specificity, converts them into typed structures (see `TaggedDataTypes.h`), and finally assembles
 * `GameMaths` variants per bet type and RTP.
 */
#include <xlnt/xlnt.hpp>
#include <algorithm>
#include <ranges>
#include <json/json.h>

#include "TaggedRange.h"
#include "TaggedDataTypes.h"
#include "../GameMaths.h"
#include <absl/container/flat_hash_map.h>
#include "../TypeConfig.h"
#include "../../utils/Timer.h"
//template <typename T>
//using TypeConfig = absl::flat_hash_map<std::string, T>;
//using TypeConfig = std::unordered_map<std::string, T>;


/**
 * @brief High-level parser that builds `GameMaths` variants from a par sheet workbook.
 */
class ParSheet
{
public:
    std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<GameMaths>>> gameMathsVariants;
    std::vector<int> rtps;
    std::vector<std::string> betTypes;
    std::vector<double> betTypeCosts;
    std::string fileName;
    std::string gameTitle;
    std::string gameVersion;

    /**
     * @brief Construct and parse the specified par sheet file from the project's parSheets folder.
     * @param parSheetName Filename of the par sheet (e.g., `Example.xlsx`), which must be stored
     * in srcBackend/thisTitle/parSheets.
     */
    ParSheet(const std::string& parSheetName)
        : fileName(parSheetName)
    {
        Timer timer("Spreadsheet Parsing");
        xlnt::workbook wb;
        const std::filesystem::path p = std::filesystem::path(GAME_SOURCE_DIR) / "srcBackend" / "thisTitle" /
            "parSheets" / fileName;
        wb.load(p);
        extractTaggedRanges(wb);
        sortTaggedRangesByDescendingSpecificity();
        createAndCategoriseTaggedData();
        createGameMaths();
        timer.outputMilli();
    }

    /**
     * @brief Discover tagged ranges across all worksheets.
     *
     * A tagged range is identified by a merged single-row header that contains a JSON object.
     */
    void extractTaggedRanges(const xlnt::workbook& wb)
    {
        const size_t numSheets = wb.sheet_count();
        for (size_t sheetIndex = 0; sheetIndex < numSheets; sheetIndex++)
        {
            xlnt::worksheet sheet = wb.sheet_by_index(sheetIndex);
            std::vector<xlnt::range_reference> mergedRanges = sheet.merged_ranges();
            for (auto& range : mergedRanges)
            {
                if (range.height() != 1) continue;
                const std::string value = sheet.cell(range.top_left()).value<std::string>();
                const bool looksLikeJson = value.starts_with("{") && value.ends_with("}");
                if (looksLikeJson)
                {
                    Json::Value jsonTag = parseJsonObjectOrReturnNull(value);
                    if (jsonTag.isNull())
                    {
                        std::cerr << "The value in cell '" + sheet.title() + "'!" + range.to_string() +
                            " looks as though it may be a JSON object but it is malformed." << std::endl;
                        std::cerr <<
                            "\tIf this is data you wish to parse, ensure all property keys and any"
                            "string values are enclosed in quotes."
                            << std::endl;
                    }
                    else
                        taggedRanges.push_back(std::make_shared<TaggedRange>(sheet, range, jsonTag));
                }
            }
        }
    }

    /**
     * @brief Sort helper: order ranges by descending specificity (betTypes/rtps present first).
     */
    static bool compareSpecificity(const std::shared_ptr<TaggedRange>& i,
                                   const std::shared_ptr<TaggedRange>& j)
    {
        return (i->specificity > j->specificity);
    }

    /**
     * @brief Sort internal tagged ranges by descending specificity.
     */
    void sortTaggedRangesByDescendingSpecificity()
    {
        std::ranges::sort(taggedRanges, compareSpecificity);
    }

    /**
     * @brief Populate typed tagged-data collections from discovered ranges.
     */
    void createAndCategoriseTaggedData()
    {
        createSimpleTaggedData();
        createReelSets();
        createSymbols();
        createReelSetWeights();
    }

    /**
     * @brief Extract simple scalar and map data (metadata, betTypes, rtps, ints, doubles, etc.).
     * @throws std::runtime_error if required metadata (title/version) or uniqueness constraints fail.
     */
    void createSimpleTaggedData()
    {
        bool foundTitle = false;
        bool foundVersion = false;
        for (const std::shared_ptr<TaggedRange>& taggedRange : taggedRanges)
        {
            if (taggedRange->type == "metadata")
            {
                if (taggedRange->specificity)
                    throw std::runtime_error(
                        "Metadata tag must be global and must not specify RTP or betType.");
                else
                {
                    std::vector<Metadata> metadata;
                    for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                    {
                        metadata.push_back(Metadata(taggedRange, row));
                    }
                    for (const auto& dataPoint : metadata)
                    {
                        if (dataPoint.fullName == "gameTitle")
                        {
                            foundTitle = true;
                            gameTitle = dataPoint.value;
                        }
                        if (dataPoint.fullName == "gameVersion")
                        {
                            foundVersion = true;
                            gameVersion = dataPoint.value;
                        }
                    }
                }
            }
            if (taggedRange->type == "betTypes")
            {
                if (taggedBetTypes.empty())
                {
                    taggedBetTypes.push_back(std::make_shared<TaggedBetTypeMap>(taggedRange));
                    for (const auto& betType : taggedBetTypes[0]->value | std::views::keys)
                    {
                        betTypes.push_back(betType);
                        betTypeCosts.push_back(taggedBetTypes[0]->value.at(betType));
                    }
                }
                else
                    throw std::runtime_error(
                        "Multiple betTypes tables found at " + taggedRange->location + " and " +
                        taggedBetTypes[0]->taggedRangePtr->location);
            }
            else if (taggedRange->type == "rtps")
            {
                if (taggedRtpMaps.empty())
                {
                    taggedRtpMaps.push_back(std::make_shared<TaggedRtpMap>(taggedRange));
                    for (const auto& rtp : taggedRtpMaps[0]->value | std::views::keys)
                        rtps.push_back(rtp);
                }
                else
                {
                    throw std::runtime_error(
                        "Multiple rtps tables found at " + taggedRange->location + " and " +
                        taggedBetTypes[0]->taggedRangePtr->location);
                }
            }
            else if (taggedRange->type == "int")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedInts.push_back(
                        std::make_shared<TaggedInt>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "double")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedDoubles.push_back(std::make_shared<TaggedDouble>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "string")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedStrings.push_back(std::make_shared<TaggedString>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "bool")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedBools.push_back(std::make_shared<TaggedBool>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "intMap")
            {
                taggedIntMaps.push_back(std::make_shared<TaggedIntMap>(taggedRange));
            }
            else if (taggedRange->type == "doubleMap")
            {
                taggedDoubleMaps.push_back(std::make_shared<TaggedDoubleMap>(taggedRange));
            }
            else if (taggedRange->type == "stringMap")
            {
                taggedStringMaps.push_back(std::make_shared<TaggedStringMap>(taggedRange));
            }
            else if (taggedRange->type == "boolMap")
            {
                taggedBoolMaps.push_back(std::make_shared<TaggedBoolMap>(taggedRange));
            }
            else if (taggedRange->type == "weightTable" || taggedRange->type == "intWeightTable")
            {
                taggedIntWeightTables.push_back(
                    std::make_shared<TaggedIntWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "doubleWeightTable")
            {
                taggedDoubleWeightTables.push_back(
                    std::make_shared<TaggedDoubleWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "stringWeightTable")
            {
                taggedStringWeightTables.push_back(
                    std::make_shared<TaggedStringWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "boolWeightTable")
            {
                taggedBoolWeightTables.push_back(
                    std::make_shared<TaggedBoolWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "payTable")
            {
                taggedPayTables.push_back(
                    std::make_shared<TaggedPayTable>(taggedRange));
            }
            else if (taggedRange->type == "payLines")
            {
                taggedPayLines.push_back(
                    std::make_shared<TaggedPayLines>(taggedRange));
            }
            else if (taggedRange->type == "intThresholdTable"
                || taggedRange->type == "thresholdTable"
                || taggedRange->type == "awardTable"
                || taggedRange->type == "awards")
            {
                taggedIntThresholdTables.push_back(
                    std::make_shared<TaggedIntThresholdTable>(taggedRange));
            }
        }
        if (!foundTitle)
            throw std::runtime_error(
                "No gameTitle found in metadata. Par sheet must include a tagged range with type \"metadata\" and two values: gameTitle and gameVersion.");
        if (!foundVersion)
            throw std::runtime_error(
                "No gameVersion found in metadata. Par sheet must include a tagged range with type \"metadata\" and two values: gameTitle and gameVersion.");
    }

    /**
     * @brief Build `ReelSet` objects from corresponding ranges.
     * @throws std::runtime_error if specificity constraints or name collisions are violated.
     */
    void createReelSets()
    {
        const size_t windowHeight = getWindowHeight();
        const std::unordered_map<std::string, Symbol> symbolIndexMap = taggedPayTables[0]->value.
            symbolIndexMap;

        for (const std::shared_ptr<TaggedRange>& taggedRange : taggedRanges)
        {
            if (taggedRange->type == "reelSet")
            {
                if (taggedRange->specificity > 0)
                    throw std::runtime_error(
                        "Reel Set '" + taggedRange->name +
                        "' has rtp and/or betType specified. Reel Sets cannot vary across math "
                        "variants. To achieve this, create multiple reelsets with distinct names "
                        "and specify distinct reelSetWeightTable where required.");

                const TaggedReelSet taggedReelSet(taggedRange, windowHeight, symbolIndexMap);
                if (const auto& identicalReelSetIt = reelSetMap.find(taggedReelSet.fullName);
                    identicalReelSetIt != reelSetMap.end())
                    throw std::runtime_error(
                        "Reel Set at " + taggedReelSet.taggedRangePtr->location +
                        " has name matching existing reelSet. All Reel Sets must have distinct"
                        "names.");

                reelSetMap.insert({taggedReelSet.fullName, taggedReelSet.value});
                taggedReelSets.push_back(
                    std::make_shared<TaggedReelSet>(taggedReelSet));
            }
        }
    }

    /**
     * @brief Parse symbol definitions, symbol vectors, maps, and symbol weight tables.
     */
    void createSymbols()
    {
        const std::unordered_map<std::string, Symbol> symbolIndexMap = taggedPayTables[0]->value.
            symbolIndexMap;

        for (const std::shared_ptr<TaggedRange>& taggedRange : taggedRanges)
        {
            if (taggedRange->type == "symbol")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedSymbols.push_back(
                        std::make_shared<TaggedSymbol>(taggedRange, row, symbolIndexMap));
                }
            }
            else if (taggedRange->type == "symbolVector")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedSymbolVectors.push_back(
                        std::make_shared<TaggedSymbolVector>(taggedRange, row, symbolIndexMap));
                }
            }
            else if (taggedRange->type == "symbolMap")
            {
                taggedSymbolMaps.push_back(
                    std::make_shared<TaggedSymbolMap>(taggedRange, symbolIndexMap));
            }
            else if (taggedRange->type == "symbolWeightTable")
            {
                taggedSymbolWeightTables.push_back(
                    std::make_shared<TaggedSymbolWeightTable>(taggedRange, symbolIndexMap));
            }
        }
    }

    /**
     * @brief Parse reel set weight tables.
     */
    void createReelSetWeights()
    {
        for (const std::shared_ptr<TaggedRange>& taggedRange : taggedRanges)
        {
            if (taggedRange->type == "reelSetWeightTable"
                || taggedRange->type == "reelSetWeights"
                || taggedRange->type == "reelSetWeight")
            {
                taggedReelSetWeights.push_back(
                    std::make_unique<TaggedReelSetWeights>(taggedRange, reelSetMap)
                );
            }
        }
    }

    /**
     * @brief Assemble `GameMaths` variants for each bet type and RTP, selecting the most specific
     *        tagged values available per item.
     */
    void createGameMaths()
    {
        for (const auto& betType : taggedBetTypes[0]->value | std::views::keys)
        {
            gameMathsVariants[betType] = std::unordered_map<int, std::unique_ptr<GameMaths>>();
            for (const auto& rtp : taggedRtpMaps[0]->value | std::views::keys)
            {
                auto gameMathsPtr = std::make_unique<GameMaths>();
                gameMathsPtr->gameTitle = gameTitle;
                gameMathsPtr->betTypeCost = taggedBetTypes[0]->value.at(betType);
                gameMathsPtr->targetRTP = taggedRtpMaps[0]->value.at(rtp);
                gameMathsPtr->rtp = rtp;
                gameMathsPtr->betType = betType;
                addRelevantElementsToConfigForType(taggedSymbols, gameMathsPtr->symbols, rtp, betType);
                addRelevantElementsToConfigForType(taggedSymbolVectors, gameMathsPtr->symbolVectors, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedInts, gameMathsPtr->ints, rtp, betType);
                addRelevantElementsToConfigForType(taggedDoubles, gameMathsPtr->doubles, rtp, betType);
                addRelevantElementsToConfigForType(taggedStrings, gameMathsPtr->strings, rtp, betType);
                addRelevantElementsToConfigForType(taggedBools, gameMathsPtr->bools, rtp, betType);
                addRelevantElementsToConfigForType(taggedIntMaps, gameMathsPtr->intMaps, rtp, betType);
                addRelevantElementsToConfigForType(taggedDoubleMaps, gameMathsPtr->doubleMaps, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedSymbolMaps, gameMathsPtr->symbolMaps, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedStringMaps, gameMathsPtr->stringMaps, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedBoolMaps, gameMathsPtr->boolMaps, rtp, betType);
                addRelevantElementsToConfigForType(taggedPayTables, gameMathsPtr->payTables, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedIntWeightTables, gameMathsPtr->intWeightTables,
                                                   rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedReelSetWeights, gameMathsPtr->reelSetWeights,
                                                   rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedDoubleWeightTables,
                                                   gameMathsPtr->doubleWeightTables,
                                                   rtp, betType);
                addRelevantElementsToConfigForType(taggedSymbolWeightTables,
                                                   gameMathsPtr->symbolWeightTables,
                                                   rtp, betType);
                addRelevantElementsToConfigForType(taggedBoolWeightTables, gameMathsPtr->boolWeightTables,
                                                   rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedReelSets, gameMathsPtr->reelSets, rtp, betType);
                addRelevantElementsToConfigForType(taggedPayLines, gameMathsPtr->payLines, rtp, betType);
                addRelevantElementsToConfigForType(taggedIntThresholdTables,
                                                   gameMathsPtr->intThresholdTables,
                                                   rtp, betType);
                validateBetTypeCost(gameMathsPtr->betTypeCost, gameMathsPtr->ints["minBet"]);
                gameMathsVariants[betType][rtp] = std::move(gameMathsPtr);
            }
        }
    }

    template
    <typename T, typename VectorType>

    /**
     * @brief Insert the most specific elements for the given betType/RTP into a destination map.
     * @tparam T Value type stored in the destination `TypeConfig`.
     * @tparam VectorType Container of shared_ptr<TaggedData<T>>-like elements with `fullName`,
     *         `value`, and `taggedRangePtr`.
     * @param sourceElements Candidate elements with varying specificity.
     * @param destinationMap Output map keyed by `fullName`.
     * @param rtp Target RTP.
     * @param betType Target bet type.
     * @throws std::runtime_error on specificity tie for the same key.
     */
    void addRelevantElementsToConfigForType(const VectorType& sourceElements,
                                            TypeConfig<T>& destinationMap,
                                            const int rtp,
                                            const std::string& betType)
    {
        std::unordered_map<std::string, std::shared_ptr<TaggedData<T>>> intermediaryMap;

        for (const auto& elementPtr : sourceElements)
        {
            const auto& targetRtps = elementPtr->taggedRangePtr->rtps;
            if (!targetRtps.empty() && std::ranges::find(targetRtps, rtp) == targetRtps.end())
                continue;
            const auto& targetBetTypes = elementPtr->taggedRangePtr->betTypes;
            if (!targetBetTypes.empty() && std::ranges::find(targetBetTypes, betType) == targetBetTypes.
                end())
                continue;

            if (intermediaryMap.contains(elementPtr->fullName))
            {
                const auto existingElementPtr = intermediaryMap[elementPtr->fullName];
                if (existingElementPtr->taggedRangePtr->specificity == elementPtr->taggedRangePtr->
                    specificity)
                {
                    throw std::runtime_error(
                        "Conflicting " + elementPtr->taggedRangePtr->type + " tags for bet type " +
                        betType +
                        " and RTP " + std::to_string(rtp) + "found in spreadsheet at:\n" +
                        "\t" + elementPtr->taggedRangePtr->location + "\n" +
                        "\t" + existingElementPtr->taggedRangePtr->location + "."
                    );
                }
            }
            else
            {
                if (elementPtr->fullName.empty())
                    intermediaryMap.emplace("default", elementPtr);
                else
                    intermediaryMap.emplace(elementPtr->fullName, elementPtr);
            }
        }

        destinationMap.reserve(intermediaryMap.size());
        for (const auto& [name, elementPtr] : intermediaryMap)
        {
            destinationMap.insert({name, elementPtr->value});
        }
    }


    /**
     * @brief Get `numRows` from tagged ints, used as the window height for reel sets.
     * @throws std::runtime_error if `numRows` is not provided.
     */
    size_t getWindowHeight()
    {
        const auto it = std::ranges::max_element(taggedInts,
                                                 [](const std::shared_ptr<TaggedInt>& a,
                                                    const std::shared_ptr<TaggedInt>& b)
                                                 {
                                                     const bool aIsNumRows = (a->fullName == "numRows");
                                                     const bool bisNumRows = (b->fullName == "numRows");

                                                     if (aIsNumRows != bisNumRows)
                                                         return bisNumRows;
                                                     // non-"numRows" is always smaller

                                                     if (!aIsNumRows) // neither is "numRows"
                                                         return false;

                                                     return a->value < b->value;
                                                 });

        if (it == taggedInts.end() || (*it)->fullName != "numRows")
        {
            throw std::runtime_error(
                "Window height not found. PAR sheet must provide numRows of int type.");
        }

        return (*it)->value;
    }

    /**
     * @brief Parse a JSON object string. Returns null JSON value on failure.
     */
    static Json::Value parseJsonObjectOrReturnNull(const std::string& input)
    {
        static const std::unique_ptr<Json::CharReader> reader([]
        {
            Json::CharReaderBuilder builder;
            builder["collectComments"] = false;
            return std::unique_ptr<Json::CharReader>(builder.newCharReader());
        }());

        Json::Value root;
        std::string errors;

        const char* begin = input.data();
        const char* end = begin + input.size();

        if (!reader->parse(begin, end, &root, &errors))
        {
            return {};
        }

        if (!root.isObject())
        {
            return {};
        }

        return root;
    }

    /**
     * @brief Validate that `cost * minBet` is an integer.
     * @throws std::runtime_error if the constraint is violated.
     */
    static void validateBetTypeCost(const double cost, const int minBet)
    {
        // Check if cost * minbet is integer.
        double intPart;
        if (std::modf(cost * minBet, &intPart) != 0.0)
        {
            throw std::runtime_error(
                "Bet type cost must be an integer multiple of minBet. Bet type cost: " +
                std::to_string(cost) + ", minBet: " + std::to_string(minBet));
        }
    }

private
:
    std::vector<std::shared_ptr<TaggedSymbol>> taggedSymbols;
    std::vector<std::shared_ptr<TaggedSymbolVector>> taggedSymbolVectors;
    std::vector<std::shared_ptr<TaggedRange>> taggedRanges;
    std::vector<std::shared_ptr<TaggedInt>> taggedInts;
    std::vector<std::shared_ptr<TaggedDouble>> taggedDoubles;
    std::vector<std::shared_ptr<TaggedString>> taggedStrings;
    std::vector<std::shared_ptr<TaggedBool>> taggedBools;
    std::vector<std::shared_ptr<TaggedIntMap>> taggedIntMaps;
    std::vector<std::shared_ptr<TaggedDoubleMap>> taggedDoubleMaps;
    std::vector<std::shared_ptr<TaggedSymbolMap>> taggedSymbolMaps;
    std::vector<std::shared_ptr<TaggedStringMap>> taggedStringMaps;
    std::vector<std::shared_ptr<TaggedBoolMap>> taggedBoolMaps;
    std::vector<std::shared_ptr<TaggedBetTypeMap>> taggedBetTypes;
    std::vector<std::shared_ptr<TaggedRtpMap>> taggedRtpMaps;
    std::vector<std::shared_ptr<TaggedIntWeightTable>> taggedIntWeightTables;
    std::vector<std::shared_ptr<TaggedStringWeightTable>> taggedStringWeightTables;
    std::vector<std::shared_ptr<TaggedDoubleWeightTable>> taggedDoubleWeightTables;
    std::vector<std::shared_ptr<TaggedSymbolWeightTable>> taggedSymbolWeightTables;
    std::vector<std::shared_ptr<TaggedBoolWeightTable>> taggedBoolWeightTables;
    std::vector<std::shared_ptr<TaggedReelSet>> taggedReelSets;
    std::vector<std::shared_ptr<TaggedPayTable>> taggedPayTables;
    std::unordered_map<std::string, ReelSet> reelSetMap;
    std::vector<std::shared_ptr<TaggedReelSetWeights>> taggedReelSetWeights;
    std::vector<std::shared_ptr<TaggedPayLines>> taggedPayLines;
    std::vector<std::shared_ptr<TaggedIntThresholdTable>> taggedIntThresholdTables;
};
