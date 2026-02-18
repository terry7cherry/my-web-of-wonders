#pragma once

#include "../gameMaths/parser/ParSheet.h"
#include "../../thisTitle/forces/simForces.h"
#include "../console/Console.h"

class UserConfig
{
public:
    UserConfig(bool rnStream = false);

    explicit UserConfig(const std::string& cachedConfig, bool rnStream = false);
    static UserConfig getNewOrCachedConfig(bool rnStream = false);

    inline static Console gameConsole;
    inline static std::string headerText = "";
    uint64_t simSize;
    ParSheet parSheet;
    std::string rnStreamFileName;
    int rtp;
    std::string betType;
    Forces::SimForce force;
    std::optional<int> seedChoice;
    int seed;

private:
    void saveOrUpdateCache(bool rnStream = false) const;

    static std::vector<std::string> getCachedConfigLines(bool rnStream = false);

    static std::vector<std::string> split(const std::string& line);

    static std::optional<size_t> getUserChoiceFromList(const std::vector<std::string>& choices,
                                                       const std::string& title);

    static std::optional<size_t> getUserCachedConfigChoice(
        const std::vector<std::vector<std::string>>& choices,
        const std::string& title,
        bool rnStream = false);

    static std::optional<unsigned int> getUserInputAsInt(const std::string& title, const std::string& prompt);

    static ParSheet getUserParSheetChoice();

    static std::string getUserRnStreamFileChoice();

    static int getUserRtpChoice(const ParSheet& parSheet, bool rnStream = false);

    static std::string getUserBetTypeChoice(const ParSheet& parSheet);

    static Forces::SimForce getUserForceChoice();

    static std::optional<int> getUserSeedChoice();

    static uint64_t getUserSimSizeChoice();

    static Forces::SimForce stringToForce(const std::string& stringValue);
};
