#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum CollectorPosition
{
    TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT
};

inline std::vector<std::string> collectorPositionNames = {
    "topLeft", "topRight", "bottomLeft", "bottomRight"
};

inline std::unordered_map<std::string, CollectorPosition> collectorPositionMap = {
    {"topLeft", TOP_LEFT},
    {"topRight", TOP_RIGHT},
    {"bottomLeft", BOTTOM_LEFT},
    {"bottomRight", BOTTOM_RIGHT}
};
