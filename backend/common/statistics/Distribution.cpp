#include "../headers/pch.h"
#include "Distribution.h"

;

namespace MathLibrary
{
	// Check subfolder exists, create if not
	// Check file can be opened, report issue if not
	void Distribution::checkFile(void)
	{
		// set folder
		szFilePath = szDistributionFolder + "\\" + szFilename  + szExtension;
		if (makeSubFolder(szDistributionFolder))
		{
			reportError("Distribution.checkFile", "Can't create sub-folder: " + szDistributionFolder);
			exit(1);
		}

		// make sure we can open file before simulation starts
		std::ofstream outfile;
		outfile.open(szFilePath);
		if (!outfile.is_open())
			reportError("Distribution.checkFile", "Can't open file: " + szFilePath, "Warning");
		outfile.close();
	}

	// Create fullFilename suitable for tableau, "baseFileName-szMode_rtp_bet" 
	// Check file can be opened
	void Distribution::setFilename(std::string szRtp, std::string szBet, std::string szMode)
	{
		// set the filename
		szFilename = szBaseFilename;
		if (szMode.size())
			szFilename = szFilename + szMode;
		if (szRtp.size())
			szFilename += (szSep + szRtp);
		if (szBet.size())
			szFilename += (szSep + szBet);
		checkFile();
	}

	// add an item to the distribution if it does not already exist
	// then increment its count by pased value
	void Distribution::add(int item, int value)
	{
		if (exists(item))
			intMap[item] += value;
		else
			intMap.insert(std::pair<int, int>(item, value));
	}

	// output the distribution to a csv file as <value, count> pair on each line
	// Parameter hiFirst sets the output order, true = high to low
	// Pass doConsoleStatus = true if you want to see the tile of the written file on the console
	void Distribution::fileOutput(bool hiFirst, bool doConOut)
	{
		std::ofstream outfile;

		outfile.open(szFilePath);
		if (!outfile.is_open())
		{
			reportError("Distribution.fileOutput", "Can't open file: " + szFilename + ". Output aborted", "Warning");
			return;
		}

		if (doConOut)
			std::cout << "Writing Distribution file: " << szFilename << ". . .";

		if (hiFirst)
		{					// output in reverse (hi to lo)
			for (auto it = intMap.rbegin(); it != intMap.rend(); ++it)
				outfile << std::to_string(it->first) << ", " << std::to_string(it->second) << std::endl;
		}
		else
		{
			for (auto it = intMap.begin(); it != intMap.end(); ++it)
				outfile << std::to_string(it->first) << ", " << std::to_string(it->second) << std::endl;
		}
		outfile.close();

		if (doConOut)
			std::cout << "Done\r\n";
	}
}