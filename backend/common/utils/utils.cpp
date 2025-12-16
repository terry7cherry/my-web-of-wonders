#include "../headers/pch.h"
#include <filesystem>
#include "../rng/MathsRng.h"
#include "../rng/RngData.h"
#include "../console/Console.h"

namespace fs = std::filesystem;

VERSION_DATA gameVersion;

// global Class instances need to be declared in ".c"
// end externed in the Class definition, "*.h" files
MathsRng mathRng;
RngData rnStream;

// Display a formatted error message to the console
// "szErrorType: szFunction() - szMessage"
void reportError(std::string szFunction, std::string szMessage, std::string szErrorType) {
	std::cout << std::endl << szErrorType << ": " << szFunction << " - " << szMessage << std::endl;
}

// try to create passed szSubFolder
// if exists or created then return 0
// else report Error and return system error number
int makeSubFolder(const std::string &folder) {
	try {
		if (!fs::exists(folder)) {
			fs::create_directories(folder);
		}
		return false; // no error
	} catch (const std::exception &e) {
		std::cerr << "Error creating directory: " << e.what() << std::endl;
		return 1; // error happened
	}
}


// DEL TEL - move to class Console
void showTabs(int count, bool startNewLine) {
	if (startNewLine) std::cout << std::endl;
	for (int i = 0; i < count; i++)
		std::cout << "\t";
}

// DEL TEL - move to class Console
void showInt(int tabCount, std::string szDesc, int val, bool startNewLine) {
	showTabs(tabCount, startNewLine);
	std::cout << szDesc << std::to_string(val);
}

// DEL TEL - move to class Console
// displays content of passed vector, 'szTitle = i0,i1,i2,...,iN'
void showVector(std::string szTitle, std::vector<int> &showVector) {
	printf("%s = ", szTitle.c_str());
	if (showVector.size()) {
		int lastIndex = (int) showVector.size() - 1;
		for (size_t i = 0; i < showVector.size(); i++)
			printf("%8d%s", showVector[i], (i == lastIndex) ? "" : ", ");
	} else
		printf("empty");
	printf("\r\n");
}

// DEL TEL - move to class Console
// displays content of passed vector, 'szTitle = i0,i1,i2,...,iN'
void showVector(std::string szTitle, std::vector<uint64_t> &showVector, bool showPercents) {
	uint64_t totalCount = 0;
	if (showVector.size()) {
		printf("%s  = ", szTitle.c_str());
		int lastIndex = (int) showVector.size() - 1;
		for (size_t i = 0; i < showVector.size(); i++) {
			printf("%8lld%s", showVector[i], (i == lastIndex) ? "" : ", ");
			totalCount += showVector[i];
		}
		printf("\r\n");

		if (showPercents && totalCount) {
			printf("%s%% = ", szTitle.c_str());
			for (size_t i = 0; i < showVector.size(); i++)
				printf("%8.2f%s", (float) showVector[i] * 100 / totalCount, (i == lastIndex) ? "" : ", ");
			printf("\r\n");
		}
	} else
		printf("empty]\r\n");
}


void showAverageWin(std::string szTitle, std::vector<uint64_t> &winVector, std::vector<uint64_t> &hitsVector,
                    int bet) {
	uint64_t totalCount = 0;
	if (winVector.size()) {
		printf("%s  = [", szTitle.c_str());
		int lastIndex = (int) winVector.size() - 1;
		for (size_t i = 0; i < winVector.size(); i++)
			printf("%6.2f%s", (float) winVector[i] / hitsVector[i] / bet, (i == lastIndex) ? "" : ", ");
		printf("]\r\n");
	} else
		printf("empty]\r\n");
}
