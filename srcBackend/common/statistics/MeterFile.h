#pragma once

#include <string>
#include <fstream>

// MathsLibrary
#include "MeterList.h"
#include "MeterMap.h"

class MeterFile
{
public:
	FILE* fp;
	bool isTextFile;
	std::string szName, szFolder;
	int meterCount = 0;
	std::vector<MeterList*> meterListVector;
	std::vector<MeterMap*> meterMapVector;

	// create a file
	// csv is default but may be txt
	MeterFile(std::string szFileName, bool isText = false);
	MeterFile(std::string szFileName, bool isText, std::string szFolderName);

	// pass game name, sim version, maths version, target RTP
	void setOutputFolder(std::string szFolderName);
	void openFile(std::string szFileName, bool isText);

	void addMeters(MeterList* lpMeterList);
	void addMeters(MeterMap* lpMeterMap);

	// Output:	header data and data/time stamp
	//			Optionally Elapsed time 
	//			all added meters
	// Close the file when output is complete
	void output(double duration = 0);


	// Output header data and data/time stamp
	// Optionally Elapsed time
	void outputHeader(double duration = 0);

	// Output all added meters
	// Close the file when output is complete
	void outputMeters(void);

	// Output a single line "szName, szData"
	void outputLine(const char* szName, const char* szData);
};

