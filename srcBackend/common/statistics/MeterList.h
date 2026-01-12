#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include "../headers/types.h"

#define METER_TEXT_LEN (50)
#define METER_OUTPUT_TAB (1)
#define METER_OUTPUT_CSV (2)

;

class MeterList
{
public:
	std::string szName;
	int size;
	int64* valueList;
	std::vector <std::string>* lpNameVector;

	MeterList(std::string szSetName, std::vector <std::string>* lpNames, int refSize = 0);

	// set all meters to zero
	void reset(void);

	//   meter[id]++
	void inc(int id);

	//   meter[id] +=  val
	void add(int id, int64 val);

	//   meter[id] = val
	void set(int id, int64 val);

	//   set if val > meter[id]
	bool setIfMax(int id, int64 val);

	//   set if val < meter[id]
	bool setIfMin(int id, int64 val);

	//   retrun meter[id]
	int64 get(int id);

	// meter output text (name-value) may be comma or tab separated
	// METER_OUTPUT_TAB or METER_OUTPUT_CSV (default)
	void setTextType(int type);

	//   return meter id and value string
	char *getText(int id);

	// Write all meter id and value strings to the passed file
	void fileOutput(FILE* fp);

	// Write all meter id and value strings to screen
	void toConsole(void);

private:
	void reportError(int id, std::string szErrFunc);

	int outputType;
	const char* szSeparator;
	char text[METER_TEXT_LEN];
};
