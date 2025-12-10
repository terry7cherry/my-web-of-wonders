/*
#include <chrono>
#include <ctime>
#include <cstdio>
#include <filesystem>

// cpp-dev-library
#include "pch.h"
#include "MeterFile.h"


// constructor: use default output folder, optionally set type as text
//  isText ? ".txt" : ".csv"
MeterFile::MeterFile(std::string szFileName, bool isText)
{
	setOutputFolder("Meters");
	openFile(szFileName, isText);
}

// constructor: use passed folder and must specify file type
//  isText ? ".txt" : ".csv"
MeterFile::MeterFile(std::string szFileName, bool isText, std::string szFolderName)
{
	setOutputFolder(szFolderName);
	openFile(szFileName, isText);
}

void MeterFile::setOutputFolder(std::string szFolderName)
{
	szFolder = szFolderName;
	if (MathLibrary::makeSubFolder(szFolder))
		exit(1);
}

void MeterFile::openFile(std::string szFileName, bool isText)
{
	isTextFile = isText;
	namespace fs = std::filesystem;
	fs::path folder(szFolder);
	fs::path file = folder / (szFileName + (isTextFile ? ".txt" : ".csv"));
	szName = file.string();

#ifdef _WIN32
	fopen_s(&fp, szName.c_str(), "w");
#else
	fp = std::fopen(szName.c_str(), "w");
#endif
}

void MeterFile::addMeters(MeterList* lpMeterList)
{
	meterListVector.push_back(lpMeterList);
	meterMapVector.push_back(nullptr);
	meterCount++;
}

void MeterFile::addMeters(MeterMap* lpMeterMap)
{
	meterListVector.push_back(nullptr);
	meterMapVector.push_back(lpMeterMap);
	meterCount++;
}

void MeterFile::output(double duration)
{
	outputHeader(duration);
	outputMeters();
}

void MeterFile::outputHeader(double duration)
{
	auto currentTime = std::chrono::system_clock::now();
	// Convert the system time to a time_t value (seconds since epoch)
	std::time_t nowTime_t = std::chrono::system_clock::to_time_t(currentTime);
	std::tm localTime{};
#ifdef _WIN32
	localtime_s(&localTime, &nowTime_t);
#else
	localtime_r(&nowTime_t, &localTime); //POSIX
#endif
	int year = localTime.tm_year + 1900;  // Years since 1900
	int month = localTime.tm_mon + 1;      // Months start from 0
	int day = localTime.tm_mday;
	int hour = localTime.tm_hour;
	int minute = localTime.tm_min;
	int second = localTime.tm_sec;

	std::string szHeader = gameVersion.szName;
	if (gameVersion.szSimDigits.size())
		szHeader += (" Sim v" + gameVersion.szSimDigits);
	szHeader += (" Maths " + gameVersion.szMathsDigits);
	if (gameVersion.szRtp.size())
		szHeader += (" RTP " + gameVersion.szRtp + "%%");
	szHeader += "\n";

	fprintf(fp, szHeader.c_str());
	fprintf(fp, "%04d-%02d-%02d,", year, month, day);
	fprintf(fp, "%02d:%02d:%02d\n\n", hour, minute, second);
	if (duration)
		fprintf(fp, "Elapsed Time (s), %6.2f\n", duration);
}

void MeterFile::outputLine(const char * szName, const char* szData)
{
	fprintf(fp, "%s, %s\n", szName, szData);
}

void MeterFile::outputMeters()
{
	for (int i = 0; i < meterCount; i++)
	{
		if (meterListVector[i])
			meterListVector[i]->fileOutput(fp);
		else
			meterMapVector[i]->fileOutput(fp);
	}
	fclose(fp);
}
*/