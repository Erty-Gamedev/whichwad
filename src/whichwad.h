#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "wad3.h"


using wadPathMap = std::unordered_map<std::string, std::vector<std::shared_ptr<WAD3::Wad3Reader>>>;
using wadReaderMap = std::unordered_map<std::filesystem::path, std::shared_ptr<WAD3::Wad3Reader>>;

struct Options
{
    bool extract = false;
    bool everything = false;
    std::string modpath = "";
    std::string texture = "";
    std::string outputDir = "extracted";
};


wadPathMap findTextureInWads(
	const std::vector<std::filesystem::path>& globs, const std::string& filter, wadReaderMap& readers
);

int whichwad(Options options);
