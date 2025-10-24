#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "wad3.h"


using wadPathMap = std::unordered_map<std::string, std::vector<WAD3::Wad3Reader*>>;
using wadReaderMap = std::unordered_map<std::filesystem::path, std::unique_ptr<WAD3::Wad3Reader>>;

struct Options
{
    bool extract = false;
    bool everything = false;
    std::string outputDir = "extracted";
    std::string mod = "";
    std::vector<std::string> textures;
    std::filesystem::path steamDir;
};


wadPathMap findTextureInWads(
	const std::set<std::filesystem::path>& globs, const std::string& filter, wadReaderMap& readers
);

int whichwad(const Options& options);
