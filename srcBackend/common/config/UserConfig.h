#pragma once

#include "../gameMaths/parser/ParSheet.h"
#include "../console/Console.h"

class UserConfig
{
public:
    UserConfig();

    UserConfig(const std::string& cachedConfig);
    static UserConfig getNewOrCachedConfig();

    inline static Console gameConsole;
    inline static std::string headerText = "";
    uint64_t simSize;
    ParSheet parSheet;
    int rtp;
    std::string betType;
    int force;
    std::optional<int> seedChoice;
    int seed;

private:
    void saveOrUpdateCache() const;


    static std::vector<std::string> getCachedConfigLines();

    static std::vector<std::string> split(const std::string& line);

    static std::optional<size_t> getUserChoiceFromList(const std::vector<std::string>& choices,
                                                       const std::string& title);

    static std::optional<size_t> getUserCachedConfigChoice(
        const std::vector<std::vector<std::string>>& choices,
        const std::string& title);

    static std::optional<unsigned int> getUserInputAsInt(const std::string& title, const std::string& prompt);

    static ParSheet getUserParSheetChoice();

    static int getUserRtpChoice(const ParSheet& parSheet);

    static std::string getUserBetTypeChoice(const ParSheet& parSheet);

    static int getUserForceChoice();

    static std::optional<int> getUserSeedChoice();

    static uint64_t getUserSimSizeChoice();
};
