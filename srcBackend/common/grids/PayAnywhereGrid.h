#pragma once
/**
 * @file PayAnywhereGrid.h
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
class PayAnywhereGrid :
    public PayGrid
{
protected:
    std::string szWinLineHeader = "Line Wins";

public:
    /**
     * @brief Construct grid with dimensions, pay table, and payline set.
     * @param reels Number of reels.
     * @param rows Number of rows.
     * @param payTable PayTable to use of win analysis.
     * @param payLines PayLines to use of win analysis.
     */
    PayAnywhereGrid(const size_t reels, const size_t rows, const PayTable& payTable)
        : PayGrid(reels, rows, payTable)
    {
        winVector.reserve(payTable.scatterSymVect.size());
        winVector.clear();
    }

    /**
     * @brief Construct from maths config using default pay table and paylines.
     * @param maths GameMaths from which to get numReels, numRows, payTable, and payLines.
     */
    explicit PayAnywhereGrid(const GameMaths& maths)
        : PayAnywhereGrid(maths.ints["numReels"],
                          maths.ints["numRows"],
                          maths.payTables["default"])
    {
    }

    /************************************/
    // Win evaluation methods
    /************************************/

    /**
     * @brief Evaluate all wins.
     * @param betMultiplier Line bet multiplier.
     * @param combineType The strategy for combining multipliers - multiply or add
     * @return Total win for all lines.
     */
    unsigned int evaluateWins(unsigned int betMultiplier,
                              MultipliersCombineType combineType = MultipliersCombineType::multiply);


    /**
     * @brief Identify paying symbols and fill `isWinVect` and `winIndexVect`.
     * @return Number of winning symbols (size of `winIndexVect`).
     */
    int setWinningSymbols();


    /**
     * @brief Count the number of active multipliers affecting a line win.
     * @param scatterWin ScatterWin object to process.
     * @return Number of active multipliers affecting win.
     */
    int countWinMultipliers(const ScatterWin& scatterWin) const;

    /**
     * @brief Count multipliers in the grid that contribute to any wins.
     * @return Number of active multipliers.
     */
    int countActiveWinMultipliers() const;

    /**
     * @brief Apply cell multipliers (from `multipliePayAnywhereGrid.hrVect`) to each win.
     * @param combineType How to combine multiple multipliers in a single win.
     * @return Total win after multipliers.
     */
    unsigned int applyWinMultipliers(MultipliersCombineType combineType);
};
