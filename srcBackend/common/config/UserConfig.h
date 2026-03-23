#pragma once

#include "../gameMaths/parser/ParSheet.h"
#include "../../thisTitle/forces/simForces.h"
#include "../console/Console.h"
#include "../stats/SessionStrategy.h"

class UserConfig
{
public:
    UserConfig();

    explicit UserConfig(const std::string& cachedConfig);
    static UserConfig getNewOrCachedConfig();

    inline static Console gameConsole;
    inline static std::string headerText = "";
    ParSheet parSheet;
#ifndef RN_STREAM
    uint64_t simSize;
#else
    std::string rnStreamFileName;
#endif
    int rtp;
    std::string betType;
    Forces::SimForce force;
#ifdef SESSION
    size_t sessionStrategyIndex;
    SessionStrategy sessionStrategy;
#endif
#ifndef RN_STREAM
    std::optional<int> seedChoice;
    int seed;
#endif

private:
    void saveOrUpdateCache() const;

    static std::vector<std::string> getCachedConfigLines();

    static std::vector<std::string> split(const std::string& line);

    static std::optional<size_t> getUserChoiceFromList(const std::vector<std::string>& choices,
                                                       const std::string& title);

    static std::optional<size_t> getUserCachedConfigChoice(
        const std::vector<std::vector<std::string>>& choices,
        const std::string& title);

    static std::filesystem::path getCacheFilePath();

    static std::optional<unsigned int> getUserInputAsInt(const std::string& title, const std::string& prompt);

    static ParSheet getUserParSheetChoice();

    static std::string getUserRnStreamFileChoice();

    static int getUserRtpChoice(const ParSheet& parSheet);

    static std::string getUserBetTypeChoice(const ParSheet& parSheet);

    static Forces::SimForce getUserForceChoice();

    static size_t getUserSessionStrategyIndexChoice();

    static std::optional<int> getUserSeedChoice();

    static uint64_t getUserSimSizeChoice();

    static Forces::SimForce stringToForce(const std::string& stringValue);
};
