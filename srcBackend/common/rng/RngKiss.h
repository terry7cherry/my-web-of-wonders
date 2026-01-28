#pragma once


#include "Rng.h"


class RngKiss : public Rng
{
public:
	/**********************************************/
	// Implement the virtual methods
	/**********************************************/

	// use the passed value as the seed
	void setSeed(uint32 newSeed) override;

	// returns integer in [0,range-1] for range < 2^32
	uint32 randInt(uint32 range, [[maybe_unused]] const std::string& sz) override;

	uint32 randInt(uint32 range) override;

	/**********************************************/
	// Kiss RNG Specific Methods
	/**********************************************/

private:
	// generate the next number
	uint32 algorithm();

	uint32 kiss_i = 0, kiss_j = 0, kiss_k = 0;
};

