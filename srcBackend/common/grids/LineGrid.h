#pragma once
/**
 * @file LineGrid.h
 * @brief Grid that evaluates traditional payline wins and prints results.
 */

#include "PayGrid.h"
#include "../gameMaths/mathElements/PayLines.h"
#include "../wins/Win.h"
#include "../wins/ScatterWin.h"
#include "../wins/LineWin.h"

/**
 * @brief A pay grid that evaluates line wins using `PayLines` definitions.
 */
class LineGrid :
    public PayGrid
{
protected:
    std::string szWinLineHeader = "Line Wins";
    const PayLines& payLines;

public:
    /**
     * @brief Construct grid with dimensions, pay table, and payline set.
     * @param reels Number of reels.
     * @param rows Number of rows.
     * @param payTable PayTable to use of win analysis.
     * @param payLines PayLines to use of win analysis.
     */
    LineGrid(const size_t reels, const size_t rows, const PayTable& payTable,
             const PayLines& payLines) : PayGrid(reels, rows, payTable),
                                         payLines(payLines)
    {
        winVector.reserve(payLines.lineCount);
        winVector.clear();
    }

    /**
     * @brief Construct from maths config using default pay table and paylines.
     * @param maths GameMaths from which to get numReels, numRows, payTable, and payLines.
     */
    explicit LineGrid(const GameMaths& maths)
        : LineGrid(maths.ints["numReels"],
                   maths.ints["numRows"],
                   maths.payTables["default"],
                   maths.payLines["default"])
    {
    }

    /************************************/
    // Win evaluation methods
    /************************************/

    /**
     * @brief Evaluate line and scatter wins.
     * @param betMultiplier Line bet multiplier.
     * @param combineType The strategy for combining multipliers - multiply or add
     * @return Total win.
     */
    unsigned int evaluateWins(unsigned int betMultiplier,
                              MultipliersCombineType combineType = MultipliersCombineType::multiply);

    /**
     * @brief Reset win buffer, evaluate line wins, apply line multipliers.
     * @param betMultiplier Line bet multiplier.
     * @param combineType The strategy for combining multipliers - multiply or add
     * @return Total win for all lines.
     */
    unsigned int evaluateLineWins(unsigned int betMultiplier,
                                  MultipliersCombineType combineType = MultipliersCombineType::multiply);

    /**
    * @brief Evaluate scatter wins and multipliers.
    * @param betMultiplier Line bet multiplier.
     * @param combineType The strategy for combining multipliers - multiply or add
     * @return Total win for scatters.
*/
    unsigned int evaluateScatterWins(unsigned int betMultiplier,
                                     MultipliersCombineType combineType = MultipliersCombineType::multiply);


    /**
     * @brief Identify paying symbols and fill `isWinVect` and `winIndexVect`.
     * @return Number of winning symbols (size of `winIndexVect`).
     */
    int setWinningSymbols();


    /**
     * @brief Count the number of active multipliers affecting a line win.
     * @param lineWin LineWin object to process.
     * @return Number of active multipliers affecting win.
     */
    int countWinMultipliers(const LineWin& lineWin) const;

    /**
     * @brief Count multipliers in the grid that contribute to any wins.
     * @return Number of active multipliers.
     */
    int countActiveWinMultipliers() const;

    /**
     * @brief Apply cell multipliers (from `multiplieLineGrid.hrVect`) to each win.
     * @param combineType How to combine multiple multipliers in a single win.
     * @return Total win after multipliers.
     */
    unsigned int applyWinMultipliers(MultipliersCombineType combineType);

    /**
     * @brief Get a payline definition by index.
     */
    const std::vector<size_t>* getPayLine(const int payLineNum) const
    {
        return &payLines.lineVect[payLineNum];
    }
};
