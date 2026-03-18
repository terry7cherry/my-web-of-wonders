#pragma once
#include <cstdio>
#include <filesystem>
#include <cstdint>
#include <string>
#include <map>
#include <vector>

class ReportBuffer
{
public:
    std::filesystem::path outputFolder;

    struct Table
    {
        std::string title;
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> rows;
    };

    ReportBuffer(uint64_t simGames, const std::string& betType, int rtp, int bet,
                 const std::string& simForce, int seed);

    // Ensure a table exists with given title and headers. If it exists, headers must match.
    void ensureTable(const std::string& title, const std::vector<std::string>& headers);

    // Add a row to a section table; creates the table if needed.
    void addRow(const std::string& title,
                const std::vector<std::string>& headers,
                const std::vector<std::string>& cells);

    // Store a simple key/value stat under a section; implemented as a 2-col table
    void addStat(const std::string& section, const std::string& key, const std::string& value);

    // Console printing with auto column widths
    void printConsole() const;

    // CSV writing
    void writeCsv() const;

    static std::filesystem::path createOutputFolder(const std::string& betType, int rtp,
                                                    int bet,
                                                    uint64_t simGames,
                                                    const std::string& simForce);

    std::filesystem::path createSubFolder(const std::string& folderName) const;

    static std::string toPercent(long double v, int precision = 3);


    static std::string toFloat(long double v, int precision = 2);

private:
    std::map<std::string, Table> tables;
    std::vector<std::string> order;
};
