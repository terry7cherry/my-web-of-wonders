#include "../../common/gameMaths/GameMaths.h"

Json::Value GameMaths::toJson() const
{
    Json::Value config;
    config["gameTitle"] = gameTitle;
    config["numRow"] = ints["numRows"];
    config["numReels"] = ints["numReels"];
    return config;
}
