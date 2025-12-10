#pragma once
#include "Rng.h"
#include "RngData.h"
#include "RngKiss.h"

namespace MathLibrary
{
	struct PassStruct {
		uint32 passVal;
		uint32 range;
		std::string sz = "";
	};

	// added to implemnet kew-style code where 'failVal' and 'range' were given 
	struct FailStruct {
		uint32 failVal;
		uint32 range;
		std::string sz = "";
	};

	enum class RngOptions {
		stdC, kiss, data, count
	};

	class MathsRng {

	public:
		bool matchRandoms = false;
		Rng* lpRng;

		// default constructor - uses kiss rng
		// call class method setRng(RngOptions option) to apply another option from library 
		MathsRng() {
			setRng(RngOptions::kiss);
		}

		inline void setRng(RngOptions option) {
			if (option == RngOptions::kiss)
				lpRng = new RngKiss();
			else if (option == RngOptions::data)
				lpRng = &rnStream;
			else
				lpRng = new Rng();
		}

		inline uint32 setSeed(void) {
			lpRng->setSeed();
			return getSeed();
		}

		inline uint32 setSeed(int newSeed) {
			lpRng->setSeed(newSeed);
			return getSeed();
		}

		// return current seed vlaue
		inline uint32 getSeed(void) { return lpRng->getSeed(); }

		// Basic methods

		// returns integer in [0,range-1] for range < 2^32
		inline uint32 randInt(uint32 range, std::string sz = "")
		{
			return lpRng->randInt(range);
		}

		// returns integer in [low,high] for low >= 0, high < 2^32
		inline uint32 getInt(uint32 low, uint32 high)
		{
			uint32 range = high + 1 - low;
			return low + randInt(range);
		}

		// returns integer in [0,range-1] for range < 2^32
		inline uint32 getInt(uint32 range, std::string sz = "")
		{
			return lpRng->randInt(range);
		}

		// returns true if random value (0 to range-1) < passVal
		// e.g. getPass(32, 1000) has a 3.2% chance of returning true
		inline bool getPass(uint32 passVal, uint32 range, std::string sz = "")
		{
			uint32 pick = lpRng->randInt(range);
			return (pick < passVal);
		}

		//struct PassStruct {
		//	uint32 passVal;
		//	uint32 range;
		//	std::string sz = "";
		//};
		// returns true if random value (0 to data.range-1) < data.passVal
		// e.g. getPass({32, 1000}) has a 3.2% chance of returning true
		inline bool getPass(PassStruct data)
		{
			uint32 pick = lpRng->randInt(data.range);
			return (pick < data.passVal);
		}

		// function added for easy conversion from kew code 
		inline bool getPass(FailStruct data)
		{
			uint32 pick = lpRng->randInt(data.range);
			return (pick >= data.failVal);
		}
	};

	extern MathsRng mathRng;
}