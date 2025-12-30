#pragma once
#include <string>
#include <memory>
#include <algorithm>
#include "TaggedRange.h"
#include "../../mathElements/ReelSet.h"
#include "../../mathElements/WeightTables.h"
#include "../../mathElements/PayTable.h"
#include "../../mathElements/PayLines.h"

typedef std::shared_ptr<TaggedRange> TaggedRangePtr;

template <typename T>
struct TaggedData
{
    const TaggedRangePtr taggedRangePtr;
    const std::string fullName;
    const T value;

    virtual ~TaggedData() = default;

protected:
    TaggedData(const TaggedRangePtr& taggedRangePtr, std::string fullName, T value)
        : taggedRangePtr(taggedRangePtr), fullName(std::move(fullName)), value(value)
    {
    }
};

template <typename T>
struct TaggedDataIndividual : TaggedData<T>
{
    explicit TaggedDataIndividual(const TaggedRangePtr& taggedRangePtr, const T value)
        : TaggedData<T>(taggedRangePtr, taggedRangePtr->name, value)
    {
    }
};

template <typename TKey, typename TValue>
struct TaggedDataMap : TaggedData<std::unordered_map<TKey, TValue>>
{
    explicit TaggedDataMap(const TaggedRangePtr& taggedRangePtr,
                           std::unordered_map<TKey, TValue> value)
        : TaggedData<std::unordered_map<TKey, TValue>>(taggedRangePtr, taggedRangePtr->name, value)
    {
    }

protected:
    template <typename KeyFunc, typename ValueFunc>
    static std::unordered_map<TKey, TValue> parseMap(const TaggedRange& range, KeyFunc keyConverter,
                                                     ValueFunc valueConverter)
    {
        std::unordered_map<TKey, TValue> map;
        for (size_t rowNum = 1; rowNum < range.data.size(); rowNum++)
        {
            const auto& row = range.data[rowNum];
            map[
                keyConverter(row[0], range.getCellLocationString(rowNum, 0))
            ] = valueConverter(row[1], range.getCellLocationString(rowNum, 1));
        }
        return map;
    }
};

template <typename T>
struct TaggedStringKeyedMap : TaggedDataMap<std::string, T>
{
    using TaggedDataMap<std::string, T>::TaggedDataMap;

protected:
    template <typename ValueFunc>
    static std::unordered_map<std::string, T> parseMap(const TaggedRange& range, ValueFunc valueConverter)
    {
        return TaggedDataMap<std::string, T>::parseMap(range, [](const std::string& key,
                                                                 const std::string& cellLocation)
                                                       {
                                                           return key;
                                                       },
                                                       valueConverter);
    }
};

template <typename T>
struct TaggedDataCombined : TaggedData<T>
{
    const std::string cellLocation;

    TaggedDataCombined(
        const TaggedRangePtr& taggedRangePtr,
        const size_t row,
        const T value)
        : TaggedData<T>(taggedRangePtr, calculateFullName(*taggedRangePtr, row), value),
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

struct TaggedIntMap : public TaggedStringKeyedMap<int>
{
    TaggedIntMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedStringKeyedMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static int convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedDoubleMap : TaggedStringKeyedMap<double>
{
    TaggedDoubleMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedStringKeyedMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static double convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedStringMap : TaggedStringKeyedMap<std::string>
{
    TaggedStringMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedStringKeyedMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static std::string convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedBoolMap : TaggedStringKeyedMap<bool>
{
    TaggedBoolMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedStringKeyedMap(taggedRangePtr, parseMap(*taggedRangePtr, convertValue))
    {
    }

private:
    static bool convertValue(const std::string& valueStr, const std::string& cellLocation);
};

struct TaggedRtpMap : TaggedDataMap<int, double>
{
    TaggedRtpMap(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataMap(taggedRangePtr, parseMap(*taggedRangePtr, convertKey, convertValue))
    {
    }

private:
    static int convertKey(const std::string& keyStr, const std::string& cellLocation);
    static double convertValue(const std::string& valueStr, const std::string& cellLocation);
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

struct TaggedPayLines : TaggedDataIndividual<PayLines>
{
    TaggedPayLines(const TaggedRangePtr& taggedRangePtr)
        : TaggedDataIndividual(taggedRangePtr, parsePayLines(*taggedRangePtr))
    {
    }

    static PayLines parsePayLines(const TaggedRange& taggedRange);
};
