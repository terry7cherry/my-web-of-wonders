#include "../headers/pch.h"
#include "Timer.h"

#include <iomanip>

;
using namespace std::chrono;

namespace MathLibrary
{
	// Creates a timer. Starts the timer unless doStart is passed false
	Timer::Timer(std::string szTimerName, bool doStart)
	{
		szName = szTimerName;
		if (doStart) start();
	}

	// Sets a start time and marks timer as active
	// Will re-set start time, with Warning, if called when timer already active
	void Timer::start(void)
	{
		if (!isActive)
			isActive = true;
		else
			reportError(":Timer.start", szName + " was re-started", "Warning");
		tStart = high_resolution_clock::now();
	}

	// Sets a stop time and marks timer as not active
	void Timer::stop(void)
	{
		if (isActive)
		{
			tLast = high_resolution_clock::now();
			isActive = false;
		}
		else
			reportError(":Timer.stop", szName + " was not active", "Warning");
	}

	// Show timer in micro seconds on console
	// Will stop the timer first unless you pass doStop = false
	void Timer::outputMicro(bool doStop)
	{
		if (isActive)
		{
			if (doStop)
				isActive = false;
			tLast = high_resolution_clock::now();
			auto duration = duration_cast<microseconds> (tLast - tStart);
			std::cout << std::fixed << std::setprecision(2) << "Micro second Timer: " << szName
				<< " = " << duration.count() << '\n';
			isActive = false;
		}
		else
			reportError("Timer.outputMicro", szName + " was not active", "Warning");
	}

	// Show timer in mil seconds on console
	// will stop the timer first unless you pass doStop = false
	void Timer::outputMilli(bool doStop)
	{
		if (isActive)
		{
			if (doStop)
				isActive = false;
			tLast = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds> (tLast - tStart);
			std::cout << std::fixed << std::setprecision(2) << "Milli second Timer: " << szName
				<< " = " << duration.count() << '\n';
		}
		else
			reportError("Timer.outputMilli", szName + " was not active", "Warning");
	}

	// Show timer in hrs:min:sec on console
	// will stop the timer first unless you pass doStop = false
	void Timer::outputTime(bool doStop)
	{
		if (isActive)
		{
			if (doStop)
				isActive = false;
			tLast = high_resolution_clock::now();
			auto duration = duration_cast<seconds> (tLast - tStart);
			auto hrs = duration_cast<hours> (duration);
			duration -= hrs;	
			auto min = duration_cast<minutes> (duration);
			duration -= min;
			auto sec = duration_cast<seconds> (duration);

			std::cout << std::fixed << std::setprecision(0) << "Timer: " << szName << " = "
				<< hrs.count() << "h, " 
				<< min.count() << "m, "
				<< sec.count() << "s\n";
		}
		else
			reportError("Timer.outputMilli", szName + " was not active", "Warning");
	}

}