#include <vector>
#include "consolestyling.h"
#include <stdio.h>
#include <wchar.h>
#include <windows.h>


bool isVirtual = false;

namespace CS
{

/*
    Check if we can enable virtual terminal (needed for ANSI escape sequences)

    From: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-enabling-virtual-terminal-processing
*/
bool init()
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwOriginalOutMode = 0;
    DWORD dwOriginalInMode = 0;
    if (!GetConsoleMode(hOut, &dwOriginalOutMode))
    {
        return false;
    }
    if (!GetConsoleMode(hIn, &dwOriginalInMode))
    {
        return false;
    }

    DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
    if (!SetConsoleMode(hOut, dwOutMode))
    {
        // We failed to set both modes, try to step down mode gracefully.
        dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
        if (!SetConsoleMode(hOut, dwOutMode))
        {
            // Failed to set any VT mode, can't do anything here.
            return false;
        }
    }

    DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
    if (!SetConsoleMode(hIn, dwInMode))
    {
        // Failed to set VT input mode, can't do anything here.
        return false;
    }

    isVirtual = true;
    return true;
}
}


/*
    Cast to C-string and use with printf()
*/
std::string setStyle(const std::string &str, const COLORS fgColor, const STYLES style, const COLORS bgColor)
{
    if (!isVirtual)
    {
        return str;
    }

    if (fgColor == COLORS::DEFAULT && style == STYLES::NONE && bgColor == COLORS::DEFAULT)
    {
        return str;
    }

    std::string styling = "\033[";
    if (style != STYLES::NONE)
    {
        styling += STYLES_MAP.at(style) + ";";
    }
    if (fgColor != COLORS::DEFAULT)
    {
        styling += FG_COLORS.at(fgColor) + ";";
    }
    if (bgColor != COLORS::DEFAULT)
    {
        styling += BG_COLORS.at(bgColor);
    }
    if (styling[styling.length()-1] == ';')
    {
        styling.pop_back();
    }
    styling += "m";

    return styling + str + RESET_STYLE;
}

void printError(const std::string &message, const bool bold)
{
    printf(setStyle(message, COLORS::RED, bold ? STYLES::BOLD : STYLES::NONE).c_str());
}

void printWarning(const std::string &message, const bool bold)
{
    printf(setStyle(message, COLORS::YELLOW, bold ? STYLES::BOLD : STYLES::NONE).c_str());
}

void printInfo(const std::string &message, const bool bold)
{
    printf(setStyle(message, COLORS::CYAN, bold ? STYLES::BOLD : STYLES::NONE).c_str());
}

void printSuccess(const std::string &message, const bool bold)
{
    printf(setStyle(message, COLORS::GREEN, bold ? STYLES::BOLD : STYLES::NONE).c_str());
}
