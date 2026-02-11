#pragma once
/**
 * @file Grid.h
 * @brief Core grid state and utilities for reel symbols and multipliers.
 */
#include <map>

#include "../gameMaths/mathElements/ReelSet.h"
#include "../spin/SpinResult.h"
#include "../gameMaths/GameMaths.h"


enum class symbolMatchTypes
{
    exact, greater, less, greaterEqual, lessEqual
};

enum class GridDataTypes
{
    symbol, multiplier
};

/**
 * @brief Mutable grid buffer used to apply spins, locks, and multipliers.
 */
class Grid
{
protected:
    std::map<Symbol, std::string> mTextMap;
    std::map<int, short> mColourMap;
    Console* lpConsole = nullptr;

public:
    /************************************/
    // generic grid details
    /************************************/

    size_t reelDataBytes; // how many byte in reelSymbolVect[reel] upto maxRows
    size_t multiplierDataBytes;
    size_t maxReels, maxRows, maxCells;
    std::vector<size_t> reelHeightVect;

    /************************************/
    // state
    /************************************/

    std::vector<bool> lockedReelVect;
    std::vector<bool> reelHasStickySymbolVect;
    std::vector<std::vector<bool>> stickySymbolVect;
    std::vector<std::vector<Symbol>> reelSymbolVect; // [reel][row]
    std::vector<std::vector<unsigned int>> multiplierVect; // [reel][row]

    /**
     * @brief Construct a grid of given dimensions.
     * @param reels Number of reels (columns).
     * @param rows Number of rows (visible window height).
     * @details reels initialised with SYMBOL_NONE (max uint8_t value).
     */
    Grid(size_t reels, size_t rows);

    /**
     * @brief Construct a grid sized according to `GameMaths` configuration.
     * @param maths Maths configuration providing reel/row dimensions as int values numReels and numRows.
     * @details reels initialised with SYMBOL_NONE (max uint8_t value).
     */
    explicit Grid(const GameMaths& maths);

    /************************************/
    // Initialise
    /************************************/

    /**
     * @brief Set all reel heights to `maxRows`.
     */
    void setReelHeights();

    /**
     * @brief Set reel heights individually.
     * @param heightVect Per-reel heights; size must equal `maxReels`.
     */
    void setReelHeights(const std::vector<size_t>& heightVect);

    /**
     * @brief Copy symbols (and multipliers) from another grid if dimensions match.
     * @param sourceGrid Source grid to copy from.
     */
    void copy(const Grid& sourceGrid);

    /**
     * @brief Copy occurrences of a symbol from another grid into this grid.
     * @param sourceGrid Source grid to read from.
     * @param symbol Symbol to copy where found.
     */
    void copySymbol(const Grid& sourceGrid, Symbol symbol);

    /************************************/
    // Pre Spin
    /************************************/

    /**
     * @brief Set all grid symbols to `SYMBOL_NONE` (max uint8_t value).
     */
    void reset();

    /**
     * @brief Set all grid multipliers to 1.
     */
    void resetMultipliers();


    /**************************************************/
    // Apply spin
    /**************************************************/

    /**
     * @brief Sets symbols according to the passed spinResult object.
     * @details Note you can pass ReelSet or ReelSetWeights (or other SpinResult constructor parameters), and implicit conversion will perform the spin automatically.
     * @param spinResult A reference to a SpinResult object that contains the details of the spin to be applied.
     * @param getLockedReels Reels marked as locked will remain unchanged unless getLockedReels is set to true.
     */
    void applySpin(const SpinResult& spinResult, bool getLockedReels = false);

    /**
     * @brief Set reel symbols using reel set and stop index.
     * @param reelNum Reel to populate.
     * @param reelSetPtr Source reel set.
     * @param reelStop Stop index within the reel band.
     */
    void setSymbolsForReel(size_t reelNum, const ReelSet* reelSetPtr, size_t reelStop);

    /**************************************************/
    // Spin lock
    /**************************************************/

    /**
     * @brief Lock the passed reels so their symbols will not change on spin.
     * @param noSpinVect Indices of reels to lock.
     */
    void setSpinLock(const std::vector<int>& noSpinVect);

    /**
     * @brief Set lock state for all reels.
     * @param spinStateVect Boolean per-reel lock state (true = locked).
     */
    void setSpinLock(const std::vector<bool>& spinStateVect);

    /**
     * @brief Unlock all reels.
     */
    void clearSpinLock();

    /**************************************************/
    // Spin lock
    /**************************************************/

    /**
     * @brief Resets all sticky symbol flags within the grid to false.
     */
    void clearSticky();

    /**
     * @brief Set sticky state of given cell.
     * @param reel
     * @param row
     * @param isSticky State to set.
     */
    void setSticky(size_t reel, size_t row, bool isSticky = true);

    /**
     * @brief Set sticky state of all cells containing specified symbol.
     * @param symbol Symbol to search for
     * @param isSticky State to set.
     */
    void setSticky(Symbol symbol, bool isSticky = true);

    /**
     * @brief Set sticky state of all cells containing any of the specified symbols.
     * @param symbols Symbols to search for.
     * @param isSticky State to set.
     */
    void setSticky(const std::vector<Symbol>& symbols, bool isSticky = true);

    /**
     * @brief Check sticky state of specified cell.
     * @param reel
     * @param row
     * @return Sticky state.
     */
    bool isSticky(size_t reel, size_t row);


    /**************************************************/
    // Convert index <-> grid positions
    /**************************************************/

    //  Reel     0   1   2   3   4
    //  Row 0    0   1   2   3   4 \
    //  Row 1    5   6   7   8   9  |< index
    //  Row 2   10  11  12  13  14 /

    /**
     * @brief Convert an int grid index to {reel,row} position.
     * @param index Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     * @return Position vector {reel,row}.
     */
    std::vector<size_t> getGridPosition(size_t index) const;

    /**
     * @brief Convert {reel,row} to linear grid index.
     * @param reel reel index.
     * @param row row index.
     * @return Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     */
    size_t getGridIndex(size_t reel, size_t row) const;

    /**
     * @brief Convert {reel,row} vector to linear grid index.
     * @param gridPos {reel, row} vector.
     * @return Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     */
    size_t getGridIndex(const std::vector<size_t>& gridPos) const
    {
        return getGridIndex(gridPos[0], gridPos[1]);
    }

    /**************************************************/
    // Access Symbols
    /**************************************************/

    /**
     * @brief Get symbol at [reel][row].
     * @param reel Reel index.
     * @param row Row index.
     * @return Symbol at position.
     */
    Symbol getReelSymbol(const size_t reel, const size_t row) const
    {
        return reelSymbolVect[reel][row];
    }

    /**
     * @brief Get symbol at grid position {reel,row}.
     * @param gridPos {reel, row} vector.
     * @return Symbol at position.
     */
    Symbol getSymbol(const std::vector<size_t>& gridPos) const
    {
        return reelSymbolVect[gridPos[0]][gridPos[1]];
    }

    /**
     * @brief Get symbol at linear index.
     * @param index Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     * @return Symbol at index.
     */
    Symbol getSymbol(const size_t index) const
    {
        return getSymbol(getGridPosition(index));
    }

    /**************************************************/
    // Set Symbols
    /**************************************************/

    /**
     * @brief Set symbol at [reel][row].
     * @param reel Reel index.
     * @param row Row index.
     * @param newSymbol Symbol to set.
     */
    void setSymbol(const size_t reel, const size_t row, const Symbol newSymbol)
    {
        reelSymbolVect[reel][row] = newSymbol;
    }

    /**
     * @brief Set symbol at grid position {reel,row}.
     * @param gridPos {reel, row} vector.
     * @param newSymbol Symbol to set.
     */
    inline void setSymbol(const std::vector<size_t>& gridPos, const Symbol newSymbol)
    {
        reelSymbolVect[gridPos[0]][gridPos[1]] = newSymbol;
    }

    /**
     * @brief Set symbol at linear index.
     * @param index Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     * @param newSymbol Symbol to set.
     */
    void setSymbol(const size_t index, const Symbol newSymbol)
    {
        setSymbol(getGridPosition(index), newSymbol);
    }

    /**
     * @brief Fill the top `count` cells in a reel with a symbol.
     * @param reel Index of reel on which to set symbols.
     * @param newSymbol Symbol to set.
     * @param count num Symbols to set (from top).
     */
    void setReelSymbols(size_t reel, Symbol newSymbol, int count);

    /**
     * @brief Fill an entire reel with a symbol.
     * @param reel Index of reel to fill.
     * @param newSymbol Symbol to fill reel with.
     */
    void setReelSymbols(size_t reel, Symbol newSymbol);

    /**
     * @brief Set symbol on specific rows of a reel.
     * @param reel Reel index.
     * @param newSymbol Symbol to set.
     * @param rowVect Either per-row flags (1 to set symbol) or list of row indices to set.
     * @return Linear indices that were set.
     */
    std::vector<size_t> setReelSymbols(size_t reel, Symbol newSymbol,
                                       const std::vector<size_t>& rowVect);


    /**************************************************/
    // Set Multipliers
    /**************************************************/

    /**
     * @brief Set multiplier at [reel][row].
     * @param reel Reel index.
     * @param row Row index.
     * @param val New multiplier value.
     */
    void setMultiplier(const size_t reel, const size_t row, const int val)
    {
        multiplierVect[reel][row] = val;
    }

    /**
     * @brief Set multiplier at grid position {reel,row}.
     * @param gridPos {reel, row} vector.
     * @param val New multiplier value.
     */
    void setMultiplier(const std::vector<size_t>& gridPos, const int val)
    {
        setMultiplier(gridPos[0], gridPos[1], val);
    }

    /**
     * @brief Set multiplier at linear index.
     * @param index Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     * @param val New multiplier value.
     */
    void setMultiplier(const size_t index, const int val)
    {
        setMultiplier(getGridPosition(index), val);
    }

    /**************************************************/
    // Get Multipliers
    /**************************************************/

    /**
     * @brief Get multiplier at [reel][row].
     * @param reel Reel index.
     * @param row Row index.
     * @return Multiplier.
     */
    unsigned int getMultiplier(const size_t reel, const size_t row) const
    {
        return multiplierVect[reel][row];
    }

    /**
     * @brief Get multiplier at grid position {reel,row}.
     * @param gridPos {reel, row} vector.
     * @return Multiplier.
     */
    unsigned int getMultiplier(const std::vector<size_t>& gridPos) const
    {
        return getMultiplier(gridPos[0], gridPos[1]);
    }

    /**
     * @brief Get multiplier at linear index.
     * @param index Linear index in row-major order. (Whole first row numbered first, then second row etc.)
     * @return Multiplier.
     */
    unsigned int getMultiplier(const int index) const
    {
        return getMultiplier(getGridPosition(index));
    }

    /************************************/
    // Analysis
    /************************************/

    /**
     * @brief Count matching symbols on a specific reel, with multiple match type options.
     * @param reel Reel index.
     * @param sym Symbol to match.
     * @param matchType Comparison mode (exact/greater/etc.).
     * @return Count of matching cells on the reel.
     */
    int getReelSymbolCount(size_t reel, Symbol sym,
                           symbolMatchTypes matchType = symbolMatchTypes::exact) const;

    /**
     * @brief Count matching symbols across the entire grid, with multiple match type options.
     * @param sym Symbol to match.
     * @param matchType Comparison mode (exact/greater/etc.).
     * @return Count of matching cells on the grid.
     */
    int getSymbolCount(Symbol sym, symbolMatchTypes matchType = symbolMatchTypes::exact) const;

    /**
     * @brief Find the first row containing a symbol on a reel.
     * @param reel Reel index.
     * @param sym Symbol to find.
     * @param matchType Comparison mode (exact/greater/etc.).
     * @return Row index if found; std::nullopt otherwise.
     */
    std::optional<size_t> getReelSymbolRow(size_t reel, Symbol sym,
                                           symbolMatchTypes matchType = symbolMatchTypes::exact) const;

    /**
     * @brief Check if reel contains matching symbol.
     * @param reel Reel index.
     * @param sym Symbol to find.
     * @param matchType Comparison mode (exact/greater/etc.).
     * @return Was symbol found?
     */
    bool reelContainsSymbol(size_t reel, Symbol sym,
                            symbolMatchTypes matchType = symbolMatchTypes::exact) const;

    static bool doSymbolsMatch(Symbol a, Symbol b, symbolMatchTypes matchType = symbolMatchTypes::exact);

    /************************************/
    // Grid processing - expand
    /************************************/

    /**
     * @brief Expand a symbol to fill an entire reel; optionally replace with another symbol.
     * @param reel Reel index.
     * @param symbol Symbol to expand.
     * @param newSym Different symbol with which to fill reel.
     * @return Number of occurrences before expansion.
     */
    int expandReelSymbol(size_t reel, Symbol symbol, Symbol newSym = SYMBOL_NONE);

    /**
     * @brief Expand a symbol to fill any reel where it appears.
     * @param symbol Symbol to expand.
     * @return Number of reels expanded.
     */
    int expandSymbol(Symbol symbol);

    /**
     * @brief Expand symbol if found only on selected reels.
     * @param symbol Symbol to expand.
     * @param reelCheckVect Indexes of reels to check/expand.
     * @return The number of reels expanded.
     */
    int expandSymbol(Symbol symbol, const std::vector<size_t>& reelCheckVect);

    /**
     * @brief If symbol found on reel, fill reel with different symbol.
     * @param symbol Symbol to search for.
     * @param newSym Symbol to fill with.
     * @return The number of reels expanded.
     */
    int expandSymbolToNew(Symbol symbol, Symbol newSym);

    /**
     * @brief Feel each of selected reels where symbol is found with different symbol.
     * @param symbol Symbol to search for.
     * @param newSym Symbol to fill with.
     * @param reelCheckVect Indexes of reels to check/expand.
     * @return The number of reels expanded.
     */
    int expandSymbolToNew(Symbol symbol, Symbol newSym, const std::vector<int>& reelCheckVect);

    /************************************/
    // Grid processing - replace
    /************************************/

    /**
     * @brief Replace all occurrences of a symbol on a reel.
     * @param reel Reel index.
     * @param symOld Symbol to find.
     * @param symNew Replacement symbol.
     * @return Number of replacements performed.
     */
    int replaceReelSymbol(size_t reel, Symbol symOld, Symbol symNew);

    /**
     * @brief Replace all occurrences of a symbol across the grid.
     * @param symOld Symbol to find.
     * @param symNew Replacement symbol.
     * @return Number of reels where replacements occurred.
     */
    int replaceSymbol(Symbol symOld, Symbol symNew);

    /**
     * @brief Replace symbol occurrences on selected reels.
     * @param symOld Symbol to find.
     * @param symNew Replacement symbol.
     * @param reelCheckVect Reels to process.
     * @return Number of reels where replacements occurred.
     */
    int replaceSymbol(Symbol symOld, Symbol symNew, const std::vector<int>& reelCheckVect);

    /**
     * @brief Replace given number of random positions (excluding blocked and already matching) with a new symbol.
     * @param newSymbol Replacement symbol.
     * @param count Number of replacements to attempt.
     * @param blockedIndexVect Linear indices to skip in row-major order. (Whole first row numbered first, then second row etc.)
     * @return Indices replaced in row-major order. (Whole first row numbered first, then second row etc.)
     */
    std::vector<int> replaceRandomSymbols(Symbol newSymbol, int count,
                                          const std::vector<int>& blockedIndexVect = {});

    /************************************/
    // Grid processing - replace not
    /************************************/

    /**
     * @brief On a reel, replace all symbols that are NOT a given symbol.
     * @param reel Reel index.
     * @param excludedSym Symbol to leave as is.
     * @param symNew Replacement symbol.
     * @return Number of replacements on the reel.
     */
    int replaceReelNotSymbol(int reel, Symbol excludedSym, Symbol symNew);

    /**
     * @brief Replace all symbols that are NOT a given symbol across the grid.
     * @param excludedSym Symbol to leave as is.
     * @param symNew Replacement symbol.
     * @return Number of reels where replacements occurred.
     */
    int replaceNotSymbol(Symbol excludedSym, Symbol symNew);
};
