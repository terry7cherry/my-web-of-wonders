#include "ReportBuffer.h"
#include <fstream>
#include "../utils/utils.h"

ReportBuffer::ReportBuffer(const uint64_t simGames, const std::string& betType, const int rtp, const int bet,
                           const std::string& simForce, const int seed
) :
    outputFolder(
        createOutputFolder(betType, rtp, bet, simGames, simForce))
{
    addStat("RUN SETTINGS", "Games", addThousandsSeparator(simGames));
    addStat("RUN SETTINGS", "Bet", std::to_string(bet));
    addStat("RUN SETTINGS", "RTP", std::to_string(rtp));
    addStat("RUN SETTINGS", "Seed", std::to_string(seed));
    if (!betType.empty() && betType != "default") addStat("RUN SETTINGS", "BetType", betType);
}

void ReportBuffer::ensureTable(const std::string& title, const std::vector<std::string>& headers)
{
    auto it = tables.find(title);
    if (it == tables.end())
    {
        tables.emplace(title, Table{title, headers, {}});
        order.push_back(title);
    }
    else
    {
        if (it->second.headers != headers)
        {
            // If headers differ, prefer the first encountered and allow adding with resized cells.
            // For safety, we align by column count of existing table.
        }
    }
}

void ReportBuffer::addRow(const std::string& title,
                          const std::vector<std::string>& headers,
                          const std::vector<std::string>& cells)
{
    ensureTable(title, headers);
    auto& tbl = tables[title];
    tbl.rows.push_back(cells);
}

void ReportBuffer::addStat(const std::string& section, const std::string& key, const std::string& value)
{
    addRow(section, {"Key", "Value"}, {key, value});
}

void ReportBuffer::printConsole() const
{
    for (const auto& name : order)
    {
        const auto& tbl = tables.at(name);
        std::printf("\n\n%s\n", tbl.title.c_str());
        // compute widths
        std::vector<size_t> widths(tbl.headers.size(), 0);
        for (size_t i = 0; i < tbl.headers.size(); ++i)
            widths[i] = std::max<size_t>(widths[i], tbl.headers[i].size());
        for (const auto& row : tbl.rows)
        {
            for (size_t i = 0; i < row.size() && i < widths.size(); ++i)
                widths[i] = std::max(widths[i], row[i].size());
        }
        // header
        for (size_t i = 0; i < tbl.headers.size(); ++i)
        {
            std::printf("%-*s%s", static_cast<int>(widths[i]), tbl.headers[i].c_str(),
                        (i + 1 < tbl.headers.size()) ? "  " : "\n");
        }
        // rows
        for (const auto& row : tbl.rows)
        {
            for (size_t i = 0; i < tbl.headers.size(); ++i)
            {
                const char* cell = (i < row.size() ? row[i].c_str() : "");
                std::printf("%-*s%s", static_cast<int>(widths[i]), cell,
                            (i + 1 < tbl.headers.size()) ? "  " : "\n");
            }
        }
    }
}

void ReportBuffer::writeCsv() const
{
    const std::filesystem::path csvFile = outputFolder / "Report.csv";
    std::ofstream out(csvFile);
    if (!out.is_open())
    {
        reportError("Stats.ReportCSV", std::string("Can't open file: ") + csvFile.string(),
                    "Warning");
        return;
    }
    auto esc = [](const std::string& s) -> std::string
    {
        const bool needQuotes = s.find_first_of(",\n\r\"") != std::string::npos;
        if (!needQuotes) return s;
        std::string r = "\"";
        for (char c : s)
        {
            if (c == '"') r += '"';
            r += c;
        }
        r += '"';
        return r;
    };

    bool firstSection = true;
    for (const auto& name : order)
    {
        const auto& tbl = tables.at(name);
        if (!firstSection) out << "\n"; // blank line between sections
        firstSection = false;
        out << tbl.title << "\n";
        for (size_t i = 0; i < tbl.headers.size(); ++i)
        {
            out << esc(tbl.headers[i]);
            if (i + 1 < tbl.headers.size()) out << ",";
        }
        out << "\n";
        for (const auto& row : tbl.rows)
        {
            for (size_t i = 0; i < tbl.headers.size(); ++i)
            {
                const std::string cell = (i < row.size() ? row[i] : std::string());
                out << esc(cell);
                if (i + 1 < tbl.headers.size()) out << ",";
            }
            out << "\n";
        }
    }
    out.close();
}

std::filesystem::path ReportBuffer::createOutputFolder(const std::string& betType, const int rtp,
                                                       const int bet,
                                                       const uint64_t simGames,
                                                       const std::string& simForce)
{
    namespace fs = std::filesystem;

    const fs::path distributionsRoot = fs::path(GAME_SOURCE_DIR) / "Simulations";
    fs::create_directories(distributionsRoot);

    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H.%M");

    if (simGames > BILLION)
        oss << "_" << simGames / BILLION << "b";
    else
        oss << "_" << simGames / MILLION << "m";

    oss << "_RTP" << rtp << "_bet" << bet;

    if (betType != "default") oss << "_" << betType;
    if (simForce != "None") oss << "_" << simForce;

    const fs::path outputFolder = distributionsRoot / oss.str();
    fs::create_directories(outputFolder);
    return outputFolder;
}

std::filesystem::path ReportBuffer::createSubFolder(const std::string& folderName) const
{
    std::filesystem::path path = outputFolder / folderName;
    std::filesystem::create_directories(path);
    return path;
}
