#pragma once
#include <map>
#include <string>
#include <fstream>

#include "../headers/pch.h"
;

class MeterMap
{
public:
	std::string szName;
	std::map<int, uint64_t> intMap;
	const char* (*lpKeyOutputFunc)(int, uint64_t) = nullptr;

	// counts occurrences of unspecified integer values
	MeterMap(std::string szSetName);

	~MeterMap();

	// remove all data from intMap
	void reset(void);

	// add an item to the map if it does not already exist then
	// map[item] += value
	void add(int item, int value=1);

	// add a user defined method for file output
	void setKeyOutputFunction(const char* (*lpFunc)(int, uint64_t));

	// output using a user defined method or the default
	// optionally change output order hi to lo
	void fileOutput(FILE* fp, bool hiLo = false);

	// output using a user defined method or the default
	void fileOutput(std::ofstream &file);

	// Vertically output to screen using a user defined method or the default
	// optionally change output order hi to lo
	void consoleOutV(bool hiLo = false);

	// Horizontally output to screen using a user defined method or the default
	void consoleOutH(bool showPercent);

	std::string getOutputString(std::map<int, uint64_t>::iterator& it);

private:
	bool exists(int item);
};

