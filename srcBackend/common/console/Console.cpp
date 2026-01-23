// We implement everything using ANSI escape codes and standard C++,
// so it works in CLion's emulated terminal across Windows, macOS, and Linux.

#include "../headers/pch.h"

#include "Console.h"

void Console::clearScreen(void)
{
    std::cout << "\033[2J\033[H\033[3J" << std::flush;
}


// Set the font size - not supported in generic terminals; no-op
void Console::setFontSize(const short size)
{
    fontSize = size;
}

// Optionally ensure console is minCharWidth characters wide, by reducing font size (not supported)
void Console::newFontSize(const short size, const int /*minCharWidth*/)
{
    fontSize = size;
}

// Ensure, by reducing font size, that console has minCharWidth x minCharHeight characters (not supported)
void Console::newFontSize(const short size, int /*minCharWidth*/, int /*minCharHeight*/)
{
    fontSize = size;
}

int Console::coutStyle(const std::string& text, const unsigned short style, const bool revert)
{
    setStyle(style);
    std::cout << text;
    if (revert)
        setStyle(CONSOLE_STD);
    return static_cast<int>(text.length());
}

// Displays "PRESS [SPACE BAR] " + text
void Console::showPressText(std::string text, int stringLimit, int row, int col)
{
    if (row != -1)
    {
        std::string clrLine(stringLimit, ' ');
        setCursorPos(row, col);
        std::cout << clrLine << "\r";
    }

    setStyle(CONSOLE_INVERT);
    std::cout << " PRESS ";
    setStyle(CONSOLE_INPUT);
    std::cout << "[SPACE BAR]";
    setStyle(CONSOLE_INVERT);
    std::cout << ' ' << text << ' ';
    std::cout.flush();
    blnWaitPress = true;
}

// Displays "PRESS [ANY KEY] " + text and waits for a key
void Console::waitAnyKeyText(std::string text, int stringLimit, int row, int col)
{
    std::string clrLine(stringLimit, ' ');
    if (row != -1)
    {
        setCursorPos(row, col);
        std::cout << clrLine << "\r";
    }

    setStyle(CONSOLE_INVERT);
    std::cout << " PRESS ";
    setStyle(CONSOLE_INPUT);
    std::cout << "[ANY KEY]";
    setStyle(CONSOLE_INVERT);
    std::cout << ' ' << text << ' ';
    setStyle(CONSOLE_STD);

    clearBuffer();
    waitAnyKey();

    setCursorPos(row, col);
    std::cout << clrLine << "\r" << std::flush;
}

void Console::setClearLineText(int lineLength)
{
    if (lineLength < 0) lineLength = 0;
    std::snprintf(szClrLine, sizeof(szClrLine), "%%%ds", lineLength);
}

// Clear the line and re-set cursor to start of line
// Optionally do not re-set cursor
void Console::clearLine(int line, bool setToStart)
{
    setCursorPos(line);
    std::printf(szClrLine, " ");
    if (setToStart)
        setCursorPos(line);
}

void Console::flashText(std::string text, int line, int col, int msSpeed, int repeat,
                        bool endInvert)
{
    for (int i = 0; i < repeat; ++i)
    {
        setCursorPos(line, col); // Highlight text
        coutStyle(text, CONSOLE_INVERT);
        Console::pause(msSpeed);

        setCursorPos(line, col); // Standard text
        std::cout << text << std::flush;
        Console::pause(msSpeed);
    }
    if (endInvert)
    {
        setCursorPos(line, col);
        coutStyle(text, CONSOLE_INVERT);
    }
}

void Console::hideCursor(void)
{
    std::cout << "\033[?25l" << std::flush;
}

void Console::setCursorPos(int row, int col)
{
    if (row < 1) row = 1;
    if (col < 1) col = 1;
    std::printf("\033[%d;%dH", row, col);
    std::fflush(stdout);
}

void Console::setStyle(const unsigned short style)
{
    // Map a small set of logical styles to ANSI escapes.
    if (style == CONSOLE_INVERT || style == 7 || style == 112)
    {
        std::cout << "\033[7m";
    }
    else if (style == CONSOLE_INPUT)
    {
        // Bright white on blue as an "input" cue
        std::cout << "\033[97;44m";
    }
    else
    {
        // Default style
        std::cout << "\033[0m";
    }
    std::cout << std::flush;
}

// Keyboard Input Methods
void Console::waitKeyPress(char waitChar)
{
    int ch = 0;
    do { ch = std::getchar(); }
    while (ch != EOF && static_cast<char>(ch) != waitChar);
    clearBuffer();
    blnWaitPress = false;
}

char Console::waitAnyKey(void)
{
    int ch = std::getchar();
    if (ch == EOF) ch = 0;
    clearBuffer();
    return static_cast<char>(ch);
}

// szText is a question that has a y/n answer
// szText is displayed at left edge of screen on current line
// method asks the question, waits for a Y/N key press and returns true/false
bool Console::confirmYesNo(std::string szText)
{
    int ch;
    do
    {
        std::cout << "\r" << szText << " (y or n) > " << std::flush;
        ch = waitAnyKey();
        std::cout << static_cast<char>(ch) << std::flush;
        ch = toupper(ch);
    }
    while (ch != 'Y' && ch != 'N');
    return (ch == 'Y');
}

// szText is positioned at the passed row and col
// method asks the question, waits for a Y/N key press and returns true/false
bool Console::confirmYesNo(std::string szText, int row, int col)
{
    int ch;
    do
    {
        setCursorPos(row, col);
        std::cout << szText << " (y or n) > " << std::flush;
        ch = waitAnyKey();
        std::cout << static_cast<char>(ch) << std::flush;
        ch = toupper(ch);
    }
    while (ch != 'Y' && ch != 'N');
    return (ch == 'Y');
}

void Console::clearBuffer(void)
{
    // Drain any buffered input (best-effort, cross-platform)
    while (std::cin.rdbuf()->in_avail() > 0)
        std::cin.get();
}

// Create and return a string of length, width.
// Original string, szIn, is centred within the new string
// Optionally pass padAfter = false to create a shorter string with no white space at the end
std::string Console::centreString(std::string szIn, int width, bool padAfter)
{
    std::string newString((std::max(0, width - static_cast<int>(szIn.length()))) / 2, ' ');
    newString += szIn;
    if (padAfter)
    {
        const int remaining = width - static_cast<int>(newString.length());
        if (remaining > 0)
            newString.append(static_cast<size_t>(remaining), ' ');
    }
    return newString;
}
