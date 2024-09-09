#pragma pack(1)
#include <iostream>
#include "utils.h"
#include "consolestyling.h"

int _CRT_glob = 0;

const char* NAME{ "WhichWAD" };
const char* VERSION{ "1.0.0" };

// CLI arguments and options
static std::string modpath;
static std::string texture;
static std::string outputDir = "extracted";
static bool extract = false;
static bool everything = false;


static void printVersion() {std::cout << NAME << " v" << VERSION << std::endl; }

static void printUsage()
{
    std::cout << "Usage: whichwad.exe MOD_PATH TEXTURE [OPTIONS]\n\n"
        << setStyle("REQUIRED ARGUMENTS\n", COLORS::DEFAULT, STYLES::BOLD)
        << " * MOD PATH\t\t(path)\t"
            << "path to the mod with the WAD files e.g. \".../steamapps/Half-Life/valve\"\n"
        << " * TEXTURE\t\t(text)\t"
            << "texture(s) to search for, use \";\" to delimit multiple textures\n\n"
        << setStyle("OPTIONS\n", COLORS::DEFAULT, STYLES::BOLD)
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

/*
    Prints an error message and exits
*/
static void exitError(std::string message, bool printHelp = true, int exitCode = EXIT_FAILURE)
{
    printError("Error: " + message + "\n");
    if (printHelp) {printUsage();}
    exit(EXIT_FAILURE);
}

static bool confirm_dialogue(const bool yesDefault = true)
{
    static std::string buffer;
    std::getline(std::cin, buffer);

    if (buffer.empty()) { return yesDefault; }
    if (tolower(buffer.at(0)) == 'y') { return true; }
    return false;
}


int main(int argc, char* argv[])
{
    //return test();
    CS::init();
    
    for (int count = 1; count < argc; ++count)
    {
        if (strcmp(argv[count], "--version") == 0 || strcmp(argv[count], "-v") == 0)
        {
            printVersion();
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[count], "--help") == 0 || strcmp(argv[count], "-h") == 0)
        {
            printUsage();
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[count], "--extract") == 0 || strcmp(argv[count], "-e") == 0)
        {
            extract = true;
            continue;
        }
        if (strcmp(argv[count], "--output") == 0 || strcmp(argv[count], "-o") == 0)
        {
            if ((count + 1) > argc || !(argv[count + 1])
                || strncmp(argv[count + 1], "-", 1) == 0)
            {
                exitError("Missing directory parameter for " + std::string(argv[count])
                    + " argument (use \".\" for current directory)", false);
            }
            count++;
            outputDir = argv[count];
            continue;
        }

        if (count > 2 || strncmp(argv[count], "-", 1) == 0)
        {
            exitError("Unknown argument '" + std::string(argv[count]) + "'");
        }
    }

    // Check MOD PATH
    if (argc < 2)
    {
        printUsage();
        exit(EXIT_FAILURE);
    }
    modpath = argv[1];
    if (!std::filesystem::is_directory(modpath))
    {
        exitError("'" + std::string(modpath) + "' is not a directory");
    }
    modpath = unsteampipe(modpath);

    // Check TEXTURE
    if (argc < 3) {
        exitError("Texture name must be provided");
    }
    texture = argv[2];


    if (texture == std::string("*"))
    {
        printWarning("'*' will match everything. Are you sure? (y/N) ");
        
        if (confirm_dialogue(false))
        {
            everything = true;
        }
        else
        {
            std::cout << "Exiting..." << std::endl;
            return EXIT_SUCCESS;
        }
    }

    std::vector<std::filesystem::path> globs = findWadFilesPipes(modpath);
    std::vector<std::string> textures = splitString(std::stringstream(texture), ';');
    foundMap matching_wads;
    matchTextureMap matches;
    readerPathMap readers;

    for (std::string const& tex : textures)
    {
        matches = findTextureInWad(globs, tex, readers);

        if (matches.size() == 0)
        {
            std::cout << setStyle("No texture names matching ", COLORS::RED, STYLES::BOLD)
                << setStyle(tex, COLORS::CYAN, STYLES::BOLD)
                << setStyle(" not found in any WAD in ", COLORS::RED, STYLES::BOLD)
                << setStyle(modpath, COLORS::RED) << "\n";
            continue;
        }

        matching_wads[tex] = matches;

        std::cout << setStyle(std::to_string(matches.size()), COLORS::GREEN, STYLES::BOLD)
            << setStyle(" texture names matching ", COLORS::CYAN)
            << setStyle(tex, COLORS::MAGENTA)
            << setStyle(" found:\n", COLORS::CYAN);

        if (everything) { continue; }
        
        for (std::pair<const std::string, std::vector<std::reference_wrapper<Wad3Reader*>>> const& kv : matches)
        {
            std::cout << setStyle("\t" + toUpperCase(kv.first), COLORS::YELLOW)
                << " found in " << kv.second.size() << " WADS:\n";

            for (const Wad3Reader* reader : kv.second)
            {
                std::cout << setStyle("\t" + (*reader).filepath + "\n", COLORS::BRIGHT_BLACK);
            }
        }
    }

    if (!extract || matching_wads.size() == 0)
    {
        return EXIT_SUCCESS;
    }

    std::cout << "\n";

    // Check if output dir exists, or create it
    std::filesystem::path outputPath{outputDir};
    if (!std::filesystem::is_directory(outputPath))
    {
        printWarning(std::filesystem::absolute(outputPath).string() + " does not exist. Create it? (Y/n) ");

        if (!confirm_dialogue(true))
        {
            std::cout << "Output dir not created, aborted\n";
            return EXIT_FAILURE;
        }

        if (std::filesystem::create_directories(outputPath))
        {
            printInfo(std::filesystem::absolute(outputPath).string() + "created\n");
        }
        else
        {
            exitError("Could not create path '"
                + std::filesystem::absolute(outputPath).string() + "'");
        }
    }

    std::string outputFile;
    bool chosenMultiWad;
    for (std::pair<const std::string, matchTextureMap> const& kv : matching_wads)
    {
        for (std::pair<const std::string, std::vector<std::reference_wrapper<Wad3Reader*>>> match_readers : kv.second)
        {
            outputFile = (outputPath / match_readers.first).string() + ".bmp";

            if (match_readers.second.size() == 1)
            {
                Wad3Reader* reader = match_readers.second[0];

                std::cout << setStyle("Saving texture from "
                    + std::filesystem::path{(*reader).filepath }.filename().string() + " to ", COLORS::CYAN)
                    << setStyle(outputFile, COLORS::CYAN, STYLES::BOLD) << "\n";

                SaveTexture(
                    (*reader).textures.at(toLowerCase(match_readers.first)),
                    outputFile.c_str()
                );

                continue;
            }

            std::cout << setStyle(toUpperCase(match_readers.first))
                << setStyle(" found in " + std::to_string(match_readers.second.size())
                            + " WADs. It's time to choose:\n", COLORS::GREEN);

            chosenMultiWad = false;
            for (const Wad3Reader* reader : match_readers.second)
            {
                std::string readerFilename = std::filesystem::path{(*reader).filepath }.filename().string();
                std::cout << "Extract from " + readerFilename + "? (Y/n) ";

                if (confirm_dialogue(true))
                {
                    chosenMultiWad = true;
                    std::cout << setStyle("Saving texture from " + readerFilename + " to ", COLORS::CYAN)
                        << setStyle(outputFile, COLORS::CYAN, STYLES::BOLD) << "\n";

                    SaveTexture(
                        (*reader).textures.at(toLowerCase(match_readers.first)),
                        outputFile.c_str()
                    );

                    break;
                }
            }

            if (!chosenMultiWad)
            {
                std::cout << setStyle(toUpperCase(match_readers.first) + " was not extracted.", COLORS::YELLOW) << std::endl;
            }
        }
    }

    return EXIT_SUCCESS;
}
