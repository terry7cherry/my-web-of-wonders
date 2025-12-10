#include "../headers/pch.h"
#include "WeightTable.h"
#include "../rng/MathsRng.h"

namespace MathLibrary
{
	//// sourceVector items should contain fields: result, weight
	//WeightTable::WeightTable(std::vector <weightData> sourceVector, std::string sz)
	//{
	//	szPicker = sz;
	//	itLastPick = pickList.end();
	//	setWeightVector(sourceVector);
	//}

	// sourceVector items should contain fields: { result, weight }
	WeightTable::WeightTable(std::vector <weightData> sourceVector, std::string sz)
	{
		szPicker = sz;
		itLastPick = pickList.end();
		setWeightVector(sourceVector);
	}

	// sourceVector contains weights
	// The result fields of the WeightTable are the vector index of each weight 
	WeightTable::WeightTable(std::vector <int> sourceVector, std::string sz)
	{
		szPicker = sz;
		itLastPick = pickList.end();
		setWeightVector(sourceVector);
	}

	WeightTable::~WeightTable()
	{
		pickList.clear();
	}

	// sourceVector items should contain fields: result, weight
	// create a local weightVector from sourceVector and add index fields
	// Update the pickList 
	void WeightTable::setWeightVector(std::vector <weightData> sourceVector)
	{
		weightVector = sourceVector;

		int i = 0;
		for (auto it = weightVector.begin(); it != weightVector.end(); ++it)
			it->index = i++;				// set index values in the local weight vector

		reset();
	}

	// sourceVector contains weights
	// create a local weightVector and add fields: index, result (= index)
	// Update the pickList
	void WeightTable::setWeightVector(std::vector <int> sourceVector)
	{
		weightVector.clear();

		int i = 0;
		for (auto it = sourceVector.begin(); it != sourceVector.end(); ++it, i++)
		{
			weightVector.push_back({ i, *it });	// set values in the local weight vector
			weightVector[i].index = i;
		}
		reset();
	}

	// Re-create pickList from the original weightVector
	void WeightTable::reset(void)
	{
		pickList.clear();
		for (size_t i=0; i< weightVector.size(); i++)
			pickList.push_back(weightVector[i]);

		updateList();
	}

	// calculate the pick data for the current pickList
	void WeightTable::updateList(void)
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
	int WeightTable::pick(bool debugOutput)
	{
		if (pickList.size() == 0)
			printf("\r\nWeightTable.pick(): No items available to pick from\r\n");
		else if (pickList.size() == 1) //don't use random if only one pick left (for GDM matching)
		{
			itLastPick = pickList.begin();
		}
		else
		{
			int32 rand = mathRng.getInt(range, szPicker.c_str());
			for (itLastPick = pickList.begin(); itLastPick != pickList.end(); ++itLastPick)
			{
				if (rand < itLastPick->range)
					break;
			}
			if (itLastPick != pickList.end())
				weightVector[itLastPick->index].pickCount++;
			else
			{
				reportError("WeightTable::pick", "RNG value is out of required range");
				return -1;
			}
		}

		//if (debugOutput && lpRng->fpSequence)
		//	fprintf(lpRng->fpSequence, "%s:value = %d\n", szPicker.c_str(), itLastPick->result);

		return itLastPick->result;
	}

	// force result with passed index in the current pickList 
	int WeightTable::forcePickIndex(int index)
	{
#if _DEBUG
		if (index >= (int)pickList.size())
			reportError("WeightTable::forcePickIndex", "index >= pickList.size()");
#endif

		itLastPick = pickList.end();
		for (auto it = pickList.begin(); itLastPick == pickList.end() && it != pickList.end(); ++it)
		{
			if (it->index == index)
				itLastPick = it;
		}

#if _DEBUG
		if (itLastPick == pickList.end())
			reportError("WeightTable::forcePickIndex", "index not found in pickList");
#endif

		return itLastPick->result;
	}

	// force result with passed result from the current pickList 
	int WeightTable::forcePickResult(int result)
	{
		itLastPick = pickList.end();
		for (auto it = pickList.begin(); itLastPick == pickList.end() && it != pickList.end(); ++it)
		{
			if (it->result == result)
				itLastPick = it;
		}

#if _DEBUG
		if (itLastPick == pickList.end())
			reportError("WeightTable::forcePickResult", "result not found pickList");
#endif
		return itLastPick->result;
	}

	// Remove the passed item from the pickList and update the pickList if required
	void WeightTable::removeItem(PICKLIST_IT it, bool blnUpdateList)
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




	void WeightTable::test(uint64_t samples, bool blnConsole)
	{
		for (uint64_t i = 0; i < samples; i++)
			pick();				// test 'samples' outcomes

		if (blnConsole)
		{
			printf("%8s\t%8s\t%8s\t%8s\r\n", "Item", "Weight", "Count", "P(Item)");
			for (auto it = weightVector.begin(); it != weightVector.end(); ++it)
			{
				float chance = (float)100 * it->pickCount / samples;
				printf("%8d\t%8d\t%8lld\t%6.3f\r\n", it->result, it->weight, it->pickCount, chance);
			}
		}
	}

	void WeightTable::cout(void)
	{
		std::cout << "\r\nWeightTable: " << szPicker << "\r\n";
		std::cout << "\t" << "Value" << "\t" << "Weight\r\n";
		for (auto entry : weightVector)
			std::cout << "\t" << entry.result << "\t" << entry.weight << "\r\n";
		std::cout << "\r\n";
	}
}