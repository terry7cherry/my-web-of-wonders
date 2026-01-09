#pragma once
#include <xlnt/xlnt.hpp>
#include <algorithm>
#include <ranges>

#include "TaggedRange.h"
#include "TaggedDataTypes.h"
#include "../GameMaths.h"
#include <absl/container/flat_hash_map.h>
#include "../caching/TypeConfig.h"
//template <typename T>
//using TypeConfig = absl::flat_hash_map<std::string, T>;
//using TypeConfig = std::unordered_map<std::string, T>;


class TaggedSheet
{
public:
    std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<GameMaths>>> gameMathsVariants;
    std::vector<int> rtps;
    std::vector<std::string> betTypes;
    std::string gameTitle;
    std::string gameVersion;

    TaggedSheet(const std::string& parSheetName)
    {
        xlnt::workbook wb;
        const std::filesystem::path p = std::filesystem::path(GAME_SOURCE_DIR) / "backend" / "thisTitle" /
            "parSheets" / parSheetName;
        wb.load(p);
        extractTaggedRanges(wb);
        sortTaggedRangesByDescendingSpecificity();
        createAndCategoriseTaggedData();
        createGameMaths();
        std::string testString = "test";
    }

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

    static bool compareSpecificity(const std::shared_ptr<TaggedRange>& i,
                                   const std::shared_ptr<TaggedRange>& j)
    {
        return (i->specificity > j->specificity);
    }

    void sortTaggedRangesByDescendingSpecificity()
    {
        std::ranges::sort(taggedRanges, compareSpecificity);
    }

    void createAndCategoriseTaggedData()
    {
        createSimpleTaggedData();
        createReelSets();
        createReelSetWeights();
    }

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
                        betTypes.push_back(betType);
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

    void createReelSets()
    {
        const size_t windowHeight = getWindowHeight();

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

                const TaggedReelSet taggedReelSet(taggedRange, windowHeight);
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

    void createGameMaths()
    {
        for (const auto& betType : taggedBetTypes[0]->value | std::views::keys)
        {
            gameMathsVariants[betType] = std::unordered_map<int, std::unique_ptr<GameMaths>>();
            for (const auto& rtp : taggedRtpMaps[0]->value | std::views::keys)
            {
                auto gameMathsPtr = std::make_unique<GameMaths>();
                addRelevantElementsToConfigForType(taggedInts, gameMathsPtr->ints, rtp, betType);
                addRelevantElementsToConfigForType(taggedDoubles, gameMathsPtr->doubles, rtp, betType);
                addRelevantElementsToConfigForType(taggedStrings, gameMathsPtr->strings, rtp, betType);
                addRelevantElementsToConfigForType(taggedBools, gameMathsPtr->bools, rtp, betType);
                addRelevantElementsToConfigForType(taggedIntMaps, gameMathsPtr->intMaps, rtp, betType);
                addRelevantElementsToConfigForType(taggedDoubleMaps, gameMathsPtr->doubleMaps, rtp, betType);
                addRelevantElementsToConfigForType(taggedStringMaps, gameMathsPtr->stringMaps, rtp, betType);
                addRelevantElementsToConfigForType(taggedBoolMaps, gameMathsPtr->boolMaps, rtp, betType);
                addRelevantElementsToConfigForType(taggedPayTables, gameMathsPtr->payTables, rtp, betType);
                addRelevantElementsToConfigForType(taggedIntWeightTables, gameMathsPtr->intWeightTables, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedReelSetWeights, gameMathsPtr->reelSetWeights, rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedDoubleWeightTables,
                                                   gameMathsPtr->doubleWeightTables,
                                                   rtp, betType);
                addRelevantElementsToConfigForType(taggedBoolWeightTables, gameMathsPtr->boolWeightTables,
                                                   rtp,
                                                   betType);
                addRelevantElementsToConfigForType(taggedReelSets, gameMathsPtr->reelSets, rtp, betType);
                addRelevantElementsToConfigForType(taggedPayLines, gameMathsPtr->payLines, rtp, betType);
                addRelevantElementsToConfigForType(taggedIntThresholdTables, gameMathsPtr->intThresholdTables,
                                                   rtp, betType);
                gameMathsVariants[betType][rtp] = std::move(gameMathsPtr);
            }
        }
    }

    template <typename T, typename VectorType>
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
            if (!targetBetTypes.empty() && std::ranges::find(targetBetTypes, betType) == targetBetTypes.end())
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

private
:
    std::vector<std::shared_ptr<TaggedRange>> taggedRanges;
    std::vector<std::shared_ptr<TaggedInt>> taggedInts;
    std::vector<std::shared_ptr<TaggedDouble>> taggedDoubles;
    std::vector<std::shared_ptr<TaggedString>> taggedStrings;
    std::vector<std::shared_ptr<TaggedBool>> taggedBools;
    std::vector<std::shared_ptr<TaggedIntMap>> taggedIntMaps;
    std::vector<std::shared_ptr<TaggedDoubleMap>> taggedDoubleMaps;
    std::vector<std::shared_ptr<TaggedStringMap>> taggedStringMaps;
    std::vector<std::shared_ptr<TaggedBoolMap>> taggedBoolMaps;
    std::vector<std::shared_ptr<TaggedBetTypeMap>> taggedBetTypes;
    std::vector<std::shared_ptr<TaggedRtpMap>> taggedRtpMaps;
    std::vector<std::shared_ptr<TaggedIntWeightTable>> taggedIntWeightTables;
    std::vector<std::shared_ptr<TaggedStringWeightTable>> taggedStringWeightTables;
    std::vector<std::shared_ptr<TaggedDoubleWeightTable>> taggedDoubleWeightTables;
    std::vector<std::shared_ptr<TaggedBoolWeightTable>> taggedBoolWeightTables;
    std::vector<std::shared_ptr<TaggedReelSet>> taggedReelSets;
    std::vector<std::shared_ptr<TaggedPayTable>> taggedPayTables;
    std::unordered_map<std::string, ReelSet> reelSetMap;
    std::vector<std::shared_ptr<TaggedReelSetWeights>> taggedReelSetWeights;
    std::vector<std::shared_ptr<TaggedPayLines>> taggedPayLines;
    std::vector<std::shared_ptr<TaggedIntThresholdTable>> taggedIntThresholdTables;
};
