#include <absl/container/flat_hash_map.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <json/json.h>
#include "TaggedDataTypes.h"


/******************************************/
/* Helper functions                       */
/******************************************/

namespace TaggedDataHelpers
{
    Json::Value parseJsonArrayOrReturnNull(const std::string& input)
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

        if (!root.isArray())
        {
            return {};
        }

        return root;
    }

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

    std::optional<int> tryStringToInt(const std::string& valueStr)
    {
        const char* begin = valueStr.c_str();
        char* end = nullptr;

        errno = 0;
        const double val = std::strtod(begin, &end);

        // No conversion performed
        if (end == begin)
            return std::nullopt;

        // Overflow/underflow reported by strtod
        if (errno == ERANGE)
            return std::nullopt;

        // Reject NaN/Inf
        if (!std::isfinite(val))
            return std::nullopt;

        // Ensure the rest is only whitespace (same idea as your erase_if + empty check)
        while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)))
            ++end;
        if (*end != '\0')
            return std::nullopt;

        // Must be an integer value (accepts "12" and "12.0", rejects "12.3")
        if (std::trunc(val) != val)
            return std::nullopt;

        // Must fit in int
        if (val < static_cast<double>(std::numeric_limits<int>::min()) ||
            val > static_cast<double>(std::numeric_limits<int>::max()))
            return std::nullopt;

        return static_cast<int>(val);
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

ReelSet TaggedReelSet::parseReelSet(const TaggedRange& taggedRange, const size_t numRows,
                                    const std::unordered_map<std::string, Symbol>& symbolIndexMap)
{
    std::vector<std::vector<Symbol>> reels(taggedRange.width);
    const auto& data = taggedRange.data;
    for (size_t rowNum = 1; rowNum < data.size(); rowNum++)
    {
        for (size_t reelNum = 0; reelNum < taggedRange.width; reelNum++)
        {
            const std::string& valueStr = data[rowNum][reelNum];
            if (valueStr.empty()) continue;
            const auto& symbolIndexOptional = TaggedDataHelpers::tryStringToInt(valueStr);
            if (symbolIndexOptional.has_value())
            {
                reels[reelNum].push_back(static_cast<Symbol>(symbolIndexOptional.value()));
            }
            else
            {
                auto it = symbolIndexMap.find(valueStr);
                if (it != symbolIndexMap.end())
                {
                    reels[reelNum].push_back(static_cast<Symbol>(it->second));
                }
                else
                {
                    throw std::runtime_error(
                        "ReelSet '" + taggedRange.name + "' at " + taggedRange.location +
                        " references unknown symbol '" + valueStr +
                        "'. Ensure a symbol with this name exists in the paytable."
                    );
                }
            }
        }
    }
    return {taggedRange.name, numRows, reels};
}

/******************************************/
/* ReelSetWeights implementation      */
/******************************************/

ReelSetWeights TaggedReelSetWeights::parseTable(const TaggedRange& taggedRange,
                                                const std::unordered_map<std::string, ReelSet>&
                                                reelSetMap)
{
    std::vector<WeightData> weightVector;
    std::vector<ReelSet> reelSetVect;
    const bool hasLimits = taggedRange.width == 3;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        if (!reelSetMap.contains(row[0]))
            throw std::runtime_error(
                "ReelSetWeights '" + taggedRange.name + "' at " + taggedRange.location +
                " references unknown reelSet '" + row[0] + ". Ensure a reelSet with this name exists."
            );

        reelSetVect.push_back(reelSetMap.at(row[0]));

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

        if (hasLimits)
        {
            int limit;
            try
            {
                limit = TaggedDataHelpers::stringToInt(row[2], taggedRange.getCellLocationString(rowNum, 2));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing limit in reelSetWeightTable '" + taggedRange.name +
                    "'. Limit must be an int. Error: " + e.what());
            }
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight, limit);
        }
        else
        {
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight);
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
    const bool hasLimits = taggedRange.width == 3;
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
                "Use stringWeightTable, doubleWeightTable, boolWeightTable, or symbolWeightTable types where necessary. "
                "Error: " + e.what());
        }
        try
        {
            weight = TaggedDataHelpers::stringToInt(row[1], taggedRange.getCellLocationString(rowNum, 1));
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(
                "Error parsing weight in weightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        if (hasLimits)
        {
            int limit;
            try
            {
                limit = TaggedDataHelpers::stringToInt(row[2], taggedRange.getCellLocationString(rowNum, 2));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing limit in weightTable '" + taggedRange.name +
                    ". Limit must be an int. Error: " + e.what()
                );
            }
            weightVector.emplace_back(outcome, weight, limit);
        }
        else weightVector.emplace_back(outcome, weight);
    }
    return WeightTable(weightVector, taggedRange.name);
}

/******************************************/
/* DoubleWeightTable implementation       */
/******************************************/

DoubleWeightTable TaggedDoubleWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<WeightData> weightVector;
    std::vector<double> outcomeVector;
    const bool hasLimits = taggedRange.width == 3;
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
                "Error parsing weight in doubleWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        if (hasLimits)
        {
            int limit;
            try
            {
                limit = TaggedDataHelpers::stringToInt(row[2], taggedRange.getCellLocationString(rowNum, 2));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing limit in weightTable '" + taggedRange.name +
                    ". Limit must be an int. Error: " + e.what()
                );
            }
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight, limit);
        }
        else
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight);
        outcomeVector.push_back(outcome);
    }
    return {weightVector, outcomeVector, taggedRange.name};
}

/******************************************/
/* StringWeightTable implementation       */
/******************************************/

StringWeightTable TaggedStringWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<WeightData> weightVector;
    std::vector<std::string> outcomeVector;
    const bool hasLimits = taggedRange.width == 3;
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
                "Error parsing weight in stringWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        if (hasLimits)
        {
            int limit;
            try
            {
                limit = TaggedDataHelpers::stringToInt(row[2], taggedRange.getCellLocationString(rowNum, 2));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing limit in weightTable '" + taggedRange.name +
                    ". Limit must be an int. Error: " + e.what()
                );
            }
            weightVector.emplace_back(static_cast<int>(rowNum) - 1, weight, limit);
        }
        else
            weightVector.emplace_back(static_cast<int>(rowNum) - 1, weight);
        outcomeVector.push_back(row[0]);
    }
    return {weightVector, outcomeVector, taggedRange.name};
}

/******************************************/
/* StringWeightTable implementation       */
/******************************************/

SymbolWeightTable TaggedSymbolWeightTable::parseWeightTable(const TaggedRange& taggedRange,
                                                            const std::unordered_map<std::string, Symbol>&
                                                            symbolIndexMap)
{
    std::vector<WeightData> weightVector;
    std::vector<Symbol> outcomeVector;
    const bool hasLimits = taggedRange.width == 3;
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
                "Error parsing weight in symbolWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }

        if (hasLimits)
        {
            int limit;
            try
            {
                limit = TaggedDataHelpers::stringToInt(row[2], taggedRange.getCellLocationString(rowNum, 2));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing limit in symbolWeightTable '" + taggedRange.name +
                    "'. Limit must be an int. Error: " + e.what());
            }
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight, limit);
        }
        else
        {
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight);
        }

        const std::string& valueStr = row[0];
        const auto intOptional = TaggedDataHelpers::tryStringToInt(valueStr);
        if (intOptional.has_value()) outcomeVector.push_back(static_cast<Symbol>(intOptional.value()));
        else
        {
            auto it = symbolIndexMap.find(valueStr);
            if (it != symbolIndexMap.end())
                outcomeVector.push_back(it->second);
            else
                throw std::runtime_error(
                    "Unknown symbol '" + valueStr + "' at " + taggedRange.getCellLocationString(rowNum, 0) +
                    ". Unsure this symbol is in the pay table, even if it doesn't pay.");
        }
    }
    return {weightVector, outcomeVector, taggedRange.name};
}

/******************************************/
/* DoubleWeightTable implementation       */
/******************************************/

BoolWeightTable TaggedBoolWeightTable::parseWeightTable(const TaggedRange& taggedRange)
{
    std::vector<WeightData> weightVector;
    std::vector<bool> outcomeVector;
    const bool hasLimits = taggedRange.width == 3;
    for (size_t rowNum = 1; rowNum < taggedRange.data.size(); rowNum++)
    {
        const auto& row = taggedRange.data[rowNum];
        bool outcome;
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
                "Error parsing weight in boolWeightTable '" + taggedRange.name + ". Error: " + e.what());
        }
        if (hasLimits)
        {
            int limit;
            try
            {
                limit = TaggedDataHelpers::stringToInt(row[2], taggedRange.getCellLocationString(rowNum, 2));
            }
            catch (std::exception& e)
            {
                throw std::runtime_error(
                    "Error parsing limit in weightTable '" + taggedRange.name +
                    ". Limit must be an int. Error: " + e.what()
                );
            }
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight, limit);
        }
        else
            weightVector.emplace_back(static_cast<int>(rowNum - 1), weight);
        outcomeVector.push_back(outcome);
    }
    return {weightVector, outcomeVector, taggedRange.name};
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

    std::vector<std::vector<unsigned int>> payTableVect;
    std::vector<Symbol> paySymVect;
    std::vector<Symbol> scatterSymVect;
    std::vector<Symbol> wildSymVect;
    std::vector<Symbol> wildNoSubstVect;
    std::vector<Symbol> scatterWildSymVect;
    std::vector<std::string> symbolNames;

    for (size_t rowNum = 1; rowNum < data.size(); rowNum++)
    {
        const auto& row = data[rowNum];
        symbolNames.push_back(row[0]);
        std::vector<unsigned int> symbolVect(maxOAK + 1, 0);
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
        const Symbol symbolId = static_cast<Symbol>(rowNum) - 1;
        const std::string& payType = row[1];
        if (payType == "line" || payType == "ways" || payType == "way")
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
        else if (payType == "scatterWild")
        {
            wildSymVect.push_back(symbolId);
            scatterWildSymVect.push_back(symbolId);
        }
        else if (payType == "scatterWildOnly")
            scatterWildSymVect.push_back(symbolId);
    }
    return {
        payTableVect,
        paySymVect,
        scatterSymVect,
        wildSymVect,
        wildNoSubstVect,
        scatterWildSymVect,
        symbolNames
    };
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

/******************************************/
/* Symbol implementation                  */
/******************************************/

Symbol TaggedSymbol::convertValue(const std::string& valueStr,
                                  const std::string& cellLocation,
                                  const std::unordered_map<std::string, Symbol>& symbolIndexMap)
{
    const auto intOptional = TaggedDataHelpers::tryStringToInt(valueStr);
    if (intOptional.has_value()) return static_cast<Symbol>(intOptional.value());

    auto it = symbolIndexMap.find(valueStr);
    if (it != symbolIndexMap.end()) return it->second;
    throw std::runtime_error(
        "Unknown symbol '" + valueStr + "' at " + cellLocation +
        ". Ensure this symbol is present in pay table, even if it does not pay.");
}

std::vector<Symbol> TaggedSymbolVector::convertValue(const std::string& valueStr,
                                                     const std::string& cellLocation,
                                                     const std::unordered_map<std::string, Symbol>&
                                                     symbolIndexMap)
{
    if (!valueStr.starts_with("[") || !valueStr.ends_with("]"))
        throw std::runtime_error(
            "Invalid symbol list '" + valueStr + "' at " + cellLocation +
            ". Must be in the form of a json array beginning with '[' and ending with ']'.");

    Json::Value jsonValue = TaggedDataHelpers::parseJsonArrayOrReturnNull(valueStr);
    if (jsonValue.isNull())
        throw std::runtime_error(
            "Invalid symbol list '" + valueStr + "' at " + cellLocation +
            ". Does not parse to valid Json. Ensure symbols, if named, are in double quotes.");
    if (!jsonValue.isArray())
        throw std::runtime_error("Invalid symbol list '" + valueStr + "' at " + cellLocation +
            ". Symbols must be specified as a json array.");
    std::vector<Symbol> symbolVector;
    for (const auto& symbolStr : jsonValue)
    {
        symbolVector.
            push_back(TaggedSymbol::convertValue(symbolStr.asString(), cellLocation, symbolIndexMap));
    }
    return symbolVector;
}
