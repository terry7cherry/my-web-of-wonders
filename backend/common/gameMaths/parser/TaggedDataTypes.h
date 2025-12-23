#pragma once
#include <string>
#include <memory>
#include <algorithm>
#include "TaggedRange.h"
#include "../../mathElements/ReelSet.h"
#include "../../mathElements/ReelSetWeightTable.h"
#include "../../mathElements/DoubleWeightTable.h"
#include "../../mathElements/WeightTable.h"
#include "../../mathElements/StringWeightTable.h"
#include "../../mathElements/BoolWeightTable.h"
#include "../../mathElements/PayTable.h"

typedef std::shared_ptr<TaggedRange> TaggedRangePtr;

struct TaggedData
{
    const TaggedRangePtr taggedRangePtr;
    const std::string fullName;

    virtual ~TaggedData() = default;

    TaggedData(const TaggedRangePtr& taggedRangePtr, std::string fullName);
};

template <typename T>
struct TaggedDataIndividual : public TaggedData
{
    const T value;

    explicit TaggedDataIndividual(const TaggedRangePtr& taggedRangePtr, const T value)
        : TaggedData(taggedRangePtr, taggedRangePtr->name),
          value(value)
    {
    }
};

template <typename T>
struct TaggedDataMap : public TaggedData
{
    const std::unordered_map<std::string, T> value;

    explicit TaggedDataMap(const TaggedRangePtr& taggedRangePtr,
                           std::unordered_map<std::string, T> value)
        : TaggedData(taggedRangePtr, taggedRangePtr->name),
          value(value)
    {
    }

protected:
    template <typename Func>
    static std::unordered_map<std::string, T> parseMap(const TaggedRange& range, Func converter)
    {
        std::unordered_map<std::string, T> map;
        for (size_t rowNum = 1; rowNum < range.data.size(); rowNum++)
        {
            const auto& row = range.data[rowNum];
            map[row[0]] = converter(row[1], range.getCellLocationString(rowNum, 1));
        }
        return map;
    }
};

template <typename T>
struct TaggedDataCombined : public TaggedData
{
    const T value;
    const std::string cellLocation;

    TaggedDataCombined(
        const TaggedRangePtr& taggedRangePtr,
        const size_t row,
        const T value)
        : TaggedData(taggedRangePtr, calculateFullName(*taggedRangePtr, row)),
          value(value),
          cellLocation(taggedRangePtr->getCellLocationString(row, 1))
    {
    }

protected:
    static std::string calculateFullName(const TaggedRange& taggedRange, const size_t row)
    {
        if (taggedRange.name.empty()) return taggedRange.data[row][0];
        return taggedRange.name + "." + taggedRange.data[row][0];
    }

    template <typename Func>
    static T calculateValue(const TaggedRange& taggedRange, const size_t row,
                            Func convertValue)
    {
        const std::string valueStr = taggedRange.data[row][1];
        return convertValue(valueStr, taggedRange.getCellLocationString(row, 1));
    };
};

struct TaggedInt : public TaggedDataCombined<int>
{
    TaggedInt(const TaggedRangePtr& taggedRangePtr, const size_t row)
        : TaggedDataCombined(taggedRangePtr,
                             row,
                             calculateValue(*taggedRangePtr, row, convertValue))
    {
    }

private:
    static int convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedDouble : public TaggedDataCombined<double>
{
    TaggedDouble(const TaggedRangePtr& taggedRangePtr, const size_t row)
        : TaggedDataCombined(taggedRangePtr,
                             row,
                             calculateValue(*taggedRangePtr, row, convertValue))
    {
    }

private:
    static double convertValue(const std::string& valueStr,
                               const std::string& cellLocation);
};

struct TaggedString : public TaggedDataCombined<std::string>
{
    TaggedString(const TaggedRangePtr& taggedRangePtr, const size_t row)
        : TaggedDataCombined(taggedRangePtr,
                             row,
                             calculateValue(*taggedRangePtr, row, convertValue))
    {
    }

private:
    static std::string convertValue(const std::string& valueStr,
                                    const std::string& cellLocation);
};

struct TaggedBool : public TaggedDataCombined<bool>
{
    TaggedBool(const TaggedRangePtr& taggedRangePtr, const size_t row)
        : TaggedDataCombined(taggedRangePtr,
                             row,
                             calculateValue(*taggedRangePtr, row, convertValue))
    {
    }

    static bool convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedIntMap : public TaggedDataMap<int>
{
    TaggedIntMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static int convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedDoubleMap : TaggedDataMap<double>
{
    TaggedDoubleMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static double convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedStringMap : TaggedDataMap<std::string>
{
    TaggedStringMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static std::string convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedBoolMap : TaggedDataMap<bool>
{
    TaggedBoolMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static bool convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedRtpMap : TaggedDoubleMap
{
    using TaggedDoubleMap::TaggedDoubleMap;
};

struct TaggedBetTypeMap : TaggedDoubleMap
{
    using TaggedDoubleMap::TaggedDoubleMap;
};

struct TaggedReelSet : TaggedDataIndividual<ReelSet>
{
    TaggedReelSet(const TaggedRangePtr& taggedRangePtr, const size_t numRows)
        : TaggedDataIndividual(taggedRangePtr, parseReelSet(*taggedRangePtr, numRows))
    {
    }

    static ReelSet parseReelSet(const TaggedRange& taggedRange, size_t numRows);
};

struct TaggedReelSetWeights : TaggedDataIndividual<ReelSetWeightTable>
{
    TaggedReelSetWeights(const TaggedRangePtr& taggedRangePtr,
                         const std::unordered_map<std::string, ReelSet>& reelSetMap)
        : TaggedDataIndividual(taggedRangePtr, parseTable(*taggedRangePtr, reelSetMap))
    {
    }

    static ReelSetWeightTable parseTable(const TaggedRange& taggedRange,
                                         const std::unordered_map<std::string, ReelSet>& reelSetMap);
};

struct TaggedIntWeightTable : TaggedDataIndividual<WeightTable>
{
    TaggedIntWeightTable(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataIndividual(taggedRangePtr, parseWeightTable(*taggedRangePtr))
    {
    }

    static WeightTable parseWeightTable(const TaggedRange& taggedRange);
};

struct TaggedDoubleWeightTable : TaggedDataIndividual<DoubleWeightTable>
{
    TaggedDoubleWeightTable(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataIndividual(taggedRangePtr, parseWeightTable(*taggedRangePtr))
    {
    }

    static DoubleWeightTable parseWeightTable(const TaggedRange& taggedRange);
};

struct TaggedStringWeightTable : TaggedDataIndividual<StringWeightTable>
{
    TaggedStringWeightTable(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataIndividual(taggedRangePtr, parseWeightTable(*taggedRangePtr))
    {
    }

    static StringWeightTable parseWeightTable(const TaggedRange& taggedRange);
};

struct TaggedBoolWeightTable : TaggedDataIndividual<BoolWeightTable>
{
    TaggedBoolWeightTable(const TaggedRangePtr& taggedRangePtr) : TaggedDataIndividual(
        taggedRangePtr, parseWeightTable(*taggedRangePtr))
    {
    }

    static BoolWeightTable parseWeightTable(const TaggedRange& taggedRange);
};

struct TaggedPayTable : TaggedDataIndividual<PayTable>
{
    TaggedPayTable(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataIndividual(taggedRangePtr, parsePayTable(*taggedRangePtr))
    {
    }

    static PayTable parsePayTable(const TaggedRange& taggedRange);
};
