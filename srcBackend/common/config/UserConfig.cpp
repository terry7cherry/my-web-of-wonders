#include <filesystem>
#include <fstream>
#include <sstream>

#include "UserConfig.h"

#include <random>

#include "../gameMaths/parser/ParSheet.h"
#include "../console/Console.h"
#include "../../thisTitle/forces/simForces.h"
#include "../../thisTitle/stats/SessionStrategies.h"

UserConfig::UserConfig() :
    parSheet(getUserParSheetChoice()),
#ifndef RN_STREAM
    simSize(getUserSimSizeChoice()),
#else
    rnStreamFileName(getUserRnStreamFileChoice()),
#endif
    rtp(getUserRtpChoice(parSheet)),
    betType(getUserBetTypeChoice(parSheet)),
    force(getUserForceChoice())
#ifdef SESSION
    ,
    sessionStrategyIndex(getUserSessionStrategyIndexChoice()),
    sessionStrategy(SessionStats::sessionStrategies.at(sessionStrategyIndex))
#endif
#ifndef RN_STREAM
    ,
    seedChoice(getUserSeedChoice()),
    seed(seedChoice.value_or(std::random_device{}()))
#endif

{
    saveOrUpdateCache();
}


UserConfig::UserConfig(const std::string& cachedConfig) :
    parSheet(split(cachedConfig).at(0)),
#ifndef RN_STREAM
    simSize(std::stoull(split(cachedConfig).at(1))),
#else
    rnStreamFileName(split(cachedConfig).at(1)),
#endif
    rtp(std::stoi(split(cachedConfig).at(2))),
    betType(split(cachedConfig).at(3)),
    force(stringToForce(split(cachedConfig).at(4)))
#ifdef SESSION
    ,
    sessionStrategyIndex(std::stoi(split(cachedConfig).at(6))),
    sessionStrategy(SessionStats::sessionStrategies.at(sessionStrategyIndex))
#endif
#ifndef RN_STREAM
    , seedChoice(split(cachedConfig).at(5) == "RAND"
                     ? std::nullopt
                     : std::optional(std::stoi(split(cachedConfig).at(6)))
    ),
    seed(seedChoice.value_or(std::random_device{}()))
#endif


{
    saveOrUpdateCache();
    headerText += " - " + parSheet.gameTitle;
    headerText += " - RTP" + std::to_string(rtp);
    headerText += " - " + betType;
    if (force) headerText += " - " + Forces::simForceNames.at(force);
}

UserConfig::UserConfig(const int argc, char* argv[]) :
    parSheet(hasFlag({"-p", "--parSheet"}, argc, argv)
                 ? ParSheet(getArgumentByFlag({"-p", "--parSheet"}, argc, argv).value())
                 : getUserParSheetChoice()),
#ifndef RN_STREAM
    simSize(
        hasFlag({"-r", "--simSize"}, argc, argv)
            ? (std::stoll(getArgumentByFlag({"-r", "--simSize"}, argc, argv).value())
#ifndef SESSION
                *static_cast<uint64_t>(MILLION)
#endif
            )
            : getUserSimSizeChoice()),
#else
    rnStreamFileName(
        hasFlag({"--rnStream"}, argc, argv)
            ? getArgumentByFlag({"--rnStream"}, argc, argv).value()
            : getUserRnStreamFileChoice()),
#endif
    rtp(
        hasFlag({"--rtp"}, argc, argv)
            ? std::stoi(getArgumentByFlag({"--rtp"}, argc, argv).value())
            : getUserRtpChoice(parSheet)),
    betType(
        hasFlag({"-b", "--betType"}, argc, argv)
            ? getArgumentByFlag({"-b", "--betType"}, argc, argv).value()
            : getUserBetTypeChoice(parSheet))
    , force(hasFlag({"-f", "--force"}, argc, argv)
                ? static_cast<Forces::SimForce>(std::stoi(
                    getArgumentByFlag({"-f", "--force"}, argc, argv).value()))
                : getUserForceChoice())
#ifdef SESSION
    , sessionStrategyIndex(
        hasFlag({"--sessionStrategy"}, argc, argv)
            ? std::stoi(getArgumentByFlag({"--sessionStrategy"}, argc, argv).value())
            : getUserSessionStrategyIndexChoice()
    ),
    sessionStrategy(SessionStats::sessionStrategies.at(sessionStrategyIndex))
#endif
#ifndef RN_STREAM
    , seedChoice(
        hasFlag({"-s", "--seed"}, argc, argv)
            ? (getArgumentByFlag({"-s", "--seed"}, argc, argv).value().starts_with("r")
               || getArgumentByFlag({"-s", "--seed"}, argc, argv).value().starts_with("R")
                   ? std::nullopt
                   : std::optional(std::stoi(getArgumentByFlag({"-r", "--rtp"}, argc, argv).value())))
            : getUserSeedChoice()
    ),
    seed(seedChoice.value_or(std::random_device{}()))
#endif
{
    saveOrUpdateCache();
}

UserConfig UserConfig::getNewOrCachedConfig(int argc, char* argv[])
{
    namespace fs = std::filesystem;
    const fs::path cacheFilePath = getCacheFilePath();
    if (!fs::exists(cacheFilePath) || argc > 1) return UserConfig(argc, argv);
    const std::vector<std::string> lines = getCachedConfigLines();
    std::vector<std::vector<std::string>> configChoices;
    for (const auto& line : lines)
    {
        std::vector<std::string> choice;
        const std::vector<std::string> elements = split(line);
        size_t positionIndex = 0;
        choice.emplace_back(elements.at(positionIndex++));
#ifdef RN_STREAM
        choice.emplace_back(elements.at(positionIndex++));
#else
        choice.emplace_back(std::to_string(std::stoull(elements.at(positionIndex++))
#ifndef SESSION
            / MILLION
#endif
        ));
#endif
        choice.emplace_back(elements.at(positionIndex++));
        choice.emplace_back(elements.at(positionIndex++));
        choice.emplace_back(Forces::simForceNames.at(std::stoi(elements.at(positionIndex++))));
#ifndef RN_STREAM
        choice.emplace_back(elements.at(positionIndex++));
        choice.emplace_back(elements.at(positionIndex++));
#endif
#ifdef SESSION
        choice.emplace_back(elements.at(positionIndex++));
#endif
        configChoices.emplace_back(choice);
    }

    const std::optional<size_t> userChoice = getUserCachedConfigChoice(
        configChoices, "Select Cached Config");
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
    const fs::path cacheFilePath = getCacheFilePath();

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
            parSheet.fileName
#ifdef RN_STREAM
            +"|" + rnStreamFileName
#else
            + "|" + std::to_string(simSize)
#endif
            + "|" + std::to_string(rtp)
            + "|" + betType
            + "|" + std::to_string(force)
#ifndef RN_STREAM
            + "|" + (seedChoice.has_value()
                         ? std::to_string(seed)
                         : "RAND")
#endif
#ifdef SESSION
            + "|" + std::to_string(sessionStrategyIndex)
#endif
        ;

    for (const auto& line : lines)
        if (line.starts_with(newConfigLine))
            lines.erase(std::ranges::find(lines, line));

#ifndef RN_STREAM
    lines.insert(lines.begin(), newConfigLine + "|" + std::to_string(seed));
#else
    lines.insert(lines.begin(), newConfigLine);
#endif

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
    const fs::path cacheFilePath = getCacheFilePath();
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

std::filesystem::path UserConfig::getCacheFilePath()
{
    return std::filesystem::path(GAME_SOURCE_DIR)
        / "srcBackend" / "common" / "config" /
#ifdef RN_STREAM
        "rnCache.txt";
#else
#ifdef SESSION
        "sessionCache.txt";
#else
    "cache.txt";
#endif
#endif
}

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
    const std::string& title)
{
    gameConsole.setHeader(headerText, true, true);
    std::cout << std::endl;
    gameConsole.coutStyle("\t" + title, CONSOLE_INVERT);
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << std::left
        << std::setw(4) << " "
        << std::setw(50) << "PAR Sheet File Name"
#ifdef RN_STREAM
        << std::setw(50) << "RN Stream File Name"
#else
#ifdef SESSION
        << std::setw(21) << "Sim Size (sessions)"
#else
        << std::setw(15) << "Sim Size (M)"
#endif
#endif
        << std::setw(5) << "RTP"
        << std::setw(25) << "Bet Type"
        << std::setw(25) << "Force"
#ifdef SESSION
        << std::setw(25) << "Session Strategy"
#endif
#ifndef RN_STREAM
        << std::setw(10) << "Seed"
#endif
        << '\n';
    std::cout << std::string(200, '-') << '\n';

    int choiceNum = 0;
    std::cout << std::left << std::setw(4) << std::to_string(choiceNum++) + ")"
        << std::setw(50) << "NEW CONFIG"
        << std::endl;

    for (const auto& choice : choices)
        std::cout << std::left << std::setw(4) << std::to_string(choiceNum++) + ")"
            << std::setw(50) << choice.at(0)
#ifdef RN_STREAM
            << std::setw(50) << choice.at(1)
#else
#ifdef SESSION
            << std::setw(21) << choice.at(1)
#else
            << std::setw(15) << choice.at(1)
#endif
#endif
            << std::setw(5) << choice.at(2)
            << std::setw(25) << choice.at(3)
            << std::setw(25) << choice.at(4)
#ifdef SESSION
            << std::setw(25) << SessionStats::sessionStrategies.at(stoi(choice.at(6))).name
            << std::setw(40) << choice.at(7) + (choice.at(5) == "RAND"
                                                    ? " (random - new seed will be chosen)"
                                                    : "")
#else
#ifndef RN_STREAM
            << std::setw(40) << choice.at(6) + (choice.at(5) == "RAND"
                                                    ? " (random - new seed will be chosen)"
                                                    : "")
#endif
#endif
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

int UserConfig::getUserRtpChoice(const ParSheet& parSheet)
{
    std::vector<std::string> rtpOptionStrings;
    for (const int rtp : parSheet.rtps)
        rtpOptionStrings.emplace_back(std::to_string(rtp));

    std::ranges::sort(rtpOptionStrings, [](const auto& a, const auto& b) { return a > b; });

    const std::optional<size_t> userChoice = getUserChoiceFromList(rtpOptionStrings, "Select RTP");

    const int rtp = parSheet.rtps.at(userChoice.value_or(0));

    headerText += " - RTP" + std::to_string(rtp);
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

size_t UserConfig::getUserSessionStrategyIndexChoice()
{
    if (SessionStats::sessionStrategies.empty())
        throw std::runtime_error(
            "No session strategies found. Please add session strategies to the vector in srcBackend/thisTitle/stats/SessionStrateiges.h");
    std::vector<std::string> sessionStrategyNames;
    for (const SessionStrategy& sessionStrategy : SessionStats::sessionStrategies)
        sessionStrategyNames.emplace_back(sessionStrategy.name);

    const size_t userChoice = getUserChoiceFromList(sessionStrategyNames, "Select Session Strategy").
        value_or(0);
    headerText += " - " + sessionStrategyNames.at(userChoice);
    return userChoice;
}


std::optional<int> UserConfig::getUserSeedChoice()
{
    return getUserInputAsInt("Set Seed", "Enter RNG Seed (press Enter for random)");
}

uint64_t UserConfig::getUserSimSizeChoice()
{
    return getUserInputAsInt("SET SIM SIZE",
#ifdef SESSION
                             "Enter number of sessions to simulate. (press Enter for 1000)"
#else
                             "Enter number of games to simulate in millions. (press Enter for 1b)"
#endif
        ).value_or(1000)
#ifndef SESSION
        *static_cast<uint64_t>(MILLION)
#endif
        ;
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

bool UserConfig::hasFlag(const std::vector<std::string>& flags, const int argc, char* argv[])
{
    for (size_t i = 0; i < argc; i++)
        for (const std::string& flag : flags)
            if (flag == argv[i]) return true;
    return false;
}

std::optional<std::string> UserConfig::getArgumentByFlag(const std::vector<std::string>& flags,
                                                         const int argc,
                                                         char* argv[])
{
    for (size_t i = 0; i < argc; i++)
        for (const std::string& flag : flags)
            if (flag == argv[i]) return argv[i + 1];
    return std::nullopt;
}
