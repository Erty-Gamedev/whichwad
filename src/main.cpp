#include <iostream>
#include "utils.h"
#include "whichwad.h"
#include "logging.h"

int _CRT_glob = 0;

#ifndef WHICHWAD_NAME_VERSION
#define WHICHWAD_NAME_VERSION "Which Wad v0.0.0"
#endif

static Logging::Logger& logger = Logging::Logger::getLogger("whichwad");
using namespace Styling;


static void handleArgs(const int argc, char* argv[])
{
    // Eager args
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            logger.log(WHICHWAD_NAME_VERSION);
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printUsage();
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--extract") == 0 || strcmp(argv[i], "-e") == 0)
        {
            g_options.extract = true;
            continue;
        }
        if (strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0)
        {
            ++i;
            if (i < argc)
            {
                g_options.outputDir = argv[i];
                continue;
            }

            logger.error("Missing directory parameter for %s argument (use \".\" for current directory)", argv[i - 1]);
            exit(EXIT_FAILURE);
        }
        if (strcmp(argv[i], "--steamdir") == 0 || strcmp(argv[i], "-s") == 0)
        {
            ++i;
            if (i < argc)
            {
                if (std::filesystem::is_directory(argv[i]))
                {
                    g_options.steamDir = argv[i];
                    continue;
                }
                logger.error("%s was not a directory", argv[i]);
                exit(EXIT_FAILURE);
            }

            logger.error("Missing directory parameter for %s argument", argv[i - 1]);
            exit(EXIT_FAILURE);
        }
        if (strcmp(argv[i], "--mod") == 0 || strcmp(argv[i], "-m") == 0)
        {
            ++i;
            if (i < argc)
            {
                g_options.mods.emplace_back(unsteampipe(argv[i]));
                continue;
            }

            logger.error("Missing mod parameter for %s argument", argv[i - 1]);
            exit(EXIT_FAILURE);
        }
        if (strcmp(argv[i], "--bsp") == 0 || strcmp(argv[i], "-b") == 0)
        {
            g_options.bsp = true;
            continue;
        }

        if (strncmp(argv[i], "-", 1) == 0)
        {
            logger.error("Unknown argument '%s'", argv[i]);
            printUsage();
            exit(EXIT_FAILURE);
        }

        if (strcmp(argv[i], "*") == 0)
        {
            std::cout << Styling::style(Styling::bold) << "'*' will match everything. Are you sure? (y/N) " << Styling::style();

            if (!confirm_dialogue(false))
            {
                std::cout << "Exiting..." << std::endl;
                exit(EXIT_SUCCESS);
            }

            g_options.everything = true;
        }

        g_options.tests.emplace_back(argv[i]);
    }

    if (g_options.tests.empty())
    {
        logger.error("Texture name(s) must be provided");
        printUsage();
        exit(EXIT_FAILURE);
    }

    if (!g_options.outputDir.empty())
        g_options.outputDir.make_preferred();

    if (g_options.steamDir.empty())
        g_options.steamDir = getSteamDir();

    if (std::filesystem::is_directory(g_options.steamDir / "steamapps/common"))
        g_options.steamCommonDir = g_options.steamDir / "steamapps/common";
}



int main(int argc, char** argv)
{
    logger.setFileHandler(nullptr);

    handleArgs(argc, argv);

    g_options.findGlobs();

    const char* ext = g_options.bsp ? "BSP" : "WAD";

    logger.debug("Found %i %s files", g_options.globs.size(), ext);

    g_options.checkGlobs();


    std::unordered_map<std::string, std::set<std::filesystem::path>> foundTextures;

    for (const auto& test : g_options.tests)
    {
        if (test.matches.empty())
        {
            std::cout << style(error) << "No texture names matching " << style()
                << style(info | bold) << test.filter << style()
                << style(error) << " not found in any " << ext << " in the search path" << style() << std::endl;
            continue;
        }

        if (g_options.everything)
        {
            std::cout << style(info | bold) << test.matches.size() << style()
                << style(info) << " textures found" << std::endl;
            continue;
        }

        std::cout << style(info | bold) << test.matches.size()
            << style(info) << " texture names matching "
            << style(info | bold) << test.filter
            << style(info) << " found:" << style() << std::endl;

        for (const auto& [textureName, globs] : test.matches)
        {
            std::cout << style(warning) << "  " << toUpperCase(textureName) << style()
                << " found in " << globs.size() << " " << ext << "s:" << std::endl;

            for (const auto& glob : globs)
            {
                std::cout << style(brightBlack) << "    " << glob.string() << std::endl;
                foundTextures[textureName].insert(glob);
            }
        }
    }


    if (!g_options.extract || g_options.foundMatches == 0)
        return EXIT_SUCCESS;
    std::cout << std::endl;

    // Check if output dir exists, or create it
    if (!std::filesystem::is_directory(g_options.outputDir))
    {
        std::cout << style(warning)
            << std::filesystem::absolute(g_options.outputDir).string() + " does not exist. Create it? (Y/n) "
            << style();

        if (!confirm_dialogue(true))
        {
            std::cout << "Output dir not created, aborted\n";
            return EXIT_SUCCESS;
        }

        if (std::filesystem::create_directories(g_options.outputDir))
            printSuccess(std::filesystem::absolute(g_options.outputDir).string() + " created\n");
        else
        {
            logger.error("Could not create directory '%s'", std::filesystem::absolute(g_options.outputDir).string());
            return EXIT_FAILURE;
        }
    }

    std::string outputFile;
    for (const auto& [textureName, globs] : foundTextures)
    {
        outputFile = (g_options.outputDir / textureName).string() + ".bmp";

        if (globs.size() == 1)
        {
            std::unique_ptr<BaseReader> reader;
            auto& glob = *globs.begin();

            if (g_options.bsp)
                reader = std::make_unique<BspReader>(glob);
            else
                reader = std::make_unique<Wad3Reader>(glob);

            std::cout << style(info) << "Saving texture from "
                << glob.filename().string() << " to " << style()
                << style(info | bold) << outputFile << style() << std::endl;

            if (!reader->extract(textureName, g_options.outputDir))
                std::cout << style(warning) << "Could not save " << outputFile << std::endl;
            continue;
        }


        std::cout << toUpperCase(textureName) << style(green)
            << " found in " << globs.size() << " " << ext
            << "s. It's time to choose:" << style() << std::endl;

        bool chosenMultiWad = false;
        for (const auto& glob : globs)
        {
            const std::string& readerFilename = glob.filename().string();
            std::cout << "Extract from " + readerFilename + "? (Y/n) ";

            if (confirm_dialogue(true))
            {
                chosenMultiWad = true;
                std::unique_ptr<BaseReader> reader;

                if (g_options.bsp)
                    reader = std::make_unique<BspReader>(glob);
                else
                    reader = std::make_unique<Wad3Reader>(glob);

                std::cout << style(info) << "Saving texture from " << readerFilename << " to " << style()
                    << style(info | bold) << outputFile << style() << std::endl;

                if (!reader->extract(textureName, g_options.outputDir))
                    std::cout << style(warning) << "Could not save " << outputFile << std::endl;
                break;
            }
        }

        if (!chosenMultiWad)
            std::cout << style(warning) << toUpperCase(textureName) << " was not extracted" << style() << std::endl;
    }
}
