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
bool confirm_dialogue(const bool yesDefault = true);

std::string toLowerCase(std::string str);
std::string toUpperCase(std::string str);
std::string unsteampipe(std::string str);
void findWadFiles(std::filesystem::path modpath, std::vector<std::filesystem::path> &globs);
std::vector<std::filesystem::path> findWadFilesPipes(std::filesystem::path modpath);
std::vector<std::string> splitString(std::stringstream str, char delimiter);
bool wildcardCompare(std::string search, std::string haystack);

void printSuccess(const std::string& message);
