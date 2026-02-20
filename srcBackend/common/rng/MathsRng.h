#pragma once
#include "Rng.h"
#include "RngData.h"
#include "RngKiss.h"

struct PassStruct
{
	uint32 passVal;
	uint32 range;
	std::string sz = "";
};

// added to implement kew-style code where 'failVal' and 'range' were given
struct FailStruct
{
	uint32 failVal;
	uint32 range;
	std::string sz = "";
};

enum class RngOptions
{
	kiss, data, count
};

class MathsRng
{
public:
	bool matchRandoms = false;
	Rng* lpRng;

	// default constructor - uses kiss rng
	// call class method setRng(RngOptions option) to apply another option from library
	MathsRng()
	{
		setRng(RngOptions::kiss);
	}

	void setRng(const RngOptions option)
	{
		if (option == RngOptions::kiss)
			lpRng = new RngKiss();
		else if (option == RngOptions::data)
		{
			matchRandoms = true;
			lpRng = new RngData();
		}
		else throw std::invalid_argument("MathsRng.setRng() - invalid option.");
	}

	uint32 setSeed() const
	{
		lpRng->setSeed();
		return getSeed();
	}

	void setSeed(const uint32_t newSeed) const
	{
		lpRng->setSeed(newSeed);
	}

	// return current seed value
	uint32 getSeed() const { return lpRng->getSeed(); }

	// Basic methods

	// returns integer in [0,range-1] for range < 2^32
	uint32 randInt(const uint32 range, const std::string& sz = "") const
	{
		return lpRng->randInt(range, sz);
	}

	// returns integer in [low,high] for low >= 0, high < 2^32
	uint32 getInt(const uint32 low, const uint32 high) const
	{
		const uint32 range = high + 1 - low;
		return low + randInt(range);
	}

	// returns integer in [low,high] for low >= 0, high < 2^32
	uint32 randInt(const uint32 low, const uint32 high) const
	{
		const uint32 range = high + 1 - low;
		return low + randInt(range);
	}

	// returns integer in [0,range-1] for range < 2^32
	uint32 getInt(const uint32 range, const std::string& sz = "") const
	{
		return lpRng->randInt(range, sz);
	}

	// returns true if random value (0 to range-1) < passVal
	// e.g. getPass(32, 1000) has a 3.2% chance of returning true
	bool getPass(const uint32 passVal, const uint32 range,
	             [[maybe_unused]] std::string sz = "") const
	{
		const uint32 pick = lpRng->randInt(range);
		return (pick < passVal);
	}

	//struct PassStruct {
	//	uint32 passVal;
	//	uint32 range;
	//	std::string sz = "";
	//};
	// returns true if random value (0 to data.range-1) < data.passVal
	// e.g. getPass({32, 1000}) has a 3.2% chance of returning true
	bool getPass(const PassStruct& data) const
	{
		const uint32 pick = lpRng->randInt(data.range);
		return (pick < data.passVal);
	}

	// function added for easy conversion from kew code
	bool getPass(const FailStruct& data) const
	{
		const uint32 pick = lpRng->randInt(data.range);
		return (pick >= data.failVal);
	}

	void endSpinSequence(const int mathsWin) const
	{
		if (matchRandoms) dynamic_cast<RngData*>(lpRng)->endSpinSequence(mathsWin);
	}
};

extern thread_local MathsRng mathsRng;
