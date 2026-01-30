#include "../../common/gameMaths/GameMaths.h"

Json::Value GameMaths::toJson() const
{
    Json::Value config;
    config["gameTitle"] = gameTitle;
    config["numRow"] = ints["numRows"];
    config["numReels"] = ints["numReels"];
    config["payTable"] = payTables["default"].toJson();
    config["displayReelSet"] = reelSets["reelSet1"].toJson();
    return config;
}
