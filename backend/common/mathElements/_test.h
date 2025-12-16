#pragma once

#include <unordered_map>
#include <string>

struct TestClass {
    std::unordered_map<std::string, int> reelNameMap;

    TestClass() {
        reelNameMap["baseGame1"] = 0;
        reelNameMap["freeSpins1"] = 1;
        int reelSetIndex = reelNameMap["baseGame1"];
    }
};
