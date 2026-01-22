#pragma once
/**
 * @file WeightTables.h
 * @brief Weighted random selection tables for ints, doubles, bools, strings, symbols, and reel sets.
 */
#include "ReelSet.h"
#include "../../headers/pch.h"

// Create a vector of this structure to define WeightTable
// The First 2 items are required:
//		int result, int weight
// 3rd item is needed for advanced WeightTable and for PopTable
/**
 * @brief One entry in a weight table.
 */
struct WeightData
{
    int result; // required: result if this item is picked
    int weight; // required: weight given to this result when calculating table chances
    int limit; // optional: maximum number of this item that can be picked

    int index; // items index in the table vector, added during table creation
    int range; // calculated during table creation
};


/**
 * @brief Base weighted table supporting limits and removable items.
 */
class WeightTable
{
public:
    // Variables
    std::vector<WeightData> weightVector;
    std::list<WeightData> pickList;
    int range = -1;
    std::string szPicker = "";
    int minVal = -1;
    int maxVal = -1;

    /**
     * @brief Default constructor.
     */
    WeightTable()
    {
    }

    /**
     * @brief Construct from vector of WeightData.
     */
    WeightTable(std::vector<WeightData> vector, std::string sz = "");

    /**
     * @brief Construct from vector of int weights; result becomes the index.
     */
    WeightTable(std::vector<int> vector, std::string sz = "");

    ~WeightTable();

    /**
     * @brief Replace table entries with WeightData and rebuild pick list.
     */
    void setWeightVector(std::vector<WeightData> sourceVector);

    /**
     * @brief Replace table entries from weights; result becomes index; rebuild pick list.
     */
    void setWeightVector(std::vector<int> sourceVector);

    /**
     * @brief Reset pick list from original weight vector (undo removals/limits).
     */
    void reset(void);

    /**
     * @brief Recalculate cumulative ranges for current pick list.
     */
    void updateList(void);

    /**
     * @brief Randomly pick a result according to weights.
     * @param debugOutput If true, write details to console/file as configured.
     * @return Picked result value.
     */
    int pick(bool debugOutput = false) const;

    /**
     * @brief Force-pick by active-item index in the current pick list.
     * @return Result value of the chosen item.
     */
    int forcePickIndex(int index);

    /**
     * @brief Force-pick an item with the specified result value from the current pick list.
     * @return Result value if successful; implementation-defined if not found.
     */
    int forcePickResult(int result);

    /**
     * @brief Get number of active items in the current pick list.
     */
    inline int getActiveCount(void)
    {
        return (int)pickList.size();
    }

    /**
     * @brief Get the result value for the original item at `index`.
     */
    inline int getResultAtIndex(int index)
    {
        return weightVector[index].result;
    }


    /**
     * @brief Remove a specific iterator from the pick list and optionally update ranges.
     */
    void removeItem(auto it, bool blnUpdateList = true);

    /**
     * @brief Remove an item with a given result from the pick list.
     * @return true if removed; false if not found.
     */
    bool removeResult(int result, bool blnUpdateList = true);

    /**
     * @brief Monte Carlo test and optional console output of frequencies.
     */
    void test(uint64_t samples, bool blnConsole);

    /**
     * @brief Dump current table to stdout.
     */
    void cout(void) const;
};

/**
 * @brief Weight table mapping to double results.
 */
class DoubleWeightTable : public WeightTable
{
public:
    std::vector<double> doubleVect;

    DoubleWeightTable(const std::vector<WeightData>& weightVector, const std::vector<double>& doubleVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          doubleVect(doubleVect)
    {
    }

    DoubleWeightTable(const std::vector<int>& weightVector, const std::vector<double>& doubleVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          doubleVect(doubleVect)
    {
    }

    /**
     * @brief Pick a double value using weights.
     */
    double pick() const { return doubleVect[WeightTable::pick()]; }
};

/**
 * @brief Weight table mapping to boolean results.
 */
class BoolWeightTable : public WeightTable
{
public:
    std::vector<bool> boolVect;

    BoolWeightTable(const std::vector<WeightData>& weightVector, const std::vector<bool>& boolVect,
                    const std::string& sz = "")
        : WeightTable(weightVector, sz),
          boolVect(boolVect)
    {
    }

    BoolWeightTable(const std::vector<int>& weightVector, const std::vector<bool>& boolVect,
                    const std::string& sz = "")
        : WeightTable(weightVector, sz),
          boolVect(boolVect)
    {
    }

    /**
     * @brief Pick a bool value using weights.
     */
    bool pick() const { return boolVect[WeightTable::pick()]; }
};

/**
 * @brief Weight table mapping to `ReelSet` results.
 */
class ReelSetWeights : public WeightTable
{
public:
    std::vector<ReelSet> reelSetVect;

    ReelSetWeights(const std::vector<WeightData>& weightVector,
                   const std::vector<ReelSet>& reelSetVect,
                   const std::string& sz = "")
        : WeightTable(weightVector, sz),
          reelSetVect(reelSetVect)
    {
    }

    ReelSetWeights(const std::vector<int>& weightVector, const std::vector<ReelSet>& reelSetVect,
                   const std::string& sz = "")
        : WeightTable(weightVector, sz),
          reelSetVect(reelSetVect)
    {
    }

    /**
     * @brief Pick a `ReelSet` using weights.
     */
    const ReelSet& pick() const { return reelSetVect[WeightTable::pick()]; }
};

/**
 * @brief Weight table mapping to std::string results.
 */
class StringWeightTable : public WeightTable
{
public:
    std::vector<std::string> stringVect;

    StringWeightTable(const std::vector<WeightData>& weightVector, const std::vector<std::string>& stringVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          stringVect(stringVect)
    {
    }

    StringWeightTable(const std::vector<int>& weightVector, const std::vector<std::string>& stringVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          stringVect(stringVect)
    {
    }

    /**
     * @brief Pick a string using weights.
     */
    std::string pick() const { return stringVect[WeightTable::pick()]; }
};

/**
 * @brief Weight table mapping to `Symbol` results.
 */
class SymbolWeightTable : public WeightTable
{
public:
    std::vector<Symbol> symbolVect;

    SymbolWeightTable(const std::vector<WeightData>& weightVector, const std::vector<Symbol>& symbolVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          symbolVect(symbolVect)
    {
    }

    SymbolWeightTable(const std::vector<int>& weightVector, const std::vector<Symbol>& symbolVect,
                      const std::string& sz = "")
        : WeightTable(weightVector, sz),
          symbolVect(symbolVect)
    {
    }

    /**
     * @brief Pick a symbol using weights.
     */
    Symbol pick() const { return symbolVect[WeightTable::pick()]; }
};
