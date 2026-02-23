#pragma once

#include "../headers/pch.h"
#include "../headers/types.h"
#include <cstdint>

;

// Display a formatted error message to the console
// "szErrorType: szFunction() - szMessage"
extern void reportError(std::string szFunction, std::string szMessage, std::string szErrorType = "Error");

extern int makeSubFolder(const std::string& szFolder);

extern void showTabs(int count, bool startNewLine = true);

extern void showInt(int tabCount, std::string szDesc, int val, bool startNewLine = true);

extern void showVector(std::string szTitle, std::vector<int>& showVector);

extern void showVector(std::string szTitle, std::vector<uint64_t>& showVector, bool showPercents = false);

extern void showAverageWin(std::string szTitle, std::vector<uint64_t>& winVector,
                           std::vector<uint64_t>& hitsVector,
                           int bet = 1);

extern std::string addThousandsSeparator(uint64_t n);
