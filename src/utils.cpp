#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>
#include "utils.h"


const char* c_STEAM_PIPES[] = { "_addon", "_hd", "_downloads" };
const char* c_WAD_SKIP_LIST[] = { "cached", "fonts", "gfx", "spraypaint", "tempdecal" };


void printUsage()
{
#ifdef _WIN32
    std::cout << "Usage: whichwad.exe MOD_PATH TEXTURE [OPTIONS]\n\n";
#else
    std::cout << "Usage: whichwad MOD_PATH TEXTURE [OPTIONS]\n\n";
#endif
    std::cout
        << Styling::bold << "REQUIRED ARGUMENTS" << Styling::reset << "\n"
        << " * MOD PATH\t\t(path)\t"
        << "path to the mod with the WAD files e.g. \".../steamapps/Half-Life/valve\"\n"
        << " * TEXTURE\t\t(text)\t"
        << "texture(s) to search for, use \";\" to delimit multiple textures\n\n"
        << Styling::bold << "OPTIONS" << Styling::reset << "\n"
        << "  --version\t-v\t\t"
        << "print application version\n"
        << "  --extract\t-e\t\t"
        << "extract the textures (8BPP BMP)\n"
        << "  --output\t-o\t(path)\t"
        << "output directory for extracted textures (default: extracted)\n"
        << "  --help\t-h\t\t"
        << "print this message and exit"
        << std::endl;
}

void exitError(std::string message, bool printHelp, int exitCode)
{
    Styling::printError("Error: " + message + "\n");
    if (printHelp) { printUsage(); }
    exit(EXIT_FAILURE);
}

bool confirm_dialogue(const bool yesDefault)
{
    static std::string buffer;
    std::getline(std::cin, buffer);

    if (buffer.empty()) { return yesDefault; }
    if (tolower(buffer.at(0)) == 'y') { return true; }
    return false;
}

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
    for (const char* pipe: c_STEAM_PIPES)
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

            for (const char* skip : c_WAD_SKIP_LIST)
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
    for (const char* pipe: c_STEAM_PIPES)
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

void Styling::printError(const std::string& message)
{
    std::cerr << error << message << reset << std::endl;
}

void Styling::printWarning(const std::string& message)
{
    std::cout << warning << message << reset << std::endl;
}

void Styling::printInfo(const std::string& message)
{
    std::cout << info << message << reset << std::endl;
}

void Styling::printSuccess(const std::string& message)
{
    std::cout << success << message << reset << std::endl;
}
