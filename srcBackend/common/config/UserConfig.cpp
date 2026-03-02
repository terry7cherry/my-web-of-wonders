#include <filesystem>
#include <fstream>
#include <sstream>

#include "UserConfig.h"

#include <random>

#include "../gameMaths/parser/ParSheet.h"
#include "../console/Console.h"
#include "../../thisTitle/forces/simForces.h"

UserConfig::UserConfig(const bool rnStream)
    : simSize(rnStream ? 0 : getUserSimSizeChoice()),
      parSheet(getUserParSheetChoice()),
      rnStreamFileName(rnStream ? getUserRnStreamFileChoice() : ""),
      rtp(getUserRtpChoice(parSheet, rnStream)),
      betType(getUserBetTypeChoice(parSheet)), force(getUserForceChoice()),
      seedChoice(rnStream ? 0 : getUserSeedChoice()), seed(seedChoice.value_or(std::random_device{}()))
{
    saveOrUpdateCache(rnStream);
}

UserConfig::UserConfig(const std::string& cachedConfig, const bool rnStream)
    : simSize(rnStream ? 0 : std::stoull(split(cachedConfig).at(1))),
      parSheet(split(cachedConfig).at(0)),
      rnStreamFileName(rnStream ? split(cachedConfig).at(1) : ""),
      rtp(std::stoi(split(cachedConfig).at(2))),
      betType(split(cachedConfig).at(3)),
      force(stringToForce(split(cachedConfig).at(4))),
      seedChoice(rnStream || split(cachedConfig).at(5) == "RAND"
                     ? std::nullopt
                     : std::optional(std::stoi(split(cachedConfig).at(6)))
      ),
      seed(rnStream ? 0 : seedChoice.value_or(std::random_device{}()))
{
    saveOrUpdateCache(rnStream);
    headerText += " - " + parSheet.gameTitle;
    headerText += " - " + (rtp > 0 ? " RTP" + std::to_string(rtp) : " All RTPs");
    headerText += " - " + betType;
    if (force) headerText += " - " + Forces::simForceNames.at(force);
}

UserConfig UserConfig::getNewOrCachedConfig(const bool rnStream)
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = fs::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" / (rnStream ? "rnCache.txt" : "cache.txt");
    if (!fs::exists(cacheFilePath)) return UserConfig(rnStream);

    const std::vector<std::string> lines = getCachedConfigLines(rnStream);

    std::vector<std::vector<std::string>> configChoices;
    for (const auto& line : lines)
    {
        std::vector<std::string> choice;
        const std::vector<std::string> elements = split(line);
        if (rnStream)
        {
            choice.emplace_back(elements.at(0));
            choice.emplace_back(elements.at(1));
            choice.emplace_back(elements.at(2));
            choice.emplace_back(elements.at(3));
            choice.emplace_back(Forces::simForceNames.at(std::stoi(elements.at(4))));
        }
        else
        {
            choice.emplace_back(elements.at(0));
            choice.emplace_back(std::to_string(std::stoull(elements.at(1)) / MILLION));
            choice.emplace_back(elements.at(2));
            choice.emplace_back(elements.at(3));
            choice.emplace_back(Forces::simForceNames.at(std::stoi(elements.at(4))));
            choice.emplace_back(elements.at(5));
            choice.emplace_back(elements.at(6));
        }
        configChoices.emplace_back(choice);
    }

    const std::optional<size_t> userChoice = getUserCachedConfigChoice(
        configChoices, "Select Cached Config", rnStream);
    if (userChoice.value_or(0) == 0) return UserConfig(rnStream);
    return UserConfig(lines.at(userChoice.value() - 1), rnStream);
}

std::vector<std::string> UserConfig::split(const std::string& line)
{
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;

    while (std::getline(ss, item, '|'))
        parts.push_back(item);

    return parts;
}


void UserConfig::saveOrUpdateCache(const bool rnStream) const
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = fs::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" / (rnStream ? "rnCache.txt" : "cache.txt");

    if (!fs::exists(cacheFilePath))
    {
        const std::ofstream ofs(cacheFilePath);
        if (!ofs)
        {
            std::cerr << "Failed to create config cache file: " << cacheFilePath << std::endl;
            throw std::runtime_error("Failed to create config cache file");
        }
    }

    std::vector<std::string> lines = getCachedConfigLines(rnStream);

    std::string newConfigLine =
        rnStream
            ? (parSheet.fileName + "|" +
                rnStreamFileName + "|" +
                std::to_string(rtp) + "|" +
                betType + "|" +
                std::to_string(force))
            : (parSheet.fileName + "|" +
                std::to_string(simSize) + "|" +
                std::to_string(rtp) + "|" +
                betType + "|" +
                std::to_string(force) + "|" +
                (seedChoice.has_value()
                     ? std::to_string(seed)
                     : "RAND"));

    for (const auto& line : lines)
        if (line.starts_with(newConfigLine))
            lines.erase(std::ranges::find(lines, line));

    lines.insert(lines.begin(), newConfigLine + "|" + std::to_string(seed));

    if (lines.size() > 10) lines.erase(lines.begin() + 10, lines.end());
    std::ofstream output(cacheFilePath);
    if (!output)
    {
        std::cerr << "Failed to write to config cache file for writing: " << cacheFilePath << std::endl;
    }

    for (const auto& line : lines) output << line << "\n";
}

std::vector<std::string> UserConfig::getCachedConfigLines(const bool rnStream)
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = fs::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" / (rnStream ? "rnCache.txt" : "cache.txt");
    std::ifstream cacheFile(cacheFilePath);
    if (!cacheFile)
        std::cerr << "Failed to open config cache file: " << cacheFilePath << std::endl;

    std::vector<std::string> lines;

    {
        std::string line;
        while (std::getline(cacheFile, line)) lines.push_back(line);
    }
    cacheFile.close();
    return lines;
};

std::optional<size_t> UserConfig::getUserChoiceFromList(const std::vector<std::string>& choices,
                                                        const std::string& title)
{
    gameConsole.setHeader(headerText, true, true);
    std::cout << std::endl;
    gameConsole.coutStyle("\t" + title, CONSOLE_INVERT);
    std::cout << std::endl;
    int choiceNum = 0;
    for (const auto& choice : choices)
        std::cout << "\t\t" + std::to_string(choiceNum++) + ") " + choice << std::endl;

    std::string choiceString;
    do
    {
        std::cout << "\tEnter a valid Option (or press Enter for '" + choices.front() + "') > ";
        std::getline(std::cin, choiceString);
        if (choiceString.empty()) return std::nullopt;
    }
    while (!std::ranges::all_of(choiceString, isdigit)
        || std::stoi(choiceString) >= choices.size());
    return std::stoi(choiceString);
}

std::optional<size_t> UserConfig::getUserCachedConfigChoice(
    const std::vector<std::vector<std::string>>& choices,
    const std::string& title,
    const bool rnStream)
{
    gameConsole.setHeader(headerText, true, true);
    std::cout << std::endl;
    gameConsole.coutStyle("\t" + title, CONSOLE_INVERT);
    std::cout << std::endl;

    if (rnStream)
        std::cout << std::left
            << std::setw(4) << " "
            << std::setw(40) << "PAR Sheet File Name"
            << std::setw(40) << "RN Stream File Name"
            << std::setw(5) << "RTP"
            << std::setw(15) << "Bet Type"
            << std::setw(15) << "Force"
            << "\n";
    else
        std::cout << std::left
            << std::setw(4) << " "
            << std::setw(40) << "PAR Sheet File Name"
            << std::setw(15) << "Sim Size (M)"
            << std::setw(5) << "RTP"
            << std::setw(15) << "Bet Type"
            << std::setw(15) << "Force"
            << std::setw(10) << "Seed" << '\n';
    std::cout << std::string(150, '-') << '\n';

    int choiceNum = 0;
    std::cout
        << std::setw(4) << std::to_string(choiceNum++)
        << std::setw(40) << "NEW CONFIG"
        << std::endl;

    for (const auto& choice : choices)
        if (rnStream)
            std::cout << std::left
                << std::setw(4) << std::to_string(choiceNum++) + ")"
                << std::setw(40) << choice.at(0)
                << std::setw(40) << choice.at(1)
                << std::setw(5) << choice.at(2)
                << std::setw(15) << choice.at(3)
                << std::setw(40) << choice.at(4)
                << "\n";
        else
            std::cout << std::left
                << std::setw(4) << std::to_string(choiceNum++) + ")"
                << std::setw(40) << choice.at(0)
                << std::setw(15) << choice.at(1)
                << std::setw(5) << choice.at(2)
                << std::setw(15) << choice.at(3)
                << std::setw(15) << choice.at(4)
                << std::setw(40) << choice.at(6) + (choice.at(5) == "RAND"
                                                        ? " (random - new seed will be chosen)"
                                                        : "")
                << '\n';

    std::string choiceString;

    do
    {
        std::cout << "\tEnter a valid Option (or press Enter for NEW CONFIG) > ";
        std::getline(std::cin, choiceString);
        if (choiceString.empty()) return std::nullopt;
    }
    while (!std::ranges::all_of(choiceString, isdigit) || std::stoi(choiceString) >= choices.size() + 1);

    return std::stoi(choiceString);
}

std::optional<unsigned int> UserConfig::getUserInputAsInt(const std::string& title, const std::string& prompt)
{
    gameConsole.setHeader(headerText, true, true);
    std::cout << std::endl;
    gameConsole.coutStyle("\t" + title, CONSOLE_INVERT);
    std::cout << std::endl;
    std::string input;
    do
    {
        std::cout << prompt << " > ";
        std::getline(std::cin, input);
        if (input.empty())
            return std::nullopt;
    }
    while (!std::ranges::all_of(input, isdigit));
    return std::stoi(input);
}

ParSheet UserConfig::getUserParSheetChoice()
{
    namespace fs = std::filesystem;
    const fs::path parSheetDirectory = fs::path(GAME_SOURCE_DIR) / "srcBackend" /
        "thisTitle" /
        "parSheets";
    const std::string extension = ".xlsx";
    std::vector<std::string> parSheetNames;

    try
    {
        std::vector<fs::directory_entry> files;

        for (const auto& entry : std::filesystem::directory_iterator(parSheetDirectory))
            if (entry.is_regular_file()
                && entry.path().extension() == extension
                && !entry.path().filename().string().starts_with("~$"))
                files.push_back(entry);

        std::ranges::sort(files, [](const auto& a, const auto& b)
        {
            return fs::last_write_time(a) > fs::last_write_time(b);
        });

        for (const auto& file : files)
            parSheetNames.emplace_back(file.path().filename().string());
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error reading parSheets directory: " << e.what() << std::endl;
        throw;
    }

    if (parSheetNames.empty())
    {
        std::cerr << "No parSheets found in " << parSheetDirectory << std::endl;
        throw std::runtime_error("No parSheets found");
    }

    const std::optional<size_t> userChoice = getUserChoiceFromList(parSheetNames, "Select PAR Sheet");

    ParSheet parSheet(parSheetNames.at(userChoice.value_or(0)));
    headerText += parSheet.gameTitle;
    return parSheet;
}

std::string UserConfig::getUserRnStreamFileChoice()
{
    namespace fs = std::filesystem;
    const fs::path rnFileDirectory = fs::path(GAME_SOURCE_DIR) / "srcBackend" /
        "thisTitle" /
        "parSheets";
    const std::string extension = ".csv";
    std::vector<std::string> rnFileNames;

    try
    {
        std::vector<fs::directory_entry> files;

        for (const auto& entry : std::filesystem::directory_iterator(rnFileDirectory))
            if (entry.is_regular_file()
                && entry.path().extension() == extension
                && !entry.path().filename().string().starts_with("~$"))
                files.push_back(entry);

        std::ranges::sort(files, [](const auto& a, const auto& b)
        {
            return fs::last_write_time(a) > fs::last_write_time(b);
        });

        for (const auto& file : files)
            rnFileNames.emplace_back(file.path().filename().string());
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error reading RN stream file directory: " << e.what() << std::endl;
        throw;
    }

    if (rnFileNames.empty())
    {
        std::cerr << "No CSV files found in " << rnFileDirectory << std::endl;
        throw std::runtime_error("No parSheets found");
    }

    const std::optional<size_t> userChoice = getUserChoiceFromList(rnFileNames, "Select RN Stream File");

    return rnFileNames.at(userChoice.value_or(0));
}

int UserConfig::getUserRtpChoice(const ParSheet& parSheet, const bool rnStream)
{
    std::vector<std::string> rtpOptionStrings;
    if (!rnStream) rtpOptionStrings.emplace_back("All");
    for (const int rtp : parSheet.rtps)
        rtpOptionStrings.emplace_back(std::to_string(rtp));

    const std::optional<size_t> userChoice = getUserChoiceFromList(rtpOptionStrings, "Select RTP");

    const int rtp = userChoice.has_value() || rnStream
                        ? parSheet.rtps.at(userChoice.value_or(0) - (rnStream ? 0 : 1))
                        : -1;

    headerText += (rtp > 0 ? " - RTP" + std::to_string(rtp) : " - All RTPs");
    return rtp;
}

std::string UserConfig::getUserBetTypeChoice(const ParSheet& parSheet)
{
    const std::optional<size_t> userChoice = getUserChoiceFromList(parSheet.betTypes, "Select Bet Type");
    const std::string betType = parSheet.betTypes.at(userChoice.value_or(0));
    headerText += " - " + betType;
    return betType;
}

Forces::SimForce UserConfig::getUserForceChoice()
{
    if (Forces::simForceNames.size() != Forces::NumForces)
        throw std::runtime_error(
            "Number of forces does not match number of force names in simForces.h");
    const size_t userChoice = getUserChoiceFromList(Forces::simForceNames, "Select Game Force").value_or(0);
    if (userChoice > 0) headerText += " - " + Forces::simForceNames.at(userChoice);
    return static_cast<Forces::SimForce>(userChoice);
}

std::optional<int> UserConfig::getUserSeedChoice()
{
    return getUserInputAsInt("Set Seed", "Enter RNG Seed (press Enter for random)");
}

uint64_t UserConfig::getUserSimSizeChoice()
{
    return getUserInputAsInt("SET SIM SIZE",
                             "Enter number of games to simulate in millions. (press Enter for 1b)"
    ).value_or(1000) * static_cast<uint64_t>(MILLION);
}

Forces::SimForce UserConfig::stringToForce(const std::string& stringValue)
{
    const int intValue = std::stoi(stringValue);
    if (intValue >= Forces::NumForces)
        throw std::runtime_error(
            "Invalid Force value from cached user config. It may be you have removed forces from simForces since running this config.");
    if (intValue < 0)
        throw
            std::runtime_error("Force value from cached config is negative. That's weird yo.");

    return static_cast<Forces::SimForce>(intValue);
}
