#pragma once

// cpp-dev-library
#include "PayGrid.h"
#include "../console/Console.h"
#include "../gameMaths/mathElements/PayLines.h"
#include "LineWin.h"


// game globals
extern Console gameConsole;

#define MAX_LINE_TEXT_LEN 45


// A reel Grid with Line wins
class LineGrid :
    public PayGrid
{
protected:
    std::string szWinLineHeader = "Line Wins";
    const PayLines* lpPayLines = nullptr;

public:
    std::vector<LineWin> winVector;
    std::string szBlankWin;

    // default constructor
    // winDirection is left_right
    LineGrid(const size_t reels, const size_t rows, const PayTable& payTable) : PayGrid(reels, rows, payTable)
    {
        szBlankWin = std::string(MAX_LINE_TEXT_LEN, ' ');
        winVector.reserve(100);
    }

    LineGrid(const size_t reels, const size_t rows, const PayTable& payTable,
             const PayLines& payLines) : LineGrid(reels, rows, payTable)
    {
        setPayLines(payLines);
    }

    LineGrid(const GameMaths& maths)
        : LineGrid(maths.ints["numReels"],
                   maths.ints["numRows"],
                   maths.payTables["default"],
                   maths.payLines["default"])
    {
    }

    // Pass the required PayLines
    void setPayLines(const PayLines& payLineData)
    {
        lpPayLines = &payLineData;
        winVector.resize(lpPayLines->lineCount);
    }

    // extend the method PayGrid::reset()
    void reset(void)
    {
        PayGrid::reset();
    }


    /************************************/
    // Win evaluation methods
    /************************************/

    // Reset the winVector then calculate wins. Applys line multipliers in multiplierVect
    // Re-populate winVector and return total win
    unsigned int evaluateWins(unsigned int betMultiplier, [[maybe_unused]] bool doMeters,
                              [[maybe_unused]] bool useDebugLine = false);


    // Identify winning symbols in the grid with the following:
    //      set vector <vector <bool>> isWinVect   // [reel][row]
    //      set vector <int> winIndexVect          // undefined size, populate with index of winning positions
    // return the number of winning symbols, winIndexVect.size()
    int setWinningSymbols();


    // count the number of active multipliers in the passed win
    int countWinMultipliers(const LineWin& lineWin) const;

    // count the number of multipliers in multiplierVect that are involved in wins
    int countActiveWinMultipliers();

    // Applies Grid.gridMultiplier (or optional passed, specialMultiplier) to all wins
    // Applies multiplier to each win and returns total win after multiplier is applied
    unsigned int applyGridMultiplier(MultipliersCombineType combineType, int specialMultiplier = 0);


    // check each win to see if includes a value in 'multiplierVect'
    // if more than 1 symbol in a win contains a multiplier, then they are combined using the passed combineType
    //      MultipliersCombineType::multiply or MultipliersCombineType::add
    // Returns the total win after multipliers are applied to all wins
    unsigned int applyWinMultipliers(MultipliersCombineType combineType);

    /****************************************/
    // Display methods
    /****************************************/

    // Clear any previous win data
    // Show the winning line data to screen, 1 line per screen row
    int showWins(int startRow, int col, bool showTitle = true, bool showDirection = false) const;


    // returns all winnning line data in vector <string>
    std::vector<std::string> getWinTextVect(std::map<int, std::string>& symbolMap,
                                            bool showDirection = false) const;


    // return pay line at index
    const std::vector<size_t>* getPayLine(const int payLineNum) const
    {
        return &lpPayLines->lineVect[payLineNum];
    }
};
