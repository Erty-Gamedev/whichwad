#ifndef WHICHWAD_CONSOLESTYLING_H
#define WHICHWAD_CONSOLESTYLING_H

#include <string>
#include <unordered_map>

enum STYLES
{
    NONE,
    BOLD,
    DIM,
    ITALIC,
    UNDERLINE,
    BLINKING,
    REVERSE,
    HIDDEN,
    STRIKETHROUGH,
};
enum COLORS
{
    DEFAULT,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
};


const std::unordered_map<COLORS, std::string> FG_COLORS{
    {COLORS::DEFAULT,         "0"},
    {COLORS::BLACK,          "30"},
    {COLORS::RED,            "31"},
    {COLORS::GREEN,          "32"},
    {COLORS::YELLOW,         "33"},
    {COLORS::BLUE,           "34"},
    {COLORS::MAGENTA,        "35"},
    {COLORS::CYAN,           "36"},
    {COLORS::WHITE,          "37"},
    {COLORS::BRIGHT_BLACK,   "90"},
    {COLORS::BRIGHT_RED,     "91"},
    {COLORS::BRIGHT_GREEN,   "92"},
    {COLORS::BRIGHT_YELLOW,  "93"},
    {COLORS::BRIGHT_BLUE,    "94"},
    {COLORS::BRIGHT_MAGENTA, "95"},
    {COLORS::BRIGHT_CYAN,    "96"},
    {COLORS::BRIGHT_WHITE,   "97"}
};
const std::unordered_map<COLORS, std::string> BG_COLORS{
    {COLORS::DEFAULT,         "0"},
    {COLORS::BLACK,          "40"},
    {COLORS::RED,            "41"},
    {COLORS::GREEN,          "42"},
    {COLORS::YELLOW,         "43"},
    {COLORS::BLUE,           "44"},
    {COLORS::MAGENTA,        "45"},
    {COLORS::CYAN,           "46"},
    {COLORS::WHITE,          "47"},
    {COLORS::BRIGHT_BLACK,   "100"},
    {COLORS::BRIGHT_RED,     "101"},
    {COLORS::BRIGHT_GREEN,   "102"},
    {COLORS::BRIGHT_YELLOW,  "103"},
    {COLORS::BRIGHT_BLUE,    "104"},
    {COLORS::BRIGHT_MAGENTA, "105"},
    {COLORS::BRIGHT_CYAN,    "106"},
    {COLORS::BRIGHT_WHITE,   "107"}
};
const std::unordered_map<STYLES, std::string> STYLES_MAP{
    {STYLES::NONE,          "0"},
    {STYLES::BOLD,          "1"},
    {STYLES::DIM,           "2"},
    {STYLES::ITALIC,        "3"},
    {STYLES::UNDERLINE,     "4"},
    {STYLES::BLINKING,      "5"},
    {STYLES::REVERSE,       "7"},
    {STYLES::HIDDEN,        "8"},
    {STYLES::STRIKETHROUGH, "9"}
};

const std::string RESET_STYLE = "\033[0m";

namespace CS
{
bool init();
}

std::string setStyle(
    const std::string &str,
    const COLORS fgColor = COLORS::DEFAULT,
    const STYLES style = STYLES::NONE,
    const COLORS bgColor= COLORS::DEFAULT
);
void printError(const std::string &message, const bool bold = true);
void printWarning(const std::string &message, const bool bold = false);
void printInfo(const std::string &message, const bool bold = false);
void printSuccess(const std::string &message, const bool bold = true);

#endif
