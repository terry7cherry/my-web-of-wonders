#pragma once

/*
#include <unordered_map>
#include <string>
#include "../common/mathElements/ReelSet.h"
#include "../common/mathElements/WeightTables.h"
#include "../common/mathElements/PayTable.h"
#include "../common/mathElements/PayLines.h"

struct GameMaths
{
    const std::string version = "1.0.0";
    const std::string gameName = "Mini Project";
    const int numReels = 5;
    const int numRows = 3;
    const int numPayLines = 25;
    const int minBet = 25;
    const int rtp = 96;
    const std::vector<std::string> rtpOptionVect = {"96"};

    std::unordered_map<std::string, int> symbolMap;
    std::unordered_map<std::string, int> reelNameMap;
    std::vector<ReelSet> reelSetVect;
    ReelSet* baseReelSet = new ReelSet(
        "baseReelSet", numReels,
        std::vector<std::vector<int>>{
            {
                0, 9, 8, 3, 4, 0, 5, 4, 9, 2, 7, 5, 1, 2, 9,
                4, 6, 5, 9, 8, 1, 4, 6, 2, 8, 3, 7, 8
            },
            {
                8, 9, 3, 2, 8, 6, 0, 3, 9, 1, 2, 6, 8, 7, 3,
                6, 1, 7, 9, 6, 3, 4, 1, 3, 2, 8, 3, 4, 8, 5,
                8,
                4, 6,
                9, 8, 4, 8, 5, 4, 8
            },
            {
                3, 9, 5, 7, 0, 3, 6, 8, 9, 7, 1, 8, 2, 7, 3,
                6, 7, 2, 9, 6, 4, 7, 2, 6, 7, 4, 6, 1, 2, 9,
                5,
                6, 2, 0
            },
            {
                7, 9, 3, 5, 9, 1, 5, 7, 9, 8, 0, 5, 8, 2, 8,
                3, 6, 1, 9, 5, 3, 6, 8, 2, 8, 6, 3, 4, 8, 2,
                6,
                8, 4,
                6, 3
            },
            {
                9, 3, 2, 1, 4, 3, 0, 2, 9, 7, 2, 6, 3, 1, 5,
                8, 4, 3, 9, 6, 0, 1, 2, 7, 4, 5, 1, 8, 2, 6,
                4,
                1
            }
        }
    );

    ReelSet* fsReelSet = new ReelSet(
        "freeSpinsReelSet", numReels, {
            {0, 5, 8, 3, 4, 0, 5, 4, 3, 2, 7, 5, 1, 2, 9, 4, 6, 5, 3, 8, 1, 4, 6, 2, 8, 3, 7, 8},
            {
                8, 7, 3, 2, 8, 6, 0, 3, 7, 1, 2, 6, 8, 7, 3, 6, 1, 7, 4, 6, 3, 4, 1, 3, 2, 8, 3, 4,
                8, 5, 8,
                4, 6,
                9, 8, 4, 8, 5, 4, 8
            },
            {
                3, 7, 5, 7, 0, 3, 6, 8, 2, 7, 1, 8, 2, 7, 3, 6, 7, 2, 1, 6, 4, 7, 2, 6, 7, 4, 6, 1,
                2, 9, 5,
                6, 2, 0
            },
            {
                7, 6, 3, 5, 9, 1, 5, 7, 3, 8, 0, 5, 8, 2, 8, 3, 6, 1, 6, 5, 3, 6, 8, 2, 8, 6, 3, 4,
                8, 2, 6,
                8, 4,
                6, 3
            },
            {
                9, 3, 2, 1, 4, 3, 0, 2, 1, 7, 2, 6, 3, 1, 5, 8, 4, 3, 2, 6, 0, 1, 2, 7, 4, 5, 1, 8,
                2, 6, 4,
                1
            },
        }
    );


    ReelSetWeightTable bgReelWeights = ReelSetWeightTable({100}, {baseReelSet});
    ReelSetWeightTable fsReelWeights = ReelSetWeightTable({100}, {fsReelSet});
    WeightTable fsMultiplierWeights = WeightTable({
            WeightData({2, 100}),
            WeightData({3, 100}),
            WeightData({4, 100}),
            WeightData({5, 100})
        }
    );
    std::unordered_map<int, int> fsStartSpins;
    std::unordered_map<int, int> fsRetriggerSpins;
    //PayTable payTable = PayTable({
    //    {0, 0, 0, 25, 100, 1000, 0}, // W1
    //    {0, 0, 0, 20, 75, 500, 0}, // H1
    //    {0, 0, 0, 20, 50, 200, 0}, // H2
    //    {0, 0, 0, 15, 30, 100, 0}, // H3
    //    {0, 0, 0, 15, 30, 100, 0}, // H4
    //    {0, 0, 0, 10, 25, 75, 0}, // L1
    //    {0, 0, 0, 10, 25, 75, 0}, // L2
    //    {0, 0, 0, 5, 20, 50, 0}, // L3
    //    {0, 0, 0, 5, 20, 50, 0}, // L4
    //    {0, 0, 0, 5, 20, 100}, // SC
    //});

    PayLines payLines = PayLines(
        {
            {1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0},
            {2, 2, 2, 2, 2},
            {0, 1, 2, 1, 0},
            {2, 1, 0, 1, 2},
            {1, 0, 1, 0, 1},
            {1, 2, 1, 2, 1},
            {0, 1, 0, 1, 0},
            {2, 1, 2, 1, 2},
            {0, 1, 1, 1, 0},
            {2, 1, 1, 1, 2},
            {1, 0, 0, 0, 1},
            {1, 2, 2, 2, 1},
            {0, 0, 1, 0, 0},
            {2, 2, 1, 2, 2},
            {1, 1, 0, 1, 1},
            {1, 1, 2, 1, 1},
            {2, 0, 0, 0, 2},
            {0, 2, 2, 2, 0},
            {2, 2, 0, 2, 2},
            {0, 0, 2, 0, 0},
            {2, 0, 1, 0, 2},
            {0, 2, 1, 2, 0},
            {2, 0, 2, 0, 2},
            {0, 2, 0, 2, 0},
        }
    );

    std::map<int, std::string> symbolTextMap = {
        {-1, "  "},
        {1, "H1"},
        {2, "H2"},
        {3, "H3"},
        {4, "H4"},
        {5, "L1"},
        {6, "L2"},
        {7, "L3"},
        {8, "L4"},
        {0, "W1"},
        {9, "SC"}
    };


    GameMaths()
    {
        symbolMap["WI"] = 0;
        symbolMap["H1"] = 1;
        symbolMap["H2"] = 2;
        symbolMap["H3"] = 3;
        symbolMap["H4"] = 4;
        symbolMap["L1"] = 5;
        symbolMap["L2"] = 6;
        symbolMap["L3"] = 7;
        symbolMap["L4"] = 8;
        symbolMap["SC"] = 9;
        symbolMap["WI"] = 10;

        reelNameMap["baseGame1"] = 0;
        reelNameMap["freeSpins1"] = 1;

        fsStartSpins[0] = 0;
        fsStartSpins[1] = 0;
        fsStartSpins[2] = 0;
        fsStartSpins[3] = 10;
        fsStartSpins[4] = 20;
        fsStartSpins[5] = 50;

        fsRetriggerSpins[0] = 0;
        fsRetriggerSpins[1] = 0;
        fsRetriggerSpins[2] = 5;
        fsRetriggerSpins[3] = 10;
        fsRetriggerSpins[4] = 20;
        fsRetriggerSpins[5] = 50;
        reelSetVect.push_back(*baseReelSet);
        reelSetVect.push_back(*fsReelSet);
    }

    std::vector<std::string> getReelList() const
    {
        std::vector<std::string> reelList;
        reelList.reserve(reelSetVect.size());
        for (const ReelSet reelSet : reelSetVect)
        {
            reelList.push_back(reelSet.mSzName);
        }
        return reelList;
    }
};
*/
