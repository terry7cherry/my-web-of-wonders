#ifndef _WIN32
#include <unistd.h>
#endif

#include "../headers/pch.h"

#include "Console.h"


#ifdef _WIN32
// deprecated - does not allow full screen to be set on Windows 11
// use initialise instead
void Console::init(const bool fullScreen)
{
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	consoleWindow = GetConsoleWindow();
	if (fullScreen)
		ShowWindow(consoleWindow, SW_MAXIMIZE);
	GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
	auto [X, Y] = GetLargestConsoleWindowSize(consoleHandle);
	maxLines = Y;
	maxWidth = X;
}

// sets up the console for use on Windows 11 and earlier
void Console::initialise(const bool fullScreen)
{
	Sleep(1000);
	//If you execute this code immediately after the program starts, you must wait here for a short period of time, otherwise GetWindow will fail. I speculate that it may be because the console has not been fully initialized.
	const HWND local_hwnd = GetConsoleWindow();
	GetWindow(local_hwnd, GW_OWNER);

	if (fullScreen)
		ShowWindow(CONSOLE_HWND, SW_MAXIMIZE);

	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	consoleWindow = GetConsoleWindow();

	GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
	auto [X, Y] = GetLargestConsoleWindowSize(consoleHandle);
	maxLines = Y;
	maxWidth = X;
	//SetWindowPos(CONSOLE_HWND, nullptr, 0, 0, 1600, 800, SWP_NOZORDER | SWP_NOMOVE);
}


// Set the font size.
// Optionally ensure console is minCharWidth characters wide, by reducing font size
void Console::newFontSize(const SHORT size, const int minCharWidth)
{
	fontSize = size;

	setFontSize(fontSize); // set font then see how many chars we have to play with
	COORD NewSBSize = GetLargestConsoleWindowSize(consoleHandle);
	while (minCharWidth && NewSBSize.X < minCharWidth)
	{
		fontSize -= 1;
		setFontSize(fontSize); // set font then see how many chars we have to play with
		NewSBSize = GetLargestConsoleWindowSize(consoleHandle);
	}
	maxLines = NewSBSize.Y;
	maxWidth = NewSBSize.X;
}

// Set the font size.
// Ensure, by reducing font size, that console has minCharWidth x minCharHeight characters
void Console::newFontSize(const SHORT size, int minCharWidth, int minCharHeight)
{
	fontSize = size;

	setFontSize(fontSize); // set font then see how many chars we have to play with
	COORD NewSBSize = GetLargestConsoleWindowSize(consoleHandle);
	while (NewSBSize.X < minCharWidth || NewSBSize.Y < minCharHeight)
	{
		fontSize -= 1;
		setFontSize(fontSize); // set font then see how many chars we have to play with
		NewSBSize = GetLargestConsoleWindowSize(consoleHandle);
	}
	maxLines = NewSBSize.Y;
	maxWidth = NewSBSize.X;
}

int Console::coutStyle(const std::string& text, const WORD style, const bool revert)
{
	//GetConsoleScreenBufferInfo(consoleHandle, )
	setStyle(style);
	std::cout << text;

	if (revert)
		setStyle(consoleInfo.wAttributes);

	return static_cast<int>(text.length());
}

// Displays "PRESS [SPACE BAR] " + text
void Console::showPressText(std::string text, int stringLimit, int row, int col)
{
	if (row != -1)
	{
		std::string clrLine = std::string(stringLimit, ' ');
		setCursorPos(row, col);
		std::cout << clrLine << "\r";
	}

	setStyle(CONSOLE_INVERT);
	printf(" PRESS ");
	setStyle(CONSOLE_INPUT);
	printf("[SPACE BAR]");
	setStyle(CONSOLE_INVERT);
	printf(" %s ", text.c_str());
	blnWaitPress = true;
}

// Displays "PRESS [ANY KEY] " + text
// Waits for a key press
void Console::waitAnyKeyText(std::string text, int stringLimit, int row, int col)
{
	std::string clrLine = std::string(stringLimit, ' ');
	if (row != -1)
	{
		setCursorPos(row, col);
		std::cout << clrLine << "\r";
	}

	setStyle(CONSOLE_INVERT);
	printf(" PRESS ");
	setStyle(CONSOLE_INPUT);
	printf("[ANY KEY]");
	setStyle(CONSOLE_INVERT);
	printf(" %s ", text.c_str());
	setStyle(CONSOLE_STD);

	clearBuffer();
	waitAnyKey();

	setCursorPos(row, col);
	std::cout << clrLine << "\r";
}

void Console::setClearLineText(int lineLength)
{
	sprintf_s(szClrLine, 10, "%%%ds", lineLength);
}

// Clear the line and re-set cursor to start of line
// Optionally do not re-set cursor
void Console::clearLine(int line, bool setToStart)
{
	setCursorPos(line);
	printf(szClrLine, " ");
	if (setToStart)
		setCursorPos(line);
}

void Console::flashText(std::string text, int line, int col, int msSpeed, int repeat,
                        bool endInvert)
{
	for (int i = 0; i < repeat; i++)
	{
		setCursorPos(line, col); // Highlight text
		coutStyle(text, CONSOLE_INVERT);
		Sleep(msSpeed);

		setCursorPos(line, col); // Standard text
		std::cout << text;
		Sleep(msSpeed);
	}
	if (endInvert)
	{
		// Highlight text
		setCursorPos(line, col);
		coutStyle(text, CONSOLE_INVERT);
	}
}

/*********************************************************/
// Easy interfaces to std Windows methods
/*********************************************************/

void Console::hideCursor(void)
{
	CONSOLE_CURSOR_INFO cursor;
	cursor.bVisible = false;
	cursor.dwSize = 20; // Needs a valid size. Fails if 0

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
}

void Console::setWindowPos(const int fromLeft, const int fromTop) const
{
	SetWindowPos(consoleWindow, HWND_TOP, fromLeft, fromTop, 0, 0, SWP_NOSIZE);
}

void Console::setWindowSize(const int width, const int height)
{
	const HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, 0, 0, width, height, true);
}

void Console::clearScreen(void)
{
	for (int i = 0; i < 300; i++)
		std::cout << "\n";
}

void Console::setCursorPos(int row, int col)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = {(SHORT)col, (SHORT)row};
	SetConsoleCursorPosition(hConsole, pos);
}

void Console::setFontSize(const SHORT size)
{
	CONSOLE_FONT_INFOEX font;
	font.cbSize = sizeof(font);
	font.nFont = 0;
	font.dwFontSize.X = 0; // Width of each character in the font
	font.dwFontSize.Y = size; // Height
	font.FontFamily = FF_DONTCARE;
	font.FontWeight = FW_NORMAL;
	wcscpy_s(font.FaceName, LF_FACESIZE, L"Consolas");
	//wcscpy(font.FaceName, L"Consolas"); // Choose your font

	SetCurrentConsoleFontEx(consoleHandle, false, &font);
}

void Console::setStyle(const WORD style)
{
	SetConsoleTextAttribute(consoleHandle, style);
}

/**********************************/
// Keyboard Input Methods
/**********************************/
void Console::waitKeyPress(char waitChar)
{
	char key;

	for (;;)
	{
		if (!getKeyPress(key)) continue;

		if (key == waitChar) break;
	}

	clearBuffer();
	blnWaitPress = false;
}

char Console::waitAnyKey(void)
{
	const char keyPress = static_cast<char>(_getch());
	clearBuffer();
	return keyPress;
}

bool Console::getKeyPress(char& key)
{
	if (!_kbhit()) return false;

	key = static_cast<char>(_getch());
	return true;
}

// szText is a question that has a y/n answer
// szText is displayed at left edge of screen on current line
// method asks the question, waits for a Y/N key press and returns true/false
bool Console::confirmYesNo(std::string szText)
{
	int ch;

	do
	{
		std::cout << "\r" << szText << " (y or n) > ";

		ch = waitAnyKey();
		std::cout << (char)ch;
		ch = toupper(ch);
	}
	while ((ch != 'Y') && (ch != 'N'));
	return (ch == 'Y');
}

// szText is positiond at the passed row and col
// method asks the question, waits for a Y/N key press and returns true/false
bool Console::confirmYesNo(std::string szText, int row, int col)
{
	int ch;

	do
	{
		setCursorPos(row, col);
		std::cout << szText << " (y or n) > ";

		ch = waitAnyKey();
		std::cout << (char)ch;
		ch = toupper(ch);
	}
	while ((ch != 'Y') && (ch != 'N'));
	return (ch == 'Y');
}

void Console::clearBuffer(void)
{
	while (_kbhit())
		_getch();
}

// Create and return a string of length, width.
// Original string, szIn, is centred within the new string
// Optinally pass padAfter = false to create a shorter string with no white space at the end
std::string Console::centreString(std::string szIn, int width, bool padAfter)
{
	std::string newString((width - szIn.length()) / 2, ' ');
	newString += szIn;

	if (padAfter)
	{
		std::string padString(width - newString.length(), ' ');
		newString += padString;
	}
	return newString;
}
#else  // non-Windows stubs

void Console::init(bool)
{
}
void Console::initialise(bool)
{
}
void Console::setClearLineText(int)
{
}
void Console::clearLine(int, bool)
{
}
void Console::flashText(std::string, int, int, int, int, bool)
{
}
int Console::coutStyle(std::string text, int, bool)
{
	std::cout << text;
	return 0;
}
void Console::showPressText(std::string, int, int, int)
{
}
void Console::waitAnyKeyText(std::string, int, int, int)
{
}
void Console::setFontSize(int)
{
}
void Console::newFontSize(int, int)
{
}
void Console::newFontSize(int, int, int)
{
}
void Console::hideCursor()
{
}
void Console::setWindowPos(int, int) const
{
}
void Console::setWindowSize(int, int)
{
}
void Console::clearScreen()
{
	// ANSI clear for POSIX terminals
	std::cout << "\033[0m\033[2J\033[3J\033[H" << std::flush;
}
void Console::setCursorPos(int row, int col)
{
	if (row < 1) row = 1;
	if (col < 1) col = 1;
	std::printf("\033[%d;%dH", row, col);
	std::fflush(stdout);
}
void Console::setStyle(int style)
{
	// Support invert (7 or 112) and reset (others)
	if (style == 112 || style == 7)
		std::cout << "\033[7m";
	else
		std::cout << "\033[0m";
	std::cout << std::flush;
}
char Console::waitAnyKey()
{
	int ch = std::getchar();
	if (ch == EOF) ch = 0;
	clearBuffer();
	return static_cast<char>(ch);
}
void Console::waitKeyPress(char waitChar)
{
	int ch = 0;
	do { ch = std::getchar(); }
	while (ch != EOF && static_cast<char>(ch) != waitChar);
	clearBuffer();
	blnWaitPress = false;
}
bool Console::getKeyPress(char&)
{
	// No non-blocking polling implemented on POSIX in this simplified version
	return false;
}
bool Console::confirmYesNo(std::string) { return true; }
bool Console::confirmYesNo(std::string, int, int) { return true; }
void Console::clearBuffer()
{
	// No-op on POSIX in this simplified version
}
std::string Console::centreString(std::string szIn, int width, bool padAfter)
{
	std::string newString((width - szIn.length()) / 2, ' ');
	newString += szIn;
	if (padAfter)
	{
		if (width > static_cast<int>(newString.length()))
			newString.append(width - newString.length(), ' ');
	}
	return newString;
}

#endif // _WIN32
