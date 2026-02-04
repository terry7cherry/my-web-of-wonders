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
    config["displayReelSet"] = reelSets["reelSet1"].toJson();
    config["suspenseSymbolCode"] = symbols["fsTriggerSymbol"];
    return config;
}
