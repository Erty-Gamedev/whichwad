#pragma once

#include <sstream>
#include <cstring>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <wchar.h>
#include <windows.h>
#endif


void printUsage();
void exitError(std::string message, bool printHelp = true, int exitCode = EXIT_FAILURE);
bool confirm_dialogue(const bool yesDefault = true);

std::string toLowerCase(std::string str);
std::string toUpperCase(std::string str);
std::string unsteampipe(std::string str);
void findWadFiles(std::filesystem::path modpath, std::vector<std::filesystem::path> &globs);
std::vector<std::filesystem::path> findWadFilesPipes(std::filesystem::path modpath);
std::vector<std::string> splitString(std::stringstream str, char delimiter);
bool wildcardCompare(std::string search, std::string haystack);


/**
 * Check if we can enable virtual terminal (needed for ANSI escape sequences)
 * From: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-enabling-virtual-terminal-processing
 */
static inline bool enableVirtualTerminal()
{
#ifdef _WIN32
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
#endif
    return true;
}


namespace Styling
{
    static inline const char* reset = "\033[0m";

    static inline const char* fgBlack = "\033[30m";
    static inline const char* fgRed = "\033[31m";
    static inline const char* fgGreen = "\033[32m";
    static inline const char* fgYellow = "\033[33m";
    static inline const char* fgBlue = "\033[34m";
    static inline const char* fgMagenta = "\033[35m";
    static inline const char* fgCyan = "\033[36m";
    static inline const char* fgWhite = "\033[37m";

    static inline const char* fgBrightBlack = "\033[90m";
    static inline const char* fgBrightRed = "\033[91m";
    static inline const char* fgBrightGreen = "\033[92m";
    static inline const char* fgBrightYellow = "\033[93m";
    static inline const char* fgBrightBlue = "\033[94m";
    static inline const char* fgBrightMagenta = "\033[95m";
    static inline const char* fgBrightCyan = "\033[96m";
    static inline const char* fgBrightWhite = "\033[97m";

    static inline const char* bold = "\033[1m"; // works
    static inline const char* dim = "\033[2m";
    static inline const char* italic = "\033[3m";
    static inline const char* underline = "\033[4m"; // works
    static inline const char* blinking = "\033[5m";
    static inline const char* reverse = "\033[6m";
    static inline const char* hidden = "\033[8m";
    static inline const char* strikethrough = "\033[8m";

    static inline const char* info = fgCyan;
    static inline const char* success = "\033[1;32m";
    static inline const char* warning = fgYellow;
    static inline const char* error = "\033[1;31m";


    static bool g_isVirtual = enableVirtualTerminal();


    void printError(const std::string& message);
    void printWarning(const std::string& message);
    void printInfo(const std::string& message);
    void printSuccess(const std::string& message);
}
