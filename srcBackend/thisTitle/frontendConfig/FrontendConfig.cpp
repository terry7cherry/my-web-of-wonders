#include "../../common/gameMaths/GameMaths.h"

Json::Value GameMaths::toJson() const
{
    Json::Value config;
    config["gameTitle"] = gameTitle;
    config["numRows"] = ints["numRows"];
    config["numReels"] = ints["numReels"];
    config["minBet"] = minBet;
    config["betTypeCost"] = betTypeCost;
    config["minBetCost"] = minBetCost;
    config["payTable"] = payTables["default"].awardsToJson();
    config["symbolNames"] = payTables["default"].symbolNamesToJson();
    config["displayReelSet"] = reelSets["reelSetA"].toJson(payTables["default"].symbolNames);
    return config;
}

std::string GameMaths::getFrontendMathsSheetName()
{
    return "HerrSpider_V0_0_1 ParSheet.xlsx";
}
