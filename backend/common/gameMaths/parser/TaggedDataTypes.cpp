#include "TaggedDataTypes.h"
#include <algorithm>
#include <stdexcept>
#include <string>

/******************************************/
/* Helper functions                       */
/******************************************/

namespace TaggedDataHelpers
{
    int stringToInt(std::string valueStr, const std::string& cellLocation)
    {
        valueStr.erase(std::ranges::remove(valueStr, ',').begin(), valueStr.end());

        try
        {
            size_t pos = 0;
            const double val = std::stod(valueStr, &pos);

            std::string remaining = valueStr.substr(pos);
            std::erase_if(remaining, ::isspace);

            if (!remaining.empty())
            {
                throw std::runtime_error(
                    "Trailing characters in integer: " + valueStr + " at " + cellLocation);
            }

            if (val != static_cast<double>(static_cast<int>(val)))
            {
                throw std::runtime_error(
                    "Value is a decimal, expected integer: " + valueStr + " at " + cellLocation);
            }

            return static_cast<int>(val);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to parse '" + valueStr + "' as integer at " +
                cellLocation + ": " + e.what());
        }
    }

    std::optional<int> tryStringToInt(std::string valueStr)
    {
        valueStr.erase(std::ranges::remove(valueStr, ',').begin(), valueStr.end());

        try
        {
            size_t pos = 0;
            const double val = std::stod(valueStr, &pos);

            std::string remaining = valueStr.substr(pos);
            std::erase_if(remaining, ::isspace);

            if (!remaining.empty())
                return {};

            if (val != static_cast<double>(static_cast<int>(val)))
                return {};

            return static_cast<int>(val);
        }
        catch (std::exception)
        {
            return {};
        }
    }

    double stringToDouble(std::string valueStr, const std::string& cellLocation)
    {
        valueStr.erase(std::ranges::remove(valueStr, ',').begin(), valueStr.end());

        try
        {
            size_t pos = 0;
            const double val = std::stod(valueStr, &pos);

            std::string remaining = valueStr.substr(pos);
            std::erase_if(remaining, ::isspace);

            if (!remaining.empty())
            {
                throw std::runtime_error(
                    "Trailing characters in float: " + valueStr + " at " + cellLocation);
            }

            return val;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to parse '" + valueStr + "' as float at " +
                cellLocation + ": " + e.what());
        }
    }

    bool stringToBool(const std::string& valueStr, const std::string& cellLocation)
    {
        // 1. Fast path for standard Excel booleans.
        if (valueStr == "TRUE") return true;
        if (valueStr == "FALSE") return false;

        // 2. Trim whitespace
        const auto begin = valueStr.find_first_not_of(" \t\n\r\f\v");
        if (begin == std::string::npos)
            throw std::invalid_argument(
                "Empty or whitespace-only input for bool at " + cellLocation);

        const auto end = valueStr.find_last_not_of(" \t\n\r\f\v");
        std::string value = valueStr.substr(begin, end - begin + 1);

        // 3. Fast path for numeric values
        if (value == "1") return true;
        if (value == "0") return false;

        // 4. Normalize case
        std::ranges::transform(value, value.begin(),
                               [](const unsigned char c)
                               {
                                   return static_cast<char>(std::tolower(c));
                               });

        // 4. Match textual forms
        if (value == "true")
            return true;
        if (value == "false")
            return false;

        throw std::invalid_argument("Invalid boolean string: '" + valueStr + "' at " + cellLocation);
    }
}


/******************************************/
/* Int implementation                     */
/******************************************/

int TaggedInt::convertValue(const std::string& valueStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToInt(valueStr, cellLocation);
}


/******************************************/
/* Double implementation                  */
/******************************************/

double TaggedDouble::convertValue(const std::string& valueStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToDouble(valueStr, cellLocation);
}

/******************************************/
/* String implementation                  */
/******************************************/

std::string TaggedString::convertValue(const std::string& valueStr,
                                       [[maybe_unused]] const std::string& cellLocation)
{
    return valueStr;
}

/******************************************/
/* Bool implementation                    */
/******************************************/

bool TaggedBool::convertValue(const std::string& valueStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToBool(valueStr, cellLocation);
}

/******************************************/
/* IntMap implementation                  */
/******************************************/

int TaggedIntMap::convertValue(const std::string& valueStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToInt(valueStr, cellLocation);
}

/******************************************/
/* DoubleMap implementation               */
/******************************************/

double TaggedDoubleMap::convertValue(const std::string& valueStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToDouble(valueStr, cellLocation);
}

/******************************************/
/* StringMap implementation               */
/******************************************/

std::string TaggedStringMap::convertValue(const std::string& valueStr,
                                          [[maybe_unused]] const std::string& cellLocation)
{
    return valueStr;
}

/******************************************/
/* BoolMap implementation                 */
/******************************************/

bool TaggedBoolMap::convertValue(const std::string& valueStr,
                                 const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToBool(valueStr, cellLocation);
}

/******************************************/
/* RtpMap implementation                  */
/******************************************/

int TaggedRtpMap::convertKey(const std::string& keyStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToInt(keyStr, cellLocation);
}

double TaggedRtpMap::convertValue(const std::string& valueStr, const std::string& cellLocation)
{
    return TaggedDataHelpers::stringToDouble(valueStr, cellLocation);
}

/******************************************/
/* ReelSet implementation                 */
/******************************************/

ReelSet TaggedReelSet::parseReelSet(const TaggedRange& taggedRange, const size_t numRows)
{
    std::vector<std::vector<int>> reels(taggedRange.width);
    const auto& data = taggedRange.data;
    for (size_t rowNum = 1; rowNum < data.size(); rowNum++)
    {
        for (size_t reelNum = 0; reelNum < taggedRange.width; reelNum++)
        {
            const std::string& valueStr = data[rowNum][reelNum];
            if (valueStr.empty()) continue;
            try
            {
                reels[reelNum].push_back(
                    TaggedDataHelpers::stringToInt(
                        valueStr, taggedRange.getCellLocationString(rowNum, reelNum)
                    )
                );
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing reelSet " + taggedRange.name +
                    ". A symbol could not be converted to an int. Error: " + e.what());
            }
        }
    }
    return {taggedRange.name, numRows, reels};
}

/******************************************/
/* ReelSetWeightTable implementation      */
/******************************************/

ReelSetWeightTable TaggedReelSetWeights::parseTable(const TaggedRange& taggedRange,
                                                    const std::unordered_map<std::string, ReelSet>&
                                                    reelSetMap)
{
    std::vector<int> weightVector;
    std::vector<ReelSet> reelSetVect;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        if (!reelSetMap.contains(row[0]))
            throw std::runtime_error(
                "ReelSetWeightTable '" + taggedRange.name + "' at " + taggedRange.location +
                " references unknown reelSet '" + row[0] + ". Ensure a reelSet with this name exists."
            );

        reelSetVect.push_back(reelSetMap.at(row[0]));
        try
        {
            weightVector.push_back(
                TaggedDataHelpers::stringToInt(row[1], taggedRange.getCellLocationString(rowNum, 1)));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing weight in reelSetWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
    }
    return {weightVector, reelSetVect, taggedRange.name};
}

/******************************************/
/* IntWeightTable implementation          */
/******************************************/

WeightTable TaggedIntWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<WeightData> weightVector;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        int outcome;
        int weight;
        try
        {
            outcome = TaggedDataHelpers::stringToInt(row[0], taggedRange.getCellLocationString(rowNum, 0));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing outcome in weightTable '" + taggedRange.name +
                "'. Note that weightTables have int outcomes by default. "
                "Use stringWeightTable, doubleWeightTable, or boolWeightTable types where necessary. "
                "Error: " + e.what());
        }
        try
        {
            weight = TaggedDataHelpers::stringToInt(row[1], taggedRange.getCellLocationString(rowNum, 1));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing weight in reelSetWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        weightVector.emplace_back(outcome, weight);
    }
    return WeightTable(weightVector, taggedRange.name);
}

/******************************************/
/* DoubleWeightTable implementation       */
/******************************************/

DoubleWeightTable TaggedDoubleWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<int> weightVector;
    std::vector<double> outcomeVector;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        double outcome;
        int weight;
        try
        {
            outcome = TaggedDataHelpers::stringToDouble(row[0], taggedRange.getCellLocationString(rowNum, 0));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing outcome in doubleWeightTable '" + taggedRange.name +
                "Error: " + e.what());
        }
        try
        {
            weight = TaggedDataHelpers::stringToInt(row[1], taggedRange.getCellLocationString(rowNum, 1));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing weight in reelSetWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        weightVector.push_back(weight);
        outcomeVector.push_back(outcome);
    }
    return DoubleWeightTable(weightVector, outcomeVector, taggedRange.name);
}

/******************************************/
/* StringWeightTable implementation       */
/******************************************/

StringWeightTable TaggedStringWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<int> weightVector;
    std::vector<std::string> outcomeVector;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        int weight;
        try
        {
            weight = TaggedDataHelpers::stringToInt(row[1], taggedRange.getCellLocationString(rowNum, 1));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing weight in reelSetWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        weightVector.push_back(weight);
        outcomeVector.push_back(row[0]);
    }
    return StringWeightTable(weightVector, outcomeVector, taggedRange.name);
}

/******************************************/
/* DoubleWeightTable implementation       */
/******************************************/

BoolWeightTable TaggedBoolWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<int> weightVector;
    std::vector<bool> outcomeVector;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        double outcome;
        int weight;
        try
        {
            outcome = TaggedDataHelpers::stringToBool(row[0], taggedRange.getCellLocationString(rowNum, 0));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing outcome in boolWeightTable '" + taggedRange.name +
                "Error: " + e.what());
        }
        try
        {
            weight = TaggedDataHelpers::stringToInt(row[1], taggedRange.getCellLocationString(rowNum, 1));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing weight in reelSetWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        weightVector.push_back(weight);
        outcomeVector.push_back(outcome);
    }
    return BoolWeightTable(weightVector, outcomeVector, taggedRange.name);
}

/******************************************/
/* PayTable implementation                */
/******************************************/

PayTable TaggedPayTable::parsePayTable(const TaggedRange& taggedRange)
{
    const auto& data = taggedRange.data;
    const auto& headers = data[0];
    size_t maxOAK = 0;
    for (const std::string& header : headers)
    {
        if (auto oak = TaggedDataHelpers::tryStringToInt(header))
        {
            maxOAK = std::max(maxOAK, static_cast<size_t>(*oak));
        }
    }

    if (maxOAK == 0)
        throw std::runtime_error(
            "No OAK headers found in PayTable at " + taggedRange.location +
            ". PayTable must include headers 'symbols' and 'payType' followed "
            "by an int header for each paying OAK.");

    std::vector<std::vector<size_t>> payTableVect;
    std::vector<size_t> paySymVect;
    std::vector<size_t> scatterSymVect;
    std::vector<size_t> wildSymVect;
    std::vector<size_t> wildNoSubstVect;

    for (size_t rowNum = 1; rowNum < data.size(); rowNum++)
    {
        const auto& row = data[rowNum];
        std::vector<size_t> symbolVect(maxOAK + 1, 0);
        for (size_t colNum = 0; colNum < headers.size(); colNum++)
            if (auto oak = TaggedDataHelpers::tryStringToInt(headers[colNum]))
            {
                try
                {
                    const int prize = TaggedDataHelpers::stringToInt(row[colNum],
                                                                     taggedRange.getCellLocationString(
                                                                         rowNum, colNum));
                    symbolVect[*oak] = prize;
                }
                catch (std::exception& e)
                {
                    throw std::runtime_error(
                        std::string("Error parsing award in payTable. Could not convert award to int. ") +
                        "Awards must be ints expressed in terms of coins. Error: " + e.what()
                    );
                }
            }
        payTableVect.push_back(symbolVect);
        const size_t symbolId = rowNum - 1;
        const std::string& payType = row[1];
        if (payType == "line")
            paySymVect.push_back(symbolId);
        else if (payType == "lineNoWild")
        {
            paySymVect.push_back(symbolId);
            wildNoSubstVect.push_back(symbolId);
        }
        else if (payType == "wild")
            wildSymVect.push_back(symbolId);
        else if (payType == "scatter")
            scatterSymVect.push_back(symbolId);
    }
    return PayTable(payTableVect, paySymVect, scatterSymVect, wildSymVect, wildNoSubstVect);
}

/******************************************/
/* PayLines implementation                */
/******************************************/

PayLines TaggedPayLines::parsePayLines(const TaggedRange& taggedRange)
{
    std::vector<std::vector<size_t>> payLinesVect;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        std::vector<size_t> payLine;
        for (size_t colNum = 0; colNum < row.size(); colNum++)
        {
            const std::string& apertureStr = row[colNum];
            try
            {
                const size_t aperture = TaggedDataHelpers::stringToInt(apertureStr,
                                                                       taggedRange.getCellLocationString(
                                                                           rowNum, colNum));
                payLine.push_back(aperture);
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    std::string("Error parsing aperture in payLine. Could not convert aperture to int. ") +
                    "Error: " + e.what()
                );
            }
        }
        payLinesVect.push_back(payLine);
    }

    return {payLinesVect};
}
