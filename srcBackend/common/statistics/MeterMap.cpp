// cpp-dev-library
#include "../headers/pch.h"
#include "MeterMap.h"
#include <inttypes.h>

// counts occurrences of unspecified integer values
MeterMap::MeterMap(std::string szSetName)
{
	szName = szSetName;
}

MeterMap::~MeterMap()
{
	intMap.clear();
}

void MeterMap::reset(void)
{
	intMap.clear();
}

// add an item to the map if it does not already exist then
// map[item] += value
void MeterMap::add(int item, int value)
{
	if (exists(item))
		intMap[item] += value;
	else
		intMap.insert(std::pair<int, int>(item, value));
}

bool MeterMap::exists(int item)
{
	return intMap.find(item) != intMap.end();
}

// add a user defined method for file output
void MeterMap::setKeyOutputFunction(const char* (*lpFunc)(int, uint64_t))
{
	lpKeyOutputFunc = lpFunc;
}

// output using a user defined method or the default
// optionally change output order hi to lo
void MeterMap::fileOutput(FILE* fp, bool hiLo)
{
	if (hiLo)
	{
		for (std::map<int, uint64_t>::iterator it = intMap.end(); it != intMap.begin(); --it)
			fprintf(fp, "%s\n", getOutputString(it).c_str());
	}
	else
	{
		for (std::map<int, uint64_t>::iterator it = intMap.begin(); it != intMap.end(); ++it)
			fprintf(fp, "%s\n", getOutputString(it).c_str());
	}
}


void MeterMap::fileOutput(std::ofstream& myfile)
{
	if (myfile.is_open())
	{
		for (std::map<int, uint64_t>::iterator it = intMap.begin(); it != intMap.end(); ++it)
			myfile << getOutputString(it) << "\n";
	}
}

// Vertically output to screen using a user defined method or the default
// optionally change output order hi to lo
void MeterMap::consoleOutV(bool hiLo)
{
	std::cout << szName << std::endl;
	if (hiLo)
	{
  for (std::map<int, uint64_t>::iterator it = intMap.end(); it != intMap.begin(); --it)
            printf("%8d,%8" PRIu64 "\n", it->first, static_cast<uint64_t>(it->second));
	}
	else
	{
  for (std::map<int, uint64_t>::iterator it = intMap.begin(); it != intMap.end(); ++it)
            printf("%8d,%8" PRIu64 "\n", it->first, static_cast<uint64_t>(it->second));
	}
}

// Horizontally output to screen using a user defined method or the default
void MeterMap::consoleOutH(bool showPercent) 
{
	std::map<int, uint64_t>::iterator it;
	uint64_t total = 0;
	if (showPercent) {
		for (it = intMap.begin(); it != intMap.end(); ++it)
			total += it->second;
	}
	std::cout << std::endl << szName << ",";
	for (it = intMap.begin(); it != intMap.end(); ++it)
		printf("%8d,", it->first);

	std::cout << std::endl << "Hits,";
 for (it = intMap.begin(); it != intMap.end(); ++it)
        printf("%8" PRIu64 ",", static_cast<uint64_t>(it->second));

	if (showPercent) {
		std::cout << std::endl << "Hit %,";
		for (it = intMap.begin(); it != intMap.end(); ++it)
			printf("%7.2f%%,", (float)it->second * 100 / total);
		printf("\n");
	}
}


std::string MeterMap::getOutputString(std::map<int, uint64_t>::iterator& it)
{
	std::string szOut;

	if (lpKeyOutputFunc)
		szOut = lpKeyOutputFunc(it->first, it->second);
	else if (szName.size())
		szOut = szName + " " + std::to_string(it->first) + "," + std::to_string(it->second);
	else
		szOut = std::to_string(it->first) + "," + std::to_string(it->second);
	return szOut;
}