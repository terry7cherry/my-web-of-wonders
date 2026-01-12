// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <vector>
#include <map>
#include <list>
#include <unordered_map>

#include <string>
#include <stdio.h>
#include <cstdio>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif
#include <algorithm>
#include <atomic>
#include <thread>
#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>   // provides getpid()
inline int getch() { return getchar(); }
inline int kbhit() { return 0; } // stub
#endif
#include <iostream>
#include <sstream>
#include <fstream>

#include "types.h"
#include "../utils/utils.h"

#endif //PCH_H
