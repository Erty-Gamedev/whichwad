#pragma pack(1)
#include <string>
#include <algorithm>
#include <cctype>
#include "utils.h"


const char* STEAM_PIPES[] = {"_addon", "_hd", "_downloads"};
const char* WAD_SKIP_LIST[] = {"cached", "fonts", "gfx", "spraypaint", "tempdecal"};


std::string toLowerCase(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char c) {
        return std::tolower(c);
    });
    return str;
}

std::string toUpperCase(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char c) {
        return std::toupper(c);
    });
    return str;
}

std::string unsteampipe(std::string str)
{
    for (const char* pipe: STEAM_PIPES)
    {
        size_t matchPosition = str.find(pipe);
        if (matchPosition != std::string::npos)
        {
            str.replace(matchPosition, std::string(pipe).length(), "");
            return str;
        }
    }
    return str;
}

void findWadFiles(std::filesystem::path modpath, std::vector<std::filesystem::path> &globs)
{
    std::filesystem::path filepath;
    std::string filestem;
    bool shouldSkip;

    for (auto const& file: std::filesystem::directory_iterator(modpath))
    {
        filepath = file.path();

        if (filepath.extension() == ".wad")
        {
            filestem = toLowerCase(filepath.stem().string());
            shouldSkip = false;

            for (const char* skip : WAD_SKIP_LIST)
            {
                if (strcmp(skip, (char*)filestem.c_str()) == 0)
                {
                    shouldSkip = true;
                    break;
                }
            }

            if (!shouldSkip) {globs.push_back(filepath);}
        }
    }
}

std::vector<std::filesystem::path> findWadFilesPipes(std::filesystem::path modpath)
{
    std::vector<std::filesystem::path> globs;
    std::filesystem::path steampiped;

    // Gather WAD files from _addon, _hd and _downloads (if they exist)
    for (const char* pipe: STEAM_PIPES)
    {
        steampiped = std::filesystem::path(modpath.string() + pipe);
        if (std::filesystem::is_directory(steampiped))
        {
            findWadFiles(std::filesystem::path(modpath.string() + pipe), globs);
        }
    }
    
    // Gather WAD files from main folder
    findWadFiles(modpath, globs);

    return globs;
}

std::vector<std::string> splitString(std::stringstream str, char delimiter = ' ')
{
    std::vector<std::string> result;
    std::string temp;
    while (std::getline(str, temp, delimiter))
    {
        result.push_back(temp);
    }
    return result;
}

bool wildcardCompare(std::string search, std::string haystack)
{
    if (search[0] == '*') {return true;}

    size_t wildcardPosition = search.find('*');
    if (wildcardPosition != std::string::npos)
    {
        return haystack.compare(0, wildcardPosition, search.substr(0, wildcardPosition)) == 0;
    }
    else
    {
        return search == haystack;
    }
}

std::vector<std::string> filterTextureMap(Wad3Reader* reader, std::string tex)
{
    std::vector<std::string> matches;
    for (std::pair<const std::string, MMData*> const& kv : (*reader).textures)
    {
        if (wildcardCompare(tex, kv.first))
        {
            matches.push_back(kv.first);
        }
    }
    return matches;
}

matchTextureMap findTextureInWad(
    std::vector<path> globs, std::string tex, readerPathMap &readers)
{
    matchTextureMap matchMap;

    for (path const& glob : globs)
    {
        if (readers.count(glob) == 0)
        {
            readers[glob] = ReadWad(glob.string().c_str());
        }

        std::vector<std::string> matches = filterTextureMap(readers[glob], tex);

        for (std::string const& match : matches)
        {
            if (matchMap.count(match) == 0)
            {
                matchMap[match] = std::vector<std::reference_wrapper<Wad3Reader*>>{};
            }
            matchMap[match].push_back(readers[glob]);
        }
    }

    return matchMap;
}
