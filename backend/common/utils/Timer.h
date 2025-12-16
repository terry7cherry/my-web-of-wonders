#pragma once

#include <ctime>
#include <chrono>
#include <iostream>

class Timer {
private:
	bool isActive = false;

public:
	std::string szName;
	std::chrono::time_point<std::chrono::high_resolution_clock> tStart;
	std::chrono::time_point<std::chrono::high_resolution_clock> tLast;

	// Creates a timer. Starts the timer unless doStart is passed false
	Timer(std::string szTimerName, bool doStart = true);

	// Sets a start time and marks timer as active
	// Will re-set start time, with Warning, if called when timer already active
	void start(void);

	// Sets a stop time and marks timer as not active
	void stop(void);

	// Show timer in micro seconds on console
	// will stop the timer first unless you pass doStop = false
	void outputMicro(bool doStop = true);

	// Show timer in mil seconds on console
	// will stop the timer first unless you pass doStop = false
	void outputMilli(bool doStop = true);

	// Show timer in hrs:min:sec on console
	// will stop the timer first unless you pass doStop = false
	void outputTime(bool doStop = true);
};
