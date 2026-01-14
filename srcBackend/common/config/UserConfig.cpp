#include <filesystem>
#include <fstream>
#include <sstream>

#include "UserConfig.h"
#include "../gameMaths/parser/ParSheet.h"
#include "../console/Console.h"
#include "../../thisTitle/forces/simForces.h"

UserConfig::UserConfig()
    : simSize(getUserSimSizeChoice()),
      parSheet(getUserParSheetChoice()),
      rtp(getUserRtpChoice(parSheet)),
      betType(getUserBetTypeChoice(parSheet)),
      force(getUserForceChoice()),
      seedChoice(getUserSeedChoice()),
      seed(seedChoice.value_or(std::rand()))
{
    saveOrUpdateCache();
}

UserConfig::UserConfig(const std::string& cachedConfig)
    : simSize(std::stoi(split(cachedConfig).at(1))),
      parSheet(split(cachedConfig).at(0)),
      rtp(std::stoi(split(cachedConfig).at(2))),
      betType(split(cachedConfig).at(3)),
      force(std::stoi(split(cachedConfig).at(4))),
      seedChoice(split(cachedConfig).at(5) == "RAND"
                     ? std::nullopt
                     : std::optional<int>(std::stoi(split(cachedConfig).at(6)))),
      seed(seedChoice.value_or(std::rand()))
{
    saveOrUpdateCache();
    headerText += " - " + parSheet.gameTitle;
    headerText += " - " + (rtp > 0 ? " RTP" + std::to_string(rtp) : " All RTPs");
    headerText += " - " + betType;
    if (force) headerText += " - " + SimForces::forceNames.at(force);
}

UserConfig UserConfig::getNewOrCachedConfig()
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = fs::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" / "cache.txt";
    if (!fs::exists(cacheFilePath)) return UserConfig();

    std::vector<std::string> lines = getCachedConfigLines();

    std::vector<std::vector<std::string>> configChoices;
    for (const auto& line : lines)
    {
        const std::vector<std::string> elements = split(line);
        std::vector<std::string> choice;
        choice.emplace_back(elements.at(0));
        choice.emplace_back(std::to_string(std::stoi(elements.at(1)) / MILLION));
        choice.emplace_back(elements.at(2));
        choice.emplace_back(elements.at(3));
        choice.emplace_back(SimForces::forceNames.at(std::stoi(elements.at(4))));
        choice.emplace_back(elements.at(5));
        choice.emplace_back(elements.at(6));
        configChoices.emplace_back(choice);
    }

    const std::optional<size_t> userChoice = getUserCachedConfigChoice(configChoices, "Select Cached Config");
    if (userChoice.value_or(0) == 0) return UserConfig();
    return UserConfig(lines.at(userChoice.value() - 1));
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


void UserConfig::saveOrUpdateCache() const
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = fs::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" / "cache.txt";

    if (!fs::exists(cacheFilePath))
    {
        const std::ofstream ofs(cacheFilePath);
        if (!ofs)
        {
            std::cerr << "Failed to create config cache file: " << cacheFilePath << std::endl;
            throw std::runtime_error("Failed to create config cache file");
        }
    }

    std::vector<std::string> lines = getCachedConfigLines();

    std::string newConfigLine =
        parSheet.fileName + "|" +
        std::to_string(simSize) + "|" +
        std::to_string(rtp) + "|" +
        betType + "|" +
        std::to_string(force) + "|" +
        (seedChoice.has_value()
             ? std::to_string(seed)
             : "RAND");

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

std::vector<std::string> UserConfig::getCachedConfigLines()
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = fs::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" / "cache.txt";
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
        std::cout << "\tEnter a valid Option (or press Enter for '" + choices.front() + "' > ";
        std::getline(std::cin, choiceString);
        if (choiceString.empty()) return std::nullopt;
    }
    while (!std::ranges::all_of(choiceString, isdigit)
        || std::stoi(choiceString) >= choices.size());
    return std::stoi(choiceString);
}

std::optional<size_t> UserConfig::getUserCachedConfigChoice(
    const std::vector<std::vector<std::string>>& choices,
    const std::string& title)
{
    gameConsole.setHeader(headerText, true, true);
    std::cout << std::endl;
    gameConsole.coutStyle("\t" + title, CONSOLE_INVERT);
    std::cout << std::endl;

    std::cout << std::left
        << std::setw(4) << " "
        << std::setw(60) << "File Name"
        << std::setw(15) << "Sim Size (M)"
        << std::setw(5) << "RTP"
        << std::setw(15) << "Bet Type"
        << std::setw(15) << "Force"
        << std::setw(10) << "Seed" << '\n';
    std::cout << std::string(150, '-') << '\n';
    int choiceNum = 0;
    std::cout
        << std::setw(4) << std::to_string(choiceNum++)
        << std::setw(40) << "NEW CONFIG" << std::endl;

    for (const auto& choice : choices)
        std::cout << std::left
            << std::setw(4) << std::to_string(choiceNum++) + ")"
            << std::setw(60) << choice.at(0)
            << std::setw(15) << choice.at(1)
            << std::setw(5) << choice.at(2)
            << std::setw(15) << choice.at(3)
            << std::setw(15) << choice.at(4)
            << std::setw(40) << choice.at(6) + (choice.at(5) == "RAND"
                                                    ? " (randomly chosen - new random seed will be chosen)"
                                                    : "")
            << '\n';

    std::string choiceString;
    do
    {
        std::cout << "\tEnter a valid Option (or press Enter for NEW CONFIG) > ";
        std::getline(std::cin, choiceString);
        if (choiceString.empty()) return std::nullopt;
    }
    while (!std::ranges::all_of(choiceString, isdigit)
        || std::stoi(choiceString) >= choices.size());
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

int UserConfig::getUserRtpChoice(const ParSheet& parSheet)
{
    std::vector<std::string> rtpOptionStrings;
    rtpOptionStrings.emplace_back("All");
    for (const int rtp : parSheet.rtps)
        rtpOptionStrings.emplace_back(std::to_string(rtp));

    const std::optional<size_t> userChoice = getUserChoiceFromList(rtpOptionStrings, "Select RTP");

    const int rtp = userChoice.has_value() ? parSheet.rtps.at(userChoice.value() - 1) : -1;

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

int UserConfig::getUserForceChoice()
{
    const size_t userChoice = getUserChoiceFromList(SimForces::forceNames, "Select Game Force").value_or(0);
    if (userChoice > 0) headerText += " - " + SimForces::forceNames.at(userChoice);
    return static_cast<int>(userChoice);
}

std::optional<int> UserConfig::getUserSeedChoice()
{
    return getUserInputAsInt("Set Seed", "Enter RNG Seed (press Enter for random)");
}

uint64_t UserConfig::getUserSimSizeChoice()
{
    return getUserInputAsInt("SET SIM SIZE",
                             "Enter number of games to simulate. (press Enter for 1b)"
    ).value_or(1000) * MILLION;
}
