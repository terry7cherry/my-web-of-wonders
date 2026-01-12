#pragma once

#include <xlnt/xlnt.hpp>
#include <json/json.h>
#include <stdexcept>
#include "../../utils/utils.h"

struct TaggedRange
{
public:
    const xlnt::worksheet sheet;
    const xlnt::range_reference range;
    const size_t width;
    const std::string location;
    const std::string type;
    const std::string name;
    const std::vector<std::string> betTypes;
    const std::vector<int> rtps;
    const std::vector<std::vector<std::string>> data;
    const int specificity;

    TaggedRange(const xlnt::worksheet& sheet, const xlnt::range_reference& range,
                const Json::Value& tag)
        : sheet(sheet),
          range(range),
          width(range.width()),
          location(calculateLocationString()),
          type(tag["type"].asString()),
          name(tag["name"].asString()),
          betTypes(calculateBetTypes(tag["betTypes"])),
          rtps(calculateRTPs(tag["rtps"])),
          data(extractData()),
          specificity(getSpecificity())
    {
    }

    std::string getCellLocationString(const xlnt::row_t row,
                                      const xlnt::column_t::index_t column) const
    {
        const xlnt::cell_reference cellRef(
            range.top_left().column() + column,
            range.top_left().row() + 1 + row);
        return "'" + sheet.title() + "'!" + cellRef.to_string();
    }

    std::string getCellLocationString(const size_t row,
                                      const size_t column) const
    {
        return getCellLocationString(static_cast<xlnt::row_t>(row),
                                     static_cast<xlnt::column_t::index_t>(column));
    }

private:
    std::string calculateLocationString() const
    {
        return "'" + sheet.title() + "'!" + range.to_string();
    }

    std::vector<std::string> calculateBetTypes(const Json::Value& betTypesJson) const
    {
        std::vector<std::string> betTypesVec;
        if (betTypesJson.isNull()) return betTypesVec;
        if (!betTypesJson.isArray())
        {
            throw std::runtime_error(
                "betTypes is incorrectly formatted at " + location +
                " (should be an array of strings).");
        }
        for (const auto& betType : betTypesJson)
        {
            betTypesVec.push_back(betType.asString());
        }
        return betTypesVec;
    }

    std::vector<int> calculateRTPs(const Json::Value& rtpsJson) const
    {
        std::vector<int> rtpsVec;
        if (rtpsJson.isNull()) return rtpsVec;
        if (!rtpsJson.isArray())
        {
            throw std::runtime_error(
                "rtps is incorrectly formatted at " + location + " (should be an array of ints)."
            );
        }
        for (const auto& rtp : rtpsJson)
        {
            rtpsVec.push_back(rtp.asInt());
        }
        return rtpsVec;
    }

    std::vector<std::vector<std::string>> extractData() const
    {
        const xlnt::column_t firstColumn = range.top_left().column();
        std::vector<std::vector<std::string>> dataVect;
        for (
            xlnt::row_t row = range.top_left().row() + 1; ; ++row
        )
        {
            std::vector<std::string> rowData;
            bool isLastRow = true;
            for (xlnt::column_t::index_t colOffset = 0;
                 colOffset < static_cast<xlnt::column_t::index_t>(width);
                 ++colOffset)
            {
                try
                {
                    xlnt::cell_reference cellRef(firstColumn.index + colOffset, row);
                    if (!sheet.has_cell(cellRef))
                    {
                        rowData.emplace_back("");
                        continue;
                    }
                    xlnt::cell cell = sheet.cell(cellRef);
                    const std::string cellValue = cell.to_string();


                    if ((cellValue.empty() || isVisuallyBlank(cell)))
                    {
                        rowData.emplace_back("");
                        continue;
                    }
                    rowData.push_back(cellValue);
                    isLastRow = false;
                }
                catch (const std::exception& e)
                {
                    reportError("TaggedRange.h::getData",
                                "xlnt/std exception at " + location + " : " + e.what());
                    throw; // rethrow so you still break where it matters
                }
            }

            if (isLastRow)
                break;
            dataVect.push_back(rowData);
        }
        return dataVect;
    }

    int getSpecificity() const
    {
        return (betTypes.empty() ? 0 : 1) + (rtps.empty() ? 0 : 1);
    }

    static bool isVisuallyBlank(const xlnt::cell& c)
    {
        // Truly empty
        if (!c.has_value() || c.data_type() == xlnt::cell_type::empty) return true;

        // Whitespace-only string treated as blank
        std::string s = c.to_string();
        std::erase_if(s, [](unsigned char ch) { return std::isspace(ch); });
        return s.empty();
    }
};
