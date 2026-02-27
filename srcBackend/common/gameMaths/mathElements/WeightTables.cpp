#include "../../headers/pch.h"
#include "WeightTables.h"
#include "../../rng/MathsRng.h"

using PICKLIST_IT = std::vector<WeightData>::iterator;

// sourceVector items should contain fields: { result, weight }
WeightTable::WeightTable(const std::vector<WeightData>& sourceVector, const std::string& sz)
{
    szPicker = sz;
    setWeightVector(sourceVector);
}

// sourceVector contains weights
// The result fields of the WeightTable are the vector index of each weight
WeightTable::WeightTable(const std::vector<int>& sourceVector, const std::string& sz)
{
    szPicker = sz;
    setWeightVector(sourceVector);
}

WeightTable::~WeightTable()
{
    pickList.clear();
}

// sourceVector items should contain fields: result, weight
// create a local weightVector from sourceVector and add index fields
// Update the pickList
void WeightTable::setWeightVector(const std::vector<WeightData>& sourceVector)
{
    weightVector = sourceVector;

    int i = 0;
    for (auto it = weightVector.begin(); it != weightVector.end(); ++it)
        it->index = i++; // set index values in the local weight vector

    reset();
}

// sourceVector contains weights
// create a local weightVector and add fields: index, result (= index)
// Update the pickList
void WeightTable::setWeightVector(std::vector<int> sourceVector)
{
    weightVector.clear();

    int i = 0;
    for (auto it = sourceVector.begin(); it != sourceVector.end(); ++it, i++)
    {
        weightVector.push_back({i, *it}); // set values in the local weight vector
        weightVector[i].index = i;
    }
    reset();
}

// Re-create pickList from the original weightVector
void WeightTable::reset()
{
    pickList.clear();
    for (size_t i = 0; i < weightVector.size(); i++)
        pickList.push_back(weightVector[i]);

    updateList();
}

// calculate the pick data for the current pickList
void WeightTable::updateList()
{
    range = 0;
    minVal = INT32_MAX;
    maxVal = INT32_MIN;

    for (auto it = pickList.begin(); it != pickList.end(); ++it)
    {
        if (it->result > maxVal)
            maxVal = it->result;
        if (it->result < minVal)
            minVal = it->result;

        range += it->weight;
        it->range = range;
    }
}


// get a result from the current pickList
int WeightTable::pick() const
{
    std::list<WeightData>::const_iterator it;

    if (pickList.empty())
    {
        printf("\r\nWeightTable.pick(): No items available to pick from\r\n");
        throw std::runtime_error("WeightTable::pickWeightDataObject: pickList is empty");
    }
    else if (pickList.size() == 1) // don't use random if only one pick left (for GDM matching)
    {
        it = pickList.begin();
    }
    else
    {
        const int32 rand = mathsRng.randInt(range,
                                            std::string("weight table - ") + szPicker.c_str());
        for (it = pickList.begin(); it != pickList.end(); ++it)
        {
            if (rand < it->range)
                break;
        }
        if (it == pickList.end())
        {
            reportError("WeightTable::pick", "RNG value is out of required range");
            throw std::runtime_error(
                "WeightTable::pickWeightDataObject: RNG value out of required range");
        }
    }

    return it->result;
}

int WeightTable::pickWithLimits()
{
    std::list<WeightData>::iterator it;

    if (pickList.empty())
    {
        printf(
            "\r\nWeightTable.pick(): No items available to pick from. It may be all that limits have been used up.\r\n");
        throw std::runtime_error("WeightTable::pickWeightDataObject: pickList is empty");
    }
    if (pickList.size() == 1) // don't use random if only one pick left (for GDM matching)
    {
        it = pickList.begin();
    }
    else
    {
        const int32 rand = mathsRng.randInt(range,
                                            std::string("weight table - ") + szPicker.c_str());
        for (it = pickList.begin(); it != pickList.end(); ++it)
        {
            if (rand < it->range)
                break;
        }
        if (it == pickList.end())
        {
            reportError("WeightTable::pick", "RNG value is out of required range");
            throw std::runtime_error(
                "WeightTable::pickWeightDataObject: RNG value out of required range");
        }
    }

    const int result = it->result;
    if (it->limit > 0)
    {
        it->limit--;
        if (it->limit == 0) removeItem(it, true);
    }
    return result;
}

// force result with passed index in the current pickList
int WeightTable::forcePickIndex(const int index)
{
    for (auto it = pickList.begin(); it == pickList.end() && it != pickList.end(); ++it)
    {
        if (it->index == index)
            return it->result;
    }


    return pickList.end()->result;
}

// force result with passed result from the current pickList
int WeightTable::forcePickResult(const int result)
{
    for (auto it = pickList.begin(); it == pickList.end() && it != pickList.end(); ++it)
    {
        if (it->result == result)
            return it->result;
    }

    return pickList.end()->result;
}

// Remove the passed item from the pickList and update the pickList if required
void WeightTable::removeItem(auto it, bool blnUpdateList)
{
    pickList.erase(it);
    if (blnUpdateList)
        updateList();
}

// Remove item with the passed result from the pickList and update the pickList data
// return (item with passed result is found ? true : false)
bool WeightTable::removeResult(int result, bool blnUpdateList)
{
    for (auto it = pickList.begin(); it != pickList.end(); ++it)
    {
        if (it->result == result)
        {
            removeItem(it, blnUpdateList);
            return true;
        }
    }
    return false;
}

void WeightTable::test(const uint64_t samples, const bool blnConsole)
{
    std::vector<long long> results;
    results.assign(weightVector.size(), 0);
    for (uint64_t i = 0; i < samples; i++)
        results[pick()]++; // test 'samples' outcomes

    if (blnConsole)
    {
        printf("%8s\t%8s\t%8s\t%8s\r\n", "Item", "Weight", "Count", "P(Item)");
        for (auto it = weightVector.begin(); it != weightVector.end(); ++it)
        {
            const long long pickCount = results[it->index];
            const float chance = static_cast<float>(100) * pickCount / samples;
            printf("%8d\t%8d\t%8lld\t%6.3f\r\n", it->result, it->weight, pickCount, chance);
        }
    }
}

void WeightTable::cout(void) const
{
    std::cout << "\r\nWeightTable: " << szPicker << "\r\n";
    std::cout << "\t" << "Value" << "\t" << "Weight\r\n";
    for (auto entry : weightVector)
        std::cout << "\t" << entry.result << "\t" << entry.weight << "\r\n";
    std::cout << "\r\n";
}

Json::Value WeightTable::toJson() const
{
    Json::Value json(Json::arrayValue);
    for (const auto& entry : weightVector)
    {
        Json::Value jsonEntry(Json::objectValue);
        jsonEntry["outcome"] = entry.result;
        jsonEntry["weight"] = entry.weight;
        json.append(jsonEntry);
    }
    return json;
}

Json::Value ReelSetWeights::toJson(const std::vector<std::string>& symbolNames) const
{
    Json::Value json(Json::arrayValue);
    for (size_t i = 0; i < weightVector.size(); i++)
    {
        Json::Value jsonEntry(Json::objectValue);
        jsonEntry["outcome"] = reelSetVect[i].toJson(symbolNames);
        jsonEntry["weight"] = weightVector[i].weight;
        json.append(jsonEntry);
    }
    return json;
}

Json::Value DoubleWeightTable::toJson() const
{
    Json::Value json(Json::arrayValue);
    for (size_t i = 0; i < weightVector.size(); i++)
    {
        Json::Value jsonEntry(Json::objectValue);
        jsonEntry["outcome"] = doubleVect[i];
        jsonEntry["weight"] = weightVector[i].weight;
        json.append(jsonEntry);
    }
    return json;
}

Json::Value BoolWeightTable::toJson() const
{
    Json::Value json(Json::arrayValue);
    for (size_t i = 0; i < weightVector.size(); i++)
    {
        Json::Value jsonEntry(Json::objectValue);
        jsonEntry["outcome"] = Json::Value(boolVect[i]);
        jsonEntry["weight"] = weightVector[i].weight;
        json.append(jsonEntry);
    }
    return json;
}

Json::Value StringWeightTable::toJson() const
{
    Json::Value json(Json::arrayValue);
    for (size_t i = 0; i < weightVector.size(); i++)
    {
        Json::Value jsonEntry(Json::objectValue);
        jsonEntry["outcome"] = stringVect[i];
        jsonEntry["weight"] = weightVector[i].weight;
        json.append(jsonEntry);
    }
    return json;
}

Json::Value SymbolWeightTable::toJson(const std::vector<std::string>& symbolNames) const
{
    Json::Value json(Json::arrayValue);
    for (size_t i = 0; i < weightVector.size(); i++)
    {
        Json::Value jsonEntry(Json::objectValue);
        jsonEntry["outcome"] = symbolNames.at(symbolVect[i]);
        jsonEntry["weight"] = weightVector[i].weight;
        json.append(jsonEntry);
    }
    return json;
}
