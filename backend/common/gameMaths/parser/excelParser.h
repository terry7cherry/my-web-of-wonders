#pragma once
#include <xlnt/xlnt.hpp>
#include "TaggedRange.h"
#include "TaggedDataTypes.h"

class TaggedSheet
{
public:
    std::vector<std::shared_ptr<TaggedRange>> taggedRanges;
    std::vector<std::unique_ptr<TaggedInt>> taggedInts;
    std::vector<std::unique_ptr<TaggedDouble>> taggedFloats;
    std::vector<std::unique_ptr<TaggedString>> taggedStrings;
    std::vector<std::unique_ptr<TaggedBool>> taggedBools;
    std::vector<std::unique_ptr<TaggedIntMap>> taggedIntMaps;
    std::vector<std::unique_ptr<TaggedDoubleMap>> taggedDoubleMaps;
    std::vector<std::unique_ptr<TaggedStringMap>> taggedStringMaps;
    std::vector<std::unique_ptr<TaggedBoolMap>> taggedBoolMaps;
    std::vector<std::unique_ptr<TaggedBetTypeMap>> taggedBetTypes;
    std::vector<std::unique_ptr<TaggedRtpMap>> taggedRtpMaps;
    std::vector<std::unique_ptr<TaggedIntWeightTable>> taggedIntWeightTables;
    std::vector<std::unique_ptr<TaggedStringWeightTable>> taggedStringWeightTables;
    std::vector<std::unique_ptr<TaggedDoubleWeightTable>> taggedDoubleWeightTables;
    std::vector<std::unique_ptr<TaggedBoolWeightTable>> taggedBoolWeightTables;
    std::unordered_map<std::string, ReelSet> reelSetMap;
    std::vector<std::unique_ptr<TaggedReelSetWeights>> taggedReelSetWeights;

    TaggedSheet()
    {
        xlnt::workbook wb;
        std::filesystem::path p = std::filesystem::path(GAME_SOURCE_DIR) / "backend" / "thisTitle" /
            "parSheets" / "test.xlsx";
        wb.load(p);
        extractTaggedRanges(wb);
        sortTaggedRangesByDescendingSpecificity();
        createAndCategoriseTaggedData();
        createGameMaths();
        std::string testString = "test";
    }

    void extractTaggedRanges(xlnt::workbook wb)
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
                Json::Value jsonTag = parseJsonObjectOrReturnNull(value);
                if (jsonTag.isNull()) continue;
                taggedRanges.push_back(std::make_shared<TaggedRange>(sheet, range, jsonTag));
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
        for (const std::shared_ptr<TaggedRange>& taggedRange : taggedRanges)
        {
            if (taggedRange->type == "betTypes")
            {
                if (taggedBetTypes.empty())
                    taggedBetTypes.push_back(
                        std::make_unique<TaggedBetTypeMap>(taggedRange));
                else
                    throw std::runtime_error(
                        "Multiple betTypes tables found at " + taggedRange->location + " and " +
                        taggedBetTypes[0]->taggedRangePtr->location);
            }
            else if (taggedRange->type == "rtps")
            {
                if (taggedRtpMaps.empty())
                    taggedRtpMaps.push_back(
                        std::make_unique<TaggedRtpMap>(taggedRange));
                else
                    throw std::runtime_error(
                        "Multiple rtps tables found at " + taggedRange->location + " and " +
                        taggedBetTypes[0]->taggedRangePtr->location);
            }
            else if (taggedRange->type == "int")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedInts.push_back(
                        std::make_unique<TaggedInt>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "double")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedFloats.push_back(std::make_unique<TaggedDouble>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "string")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedStrings.push_back(std::make_unique<TaggedString>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "bool")
            {
                for (xlnt::row_t row = 0; row < taggedRange->data.size(); row++)
                {
                    taggedBools.push_back(std::make_unique<TaggedBool>(taggedRange, row));
                }
            }
            else if (taggedRange->type == "intMap")
            {
                taggedIntMaps.push_back(std::make_unique<TaggedIntMap>(taggedRange));
            }
            else if (taggedRange->type == "doubleMap")
            {
                taggedDoubleMaps.push_back(std::make_unique<TaggedDoubleMap>(taggedRange));
            }
            else if (taggedRange->type == "stringMap")
            {
                taggedStringMaps.push_back(std::make_unique<TaggedStringMap>(taggedRange));
            }
            else if (taggedRange->type == "boolMap")
            {
                taggedBoolMaps.push_back(std::make_unique<TaggedBoolMap>(taggedRange));
            }
            else if (taggedRange->type == "weightTable" || taggedRange->type == "intWeightTable")
            {
                taggedIntWeightTables.push_back(
                    std::make_unique<TaggedIntWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "doubleWeightTable")
            {
                taggedDoubleWeightTables.push_back(
                    std::make_unique<TaggedDoubleWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "stringWeightTable")
            {
                taggedStringWeightTables.push_back(
                    std::make_unique<TaggedStringWeightTable>(taggedRange));
            }
            else if (taggedRange->type == "boolWeightTable")
            {
                taggedBoolWeightTables.push_back(
                    std::make_unique<TaggedBoolWeightTable>(taggedRange));
            }
        }
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

                reelSetMap.insert({taggedReelSet.fullName, std::move(taggedReelSet.value)});
            }
        }
    }

    void createReelSetWeights()
    {
        for (const std::shared_ptr<TaggedRange>& taggedRange : taggedRanges)
        {
            if (taggedRange->type == "reelSetWeightTable")
            {
                taggedReelSetWeights.push_back(
                    std::make_unique<TaggedReelSetWeights>(taggedRange, reelSetMap)
                );
            }
        }
    }

    void createGameMaths()
    {
        for (auto& [betType, cost] : taggedBetTypes[0]->value)
        {
            for (auto& [rtp, target] : taggedRtpMaps[0]->value)
            {
            }
        }
    }

    size_t getWindowHeight()
    {
        const auto it = std::ranges::max_element(taggedInts,
                                                 [](const std::unique_ptr<TaggedInt>& a,
                                                    const std::unique_ptr<TaggedInt>& b)
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
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;

        Json::Value root;
        std::string errors;

        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

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
};
