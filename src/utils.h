#pragma once

#include <filesystem>
#include <vector>
#include <array>


static inline const std::array<std::string, 3> c_SteamPipes{
    "_addon", "_hd", "_downloads"
};
static inline const std::array<std::string, 5> c_WadSkipList{
    "cached", "fonts", "gfx", "spraypaint", "tempdecal"
};


void printUsage();
bool confirm_dialogue(bool yesDefault = true);
std::filesystem::path getSteamDir();

std::string toLowerCase(std::string str);
std::string toUpperCase(std::string str);
std::string unsteampipe(std::string str);
std::vector<std::string> splitString(const std::string& str, char delimiter = ' ');
void trim(std::string& str, const char* trim = " \t\n\r");

bool wildcardCompare(const std::string& search, const std::string& haystack);

void printSuccess(const std::string& message);
