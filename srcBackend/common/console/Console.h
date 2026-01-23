#pragma once

#include <thread>
#include <chrono>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
// Minimal stand-ins so the class layout compiles on non-Windows.
using HWND = void*;
using HANDLE = void*;
struct CONSOLE_SCREEN_BUFFER_INFO
{
};
#endif

// owner == NULL ? Windows 10 : Windows 11
// Only really meaningful on Windows, but we keep it defined everywhere.
#ifdef _WIN32
#define CONSOLE_HWND (owner == NULL ? hwnd : owner)
#else
#define CONSOLE_HWND hwnd
#endif

/****************************************************************/
// Colour definitions
/****************************************************************/
#ifdef _WIN32
#define TEXT_WHITE   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define TEXT_CYAN    (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define TEXT_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE)
#define TEXT_YELLOW  (FOREGROUND_RED | FOREGROUND_GREEN)
#define TEXT_RED     (FOREGROUND_RED)
#define TEXT_GREEN   (FOREGROUND_GREEN)
#define TEXT_BLUE    (FOREGROUND_BLUE)
#define TEXT_BLACK   (0)
#define TEXT_GREY    (TEXT_BLACK | FOREGROUND_INTENSITY)

#define BG_WHITE   (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define BG_CYAN    (BACKGROUND_GREEN | BACKGROUND_BLUE)
#define BG_MAGENTA (BACKGROUND_RED | BACKGROUND_BLUE)
#define BG_YELLOW  (BACKGROUND_RED | BACKGROUND_GREEN)
#define BG_RED     (BACKGROUND_RED)
#define BG_GREEN   (BACKGROUND_GREEN)
#define BG_BLUE    (BACKGROUND_BLUE)
#define BG_BLACK   (0)
#define BG_GREY    (BG_BLACK | BACKGROUND_INTENSITY)

#define CONSOLE_STD    (BG_BLACK | TEXT_WHITE)
#define CONSOLE_INVERT (BG_WHITE | TEXT_BLACK)
#define CONSOLE_INPUT  (BG_WHITE | TEXT_BLUE)
#define CONSOLE_FLASH  0
#else
// Cross-platform symbolic values used by Console::setStyle to emit ANSI escapes
#define TEXT_WHITE   0
#define TEXT_CYAN    0
#define TEXT_MAGENTA 0
#define TEXT_YELLOW  0
#define TEXT_RED     0
#define TEXT_GREEN   0
#define TEXT_BLUE    0
#define TEXT_BLACK   0
#define TEXT_GREY    0

#define BG_WHITE   0
#define BG_CYAN    0
#define BG_MAGENTA 0
#define BG_YELLOW  0
#define BG_RED     0
#define BG_GREEN   0
#define BG_BLUE    0
#define BG_BLACK   0
#define BG_GREY    0

// Use small integers so we can reliably compare and map to ANSI
#define CONSOLE_STD    0
#define CONSOLE_INVERT 1
#define CONSOLE_INPUT  2
#define CONSOLE_FLASH  0
#endif


class Console
{
public:
	bool blnWaitPress = false;
	std::string szHeader;
	int maxLines = 0;
	int maxWidth = 0;


	// remember the new string and show it
	// default action is to clear screen then show header
	void setHeader(std::string text, bool show = true, bool cls = true);

	// show the saved header at the top of the console
	// default is to clear screen and show in highlight text, CONSOLE_INVERT
	void showHeader(bool cls = true, bool invert = true) const;

	void setClearLineText(int lineLength);

	// Clear the line and re-set cursor to start of line
	// Optionally do not re-set cursor
	void clearLine(int line, bool setToStart = true) const;

	static void flashText(const std::string& text, int line, int col, int msSpeed, int repeat,
	                      bool endInvert = false);

	static int coutStyle(const std::string& text, unsigned short style, bool revert = true);

	// Displays "PRESS [SPACE BAR] " + text
	void showPressText(const std::string& text, int stringLimit, int row = -1, int col = 0);

	// Displays "PRESS [ANY KEY] " + text
	// Waits for a key press
	static void waitAnyKeyText(const std::string& text, int stringLimit, int row = -1, int col = 0);

	// sets the font size to the passed value
	void setFontSize(short size);

	// Set the font size.
	// Optionally ensure console is minCharWidth characters wide, by reducing font size
	void newFontSize(short size);

	// Set the font size.
	// Ensure, by reducing font size, that console has minCharWidth x minCharHeight characters
	void newFontSize(short size, int minCharWidth, int minCharHeight);

	/*********************************************************/
	// Easy interfaces to std / Windows methods
	/*********************************************************/

	static void pause(int msTimer);

	static void hideCursor();

	static void clearScreen();

	static void setCursorPos(int row, int col = 0);

	// apply a text/background colour style
	static void setStyle(unsigned short style);

	// Keyboard Input Methods
	static char waitAnyKey();

	void waitKeyPress(char waitChar);

	static bool confirmYesNo(const std::string& szText);

	static bool confirmYesNo(const std::string& szText, int row, int col = 0);

	static void clearBuffer();

	// Create and return a string of length, width.
	// Original string, szIn, is centred within the new string
	// Optinally pass padAfter = false to create a shorter string with no white space at the end
	static std::string centreString(const std::string& szIn, int width, bool padAfter = true);

protected:
	// On Windows these are real handles; on other platforms they're just placeholders.
	HWND hwnd = nullptr;
	HWND owner = nullptr;
	HANDLE consoleHandle = nullptr;
	HWND consoleWindow = nullptr;
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo{};
	short fontSize = 0;
	char szClrLine[10] = "%80s";
};

