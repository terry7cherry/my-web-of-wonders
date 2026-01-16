#pragma once
#include "ReelSet.h"
#include "../../headers/pch.h"

// Create a vector of this structure to define WeightTable
// The First 2 items are required:
//		int result, int weight
// 3rd item is needed for advanced WeightTable and for PopTable
struct WeightData
{
    int result; // required: result if this item is picked
    int weight; // required: weight given to this result when calculating table chances
    int limit; // optional: maximum number of this item that can be picked

    int index; // items index in the table vector, added during table creation
    int range; // calculated during table creation
};


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

    // default constructor
    WeightTable()
    {
    }

    // sourceVector items must contain fields: result, weight
    WeightTable(std::vector<WeightData> vector, std::string sz = "");

    // sourceVector contains weights
    // The result fields of the WeightTable are the vector index of each weight
    WeightTable(std::vector<int> vector, std::string sz = "");

    ~WeightTable();

    // sourceVector items should contain fields: result, weight
    // create a local weightVector from sourceVector and add index fields
    // Update the pickList
    void setWeightVector(std::vector<WeightData> sourceVector);

    // sourceVector contains weights
    // create a local weightVector and add fields: index, result (= index)
    // Update the pickList
    void setWeightVector(std::vector<int> sourceVector);

    // Re-create pickList from the original weightVector
    void reset(void);

    // calculate the pick data for the current pickList
    void updateList(void);

    // get a result from the current pickList with optional debug output
    int pick(bool debugOutput = false) const;

    // force result with passed index in the current pickList
    int forcePickIndex(int index);

    // force result with passed result from the current pickList
    int forcePickResult(int result);

    // get size of current pick list
    inline int getActiveCount(void)
    {
        return (int)pickList.size();
    }

    inline int getResultAtIndex(int index)
    {
        return weightVector[index].result;
    }


    // Remove the passed item from the pickList and update the pickList if required
    void removeItem(auto it, bool blnUpdateList = true);

    // Remove item with the passed result from the pickList and update the pickList data
    // return (item with passed result is found ? true : false)
    bool removeResult(int result, bool blnUpdateList = true);

    void test(uint64_t samples, bool blnConsole);

    void cout(void) const;
};

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

    double pick() const { return doubleVect[WeightTable::pick()]; }
};

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

    bool pick() const { return boolVect[WeightTable::pick()]; }
};

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

    const ReelSet& pick() const { return reelSetVect[WeightTable::pick()]; }
};

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

    std::string pick() const { return stringVect[WeightTable::pick()]; }
};

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

    Symbol pick() const { return symbolVect[WeightTable::pick()]; }
};
