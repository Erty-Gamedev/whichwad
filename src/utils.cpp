#include <array>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>
#include <set>
#include "utils.h"
#include "logging.h"


const char* c_STEAM_PIPES[] = { "_addon", "_hd", "_downloads" };
const char* c_WAD_SKIP_LIST[] = { "cached", "fonts", "gfx", "spraypaint", "tempdecal" };


using namespace Styling;
static Logging::Logger& logger = Logging::Logger::getLogger("whichwad");


#ifdef _WIN32
#include <Windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif
static inline std::filesystem::path getExeDir()
{
#ifdef _WIN32
    std::vector<wchar_t> pathBuffer;
    DWORD copied = 0;
    do {
        pathBuffer.resize(pathBuffer.size() + MAX_PATH);
        copied = GetModuleFileNameW(NULL, &pathBuffer.at(0), static_cast<DWORD>(pathBuffer.size()));
    } while (copied >= pathBuffer.size());
    pathBuffer.resize(copied);
    std::wstring exeDir(pathBuffer.begin(), pathBuffer.end());
    return std::filesystem::path{ exeDir }.parent_path();
#else
    return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
}
static inline const std::filesystem::path c_exedir = getExeDir();
static inline const std::filesystem::path configFilePath = c_exedir / "whichwad.conf";
static inline std::unordered_map<std::string, std::string> g_configs;


static inline bool readConfigFile()
{
    std::ifstream file;
    file.open(configFilePath);
    if (!file.is_open() || !file.good())
    {
        file.close();
        return false;
    }

    std::string line, key, value;
    line.reserve(1024);
    int lineNumber = 0;
    while (std::getline(file, line))
    {
        ++lineNumber;

        // Skip comments and empty lines
        if (line.starts_with("//") || line.starts_with(";") || line.starts_with("#") || line.empty())
            continue;

        const std::vector<std::string>& parts = splitString(line, '=');
        if (parts.empty())
            continue;

        key = parts.at(0);
        trim(key);

        if (parts.size() > 1) {
            value = parts.at(1);
            trim(value);
        } else { value = ""; }

        g_configs.insert_or_assign(key, value);
    }

    file.close();
    return true;
}

static inline bool saveConfigFile()
{
    std::ofstream file;
    file.open(configFilePath);
    if (!file.is_open() || !file.good())
    {
        file.close();
        return false;
    }

    for (auto& kv : g_configs)
        file << kv.first << "=" << kv.second << "\n";

    file.close();
    return true;
}

void printUsage()
{
#ifdef _WIN32
    std::cout << "Usage: whichwad.exe TEXTURE[...] [OPTIONS]\n\n";
#else
    std::cout << "Usage: whichwad TEXTURE[...] [OPTIONS]\n\n";
#endif
    std::cout
        << style(bold) << "REQUIRED ARGUMENTS" << style() << "\n"
        << " * TEXTURE         (text)    texture(s) to search for\n\n"
        << style(bold) << "OPTIONS" << style() << "\n"
        << " --help      -h              print this message and exit\n"
        << " --version   -v              print application version\n"
        << " --extract   -e              extract the textures (8BPP BMP)\n"
        << " --output    -o    (path)    output directory for extracted textures (default: extracted)\n"
        << " --steamdir  -s    (path)    Steam directory to use for this search\n"
        << " --mod       -m    (text)    check this mod only\n"
        << std::endl;
}

bool confirm_dialogue(const bool yesDefault)
{
    static std::string buffer;
    std::getline(std::cin, buffer);

    if (buffer.empty()) { return yesDefault; }
    if (tolower(buffer.at(0)) == 'y') { return true; }
    return false;
}

static inline const char* c_defaultSteamDir = "C:/Program Files (x86)/Steam";
std::filesystem::path getSteamDir()
{
    readConfigFile();

    if (g_configs.contains("steamdir"))
    {
        if (std::filesystem::is_directory(g_configs["steamdir"]))
            return g_configs["steamdir"];
        logger.warning("\"%s\" is not a directory", g_configs["steamdir"]);
    }

    if (std::filesystem::is_directory(c_defaultSteamDir))
    {
        std::cout << "Is " << c_defaultSteamDir << " your Steam directory ? (Y / n) ";
        if (confirm_dialogue(true))
        {
            g_configs.insert_or_assign("steamdir", c_defaultSteamDir);
            saveConfigFile();
            return c_defaultSteamDir;
        }
    }


    std::string buffer;
    for (int i = 0; i < 3; ++i)
    {
        std::cout << "Enter path to Steam directory:  ";

        std::getline(std::cin, buffer);
        if (std::filesystem::is_directory(buffer))
        {
            g_configs.insert_or_assign("steamdir", buffer);
            saveConfigFile();
            return buffer;
        }

        std::cout << style(warning) << "\"" << buffer << "\" is not a directory" << style() << std::endl;
    }

    logger.error("Could not set Steam directory");
    exit(EXIT_FAILURE);
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
    if (str.empty())
        return str;

    for (const auto& steampipe : c_SteamPipes)
    {
        if (const size_t matchPosition = str.rfind(steampipe); matchPosition != std::string::npos)
        {
            str.replace(matchPosition, steampipe.length(), "");
            return str;
        }
    }
    return str;
}

std::vector<std::string> splitString(const std::string& str, const char delimiter)
{
    std::istringstream strStream{ str };
    std::vector<std::string> segments;
    std::string segment;
    while (std::getline(strStream, segment, delimiter))
    {
        segments.push_back(segment);
    }
    return segments;
}

void trim(std::string& str, const char* trim)
{
    str.erase(0, str.find_first_not_of(trim));
    str.erase(str.find_last_not_of(trim) + 1);
}

bool wildcardCompare(const std::string& search, const std::string& haystack)
{
    if (search == "*") return true;

    if (search.length() > (haystack.length() + 1))
        return false;

    size_t wildcardPosition = search.find('*');

    if (wildcardPosition == 0)
    {
        size_t searchLength = search.length() - 1;
        return haystack.compare(haystack.length() - searchLength, searchLength, search.substr(1, searchLength)) == 0;
    }

    if (wildcardPosition != std::string::npos)
        return haystack.compare(0, wildcardPosition, search.substr(0, wildcardPosition)) == 0;

    return search == haystack;
}

void printSuccess(const std::string& message)
{
    std::cout << style(success) << message << style() << std::endl;
}
