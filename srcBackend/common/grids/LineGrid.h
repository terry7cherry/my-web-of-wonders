#pragma once
/**
 * @file LineGrid.h
 * @brief Grid that evaluates traditional payline wins and prints results.
 */

#include "PayGrid.h"
#include "../gameMaths/mathElements/PayLines.h"
#include "Win.h"

#define MAX_LINE_TEXT_LEN 45

/**
 * @brief A pay grid that evaluates line wins using `PayLines` definitions.
 */
class LineGrid :
    public PayGrid
{
protected:
    std::string szWinLineHeader = "Line Wins";
    const PayLines* lpPayLines = nullptr;

public:
    std::vector<Win> winVector;
    std::string szBlankWin;

    /**
     * @brief Construct grid with dimensions and pay table; defaults to left-to-right evaluation.
     */
    LineGrid(const size_t reels, const size_t rows, const PayTable& payTable) : PayGrid(reels, rows, payTable)
    {
        szBlankWin = std::string(MAX_LINE_TEXT_LEN, ' ');
        winVector.reserve(100);
    }

    /**
     * @brief Construct grid with dimensions, pay table, and payline set.
     * @param reels Number of reels.
     * @param rows Number of rows.
     * @param payTable PayTable to use of win analysis.
     * @param payLines PayLines to use of win analysis.
     */
    LineGrid(const size_t reels, const size_t rows, const PayTable& payTable,
             const PayLines& payLines) : LineGrid(reels, rows, payTable)
    {
        setPayLines(payLines);
    }

    /**
     * @brief Construct from maths config using default pay table and paylines.
     * @param maths GameMaths from which to get numReels, numRows, payTable and payLines.
     */
    explicit LineGrid(const GameMaths& maths)
        : LineGrid(maths.ints["numReels"],
                   maths.ints["numRows"],
                   maths.payTables["default"],
                   maths.payLines["default"])
    {
    }

    /**
     * @brief Set the active payline definitions and resize internal buffers.
     */
    void setPayLines(const PayLines& payLines)
    {
        lpPayLines = &payLines;
        winVector.resize(lpPayLines->lineCount);
    }

    /************************************/
    // Win evaluation methods
    /************************************/

    /**
     * @brief Reset win buffer, evaluate line wins, and apply line multipliers.
     * @param betMultiplier Line bet multiplier.
     * @param doMeters If true, update meters.
     * @param useDebugLine If true, enable extra debug on a specific line.
     * @return Total win for all lines.
     */
    unsigned int evaluateWins(unsigned int betMultiplier, [[maybe_unused]] bool doMeters,
                              [[maybe_unused]] bool useDebugLine = false);


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
    int countWinMultipliers(const Win& lineWin) const;

    /**
     * @brief Count multipliers in the grid that contribute to any wins.
     * @return Number of active multipliers.
     */
    int countActiveWinMultipliers();

    /**
     * @brief Apply a gridMultiplier (or other multiplier) to all wins.
     * @param combineType How to combine with existing win-level multipliers.
     * @param specialMultiplier Optional override multiplier. (Omit to use gridMultiplier)
     * @return New total win after application.
     */
    unsigned int applyGridMultiplier(MultipliersCombineType combineType,
                                     std::optional<int> specialMultiplier = std::nullopt);


    /**
     * @brief Apply cell multipliers (from `multiplierVect`) to each win.
     * @param combineType How to combine multiple multipliers in a single win.
     * @return Total win after multipliers.
     */
    unsigned int applyWinMultipliers(MultipliersCombineType combineType);

    /****************************************/
    // Display methods
    /****************************************/

    /**
     * @brief Clear display area and render wins to console.
     * @param startRow Start console row to begin printing.
     * @param col Start console column.
     * @param showTitle If true, show a header.
     * @param showDirection If true, include direction arrows.
     * @return Number of rows printed.
     */
    int showWins(int startRow, int col, bool showTitle = true, bool showDirection = false) const;


    /**
     * @brief Get formatted winning-line text rows.
     * @param symbolMap Map from symbol to display string.
     * @param showDirection If true, include direction arrows.
     * @return Vector of text lines.
     */
    std::vector<std::string> getWinTextVect(const std::map<Symbol, std::string>& symbolMap,
                                            bool showDirection = false) const;


    /**
     * @brief Get a payline definition by index.
     */
    const std::vector<size_t>* getPayLine(const int payLineNum) const
    {
        return &lpPayLines->lineVect[payLineNum];
    }
};
