// cpp-dev-library
#include "../headers/pch.h"
#include "MeterList.h"
#include <cstring>   // for memset
#include <cstdio>    // for std::snprintf
#include <cinttypes> // for PRId64 if you want fully portable int64 printing
;

MeterList::MeterList(std::string szSetName, std::vector <std::string>* lpNames, int refSize)
{
	szName = szSetName;
	lpNameVector = lpNames;
	size = (int)lpNameVector->size();
	if (refSize && refSize != size)
		printf("\r\nMeterList %s: refSize <%d> != lpNames->size() <%d>", 
			szName.c_str(), refSize, size);

	valueList = new int64 [size];			// allocate an array of meters
	reset();

	setTextType(METER_OUTPUT_CSV);			// default to csv output
}


// set all meters to zero
void MeterList::reset(void)
{
	std::memset(valueList, 0, sizeof(int64) * size);
}

//   meter[id]++
void MeterList::inc(int id)
{
	valueList[id]++;
}

//   meter[id] +=  val
void MeterList::add(int id, int64 val)
{
	valueList[id] += val;
}

//   meter[id] = val
void MeterList::set(int id, int64 val)
{
	valueList[id] = val;
}

//   set if val > meter[id]
bool MeterList::setIfMax(int id, int64 val)
{
	bool isMax = false;
	if (val > valueList[id])
	{
		isMax = true;
		valueList[id] = val;
	}
	return isMax;
}

//   set if val < meter[id]
bool MeterList::setIfMin(int id, int64 val)
{
	bool isMin = false;
	if (val < valueList[id])
	{
		isMin = true;
		valueList[id] = val;
	}
	return isMin;
}

//   retrun meter[id]
int64 MeterList::get(int id)
{
	return valueList[id];
}

// meter output text (name-value) may be comma or tab separated
// METER_OUTPUT_TAB or METER_OUTPUT_CSV (default)
void MeterList::setTextType(int type)
{
	outputType = type;

	if (outputType == METER_OUTPUT_TAB)
		szSeparator = "\t";
	else
		szSeparator = ",";
}

//   return meter id and value string
char* MeterList::getText(int id)
{
	if (szName.length())
		std::snprintf(text, METER_TEXT_LEN,
			"%s %s%s%" PRId64 "\n",
			szName.c_str(),
			(*lpNameVector)[id].c_str(),
			szSeparator,
			valueList[id]);
	else
		std::snprintf(text, METER_TEXT_LEN,
			"%s%s%" PRId64 "\n",
			(*lpNameVector)[id].c_str(),
			szSeparator,
			valueList[id]);

	return text;
}

// Write all meter id and value strings to the passed file
void MeterList::fileOutput(FILE* fp)
{
	for (int i = 0; i < size; i++)
		fprintf(fp, getText(i));
}

void MeterList::toConsole(void)
{
	for (int i = 0; i < size; i++)
		printf("%s", getText(i));
}

