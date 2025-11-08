#pragma once

#include <cstring>
#include <filesystem>
#include <vector>
#include <set>
#include <unordered_map>

#ifdef _WIN32
#include <wchar.h>
#include <windows.h>
#endif


void printUsage();
bool confirm_dialogue(const bool yesDefault = true);
std::filesystem::path getSteamDir();

std::string toLowerCase(std::string str);
std::string toUpperCase(std::string str);
std::string unsteampipe(std::string str);
void findWadFiles(std::filesystem::path modpath, std::set<std::filesystem::path>& globs);
void findWadFilesPipes(std::filesystem::path modpath, std::set<std::filesystem::path>& globs);
std::vector<std::string> splitString(const std::string& str, const char delimiter = ' ');
void trim(std::string& str, const char* trim = " \t\n\r");

bool wildcardCompare(const std::string& search, const std::string& haystack);

void printSuccess(const std::string& message);
