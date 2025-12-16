#pragma once

#include <sstream>
#include <vector>
#include <fstream>

#include "Rng.h"


class RngKiss : public Rng {
public:
	/**********************************************/
	// Implement the virtual methods
	/**********************************************/

	// use the passed value as the seed
	void setSeed(uint32 newSeed) override {
		seed = kiss_i = newSeed;
		kiss_j = kiss_i * 1103515245L + 12345;
		kiss_k = kiss_j * 1103515245L + 12345;
		if (kiss_k == 0)
			kiss_k = 12345;
		kiss_j = ((kiss_i + kiss_k) % 2580) + 1;
	}

	// returns integer in [0,range-1] for range < 2^32
	uint32 randInt(uint32 range, std::string sz = "") override {
		uint32 value = uint32((uint64(algorithm()) * range) >> 32);
		return value;
	}

	/**********************************************/
	// Kiss RNG Specific Methods
	/**********************************************/

private:
	// generate the next number
	inline uint32 algorithm(void) {
		kiss_j = kiss_j ^ (kiss_j << 17);
		kiss_j = kiss_j ^ (kiss_j >> 15);
		kiss_k = (kiss_k ^ (kiss_k << 18)) & 0x7FFFFFFFL;
		kiss_k = kiss_k ^ (kiss_k >> 13);
		kiss_i = 69069L * kiss_i + 23606797L;
		return kiss_i + kiss_j + kiss_k;
	}

	uint32 kiss_i = 0, kiss_j = 0, kiss_k = 0;
};

