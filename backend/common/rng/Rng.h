#pragma once
#include <cstdlib>
#include <ctime>
#include <limits.h>

#include "../headers/types.h"

namespace MathLibrary
{

	// The base class for Random Number Generators
	// It uses the std C rand() method
	// Games should use a better implementation e.g. Mersenne Twister or Kiss
	// Other implementations should inherit from this class and replace all the virtual methods
	class Rng
	{
	public:
		uint32 seed = 0;

		// Constructors
		Rng() {
			setSeed();
		}

		Rng(uint32 newSeed) {
			setSeed(newSeed);
		}

		/****************************************************/
		// Virtual methods
		//		Derived classes should over-write these
		/****************************************************/

		// use the passed variable as the seed
		virtual void setSeed(uint32 newSeed)
		{
			std::srand(newSeed);
		}

		// returns integer in [0,range] for range < 2^32
		virtual uint32 randInt(uint32 range, std::string sz = "")
		{
			return std::rand() & range;
		}

		/****************************************************/
		// Standard Methods
		/****************************************************/

		// returns integer in [low,high] for low >= 0, high < 2^32
		inline uint32 randInt(uint32 low, uint32 high)
		{
			uint32 range = high + 1 - low;
			return low + randInt(range);
		}

		// use system time to set the seed
		inline void setSeed(void)
		{
			setSeed(hash(time(NULL), clock()));
		}

		// return current seed vlaue
		inline uint32 getSeed(void) { return seed; }

	protected:
		inline uint32 hash(time_t t, clock_t c)
		{
			// Get a uint32 from t and c
			// Better than uint32(x) in case x is floating point in [0,1]
			// Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

			static uint32 differ = 0;  // guarantee time-based seeds will change

			uint32 h1 = 0;
			unsigned char* p = (unsigned char*)&t;
			for (size_t i = 0; i < sizeof(t); ++i)
			{
				h1 *= UCHAR_MAX + 2U;
				h1 += p[i];
			}
			uint32 h2 = 0;
			p = (unsigned char*)&c;
			for (size_t j = 0; j < sizeof(c); ++j)
			{
				h2 *= UCHAR_MAX + 2U;
				h2 += p[j];
			}
			return (h1 + differ++) ^ h2;
		}
	};
}
