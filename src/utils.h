#pragma pack(1)
#ifndef WHICHWAD_UTILS_H
#define WHICHWAD_UTILS_H

#include <sstream>
#include <cstring>
#include <filesystem>
#include "wad3reader.h"


typedef std::filesystem::path path;
typedef std::unordered_map<std::filesystem::path, Wad3Reader*> readerPathMap;

std::string toLowerCase(std::string str);
std::string toUpperCase(std::string str);
std::string unsteampipe(std::string str);
void findWadFiles(path modpath, std::vector<path> &globs);
std::vector<path> findWadFilesPipes(path modpath);
std::vector<std::string> splitString(std::stringstream str, char delimiter);
bool wildcardCompare(std::string search, std::string haystack);
std::vector<std::string> filterTextureMap(Wad3Reader* reader, std::string tex);
matchTextureMap findTextureInWad(
    std::vector<path> globs, std::string tex, readerPathMap &readers);

#endif
