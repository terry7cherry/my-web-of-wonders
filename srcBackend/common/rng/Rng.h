#pragma once
#include <cstdlib>
#include <ctime>
#include <limits.h>

#include "../headers/types.h"


// The base class for Random Number Generators
// It uses the std C rand() method
// Games should use a better implementation e.g. Mersenne Twister or Kiss
// Other implementations should inherit from this class and replace all the virtual methods
class Rng
{
	friend class MathsRng;

protected:
	virtual ~Rng() = default;
	uint32 seed = 0;

	// Constructors
	Rng()
	{
		setSeed();
	}

	Rng(const uint32 newSeed)
	{
		Rng::setSeed(newSeed);
	}

	/****************************************************/
	// Virtual methods
	//		Derived classes should overwrite these
	/****************************************************/

	// use the passed variable as the seed
	virtual void setSeed(const uint32 newSeed)
	{
		std::srand(newSeed);
	}

	// returns integer in [0,range] for range < 2^32
	virtual uint32 randInt(uint32 range) = 0;

	// returns integer in [0,range] for range < 2^32
	virtual uint32 randInt(uint32 range, [[maybe_unused]] const std::string& sz) = 0;

	/****************************************************/
	// Standard Methods
	/****************************************************/

	// returns integer in [low,high] for low >= 0, high < 2^32
	uint32 randInt(const uint32 low, const uint32 high)
	{
		const uint32 range = high + 1 - low;
		return low + randInt(range);
	}

	// use system time to set the seed
	void setSeed()
	{
		setSeed(hash(time(nullptr), clock()));
	}

	// return current seed value
	uint32 getSeed(void) const { return seed; }

	static uint32 hash(time_t t, clock_t c)
	{
		// Get a uint32 from t and c
		// Better than uint32(x) in case x is floating point in [0,1]
		// Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

		static uint32 differ = 0; // guarantee time-based seeds will change

		uint32 h1 = 0;
		auto p = reinterpret_cast<unsigned char*>(&t);
		for (size_t i = 0; i < sizeof(t); ++i)
		{
			h1 *= UCHAR_MAX + 2U;
			h1 += p[i];
		}
		uint32 h2 = 0;
		p = reinterpret_cast<unsigned char*>(&c);
		for (size_t j = 0; j < sizeof(c); ++j)
		{
			h2 *= UCHAR_MAX + 2U;
			h2 += p[j];
		}
		return (h1 + differ++) ^ h2;
	}
};
