#include <iostream>
#include "utils.h"
#include "whichwad.h"
#include "logging.h"

int _CRT_glob = 0;

#ifndef WHICHWAD_NAME_VERSION
#define WHICHWAD_NAME_VERSION="Which Wad v0.0.0"
#endif

static Logging::Logger& logger = Logging::Logger::getLogger("whichwad");


int main(int argc, char** argv)
{
    logger.setFileHandler(nullptr);

    // Eager args
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            logger.log(WHICHWAD_NAME_VERSION);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printUsage();
            return EXIT_SUCCESS;
        }
    }

    Options options{};

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--extract") == 0 || strcmp(argv[i], "-e") == 0)
        {
            options.extract = true;
            continue;
        }
        if (strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0)
        {
            ++i;
            if (i < argc)
            {
                options.outputDir = argv[i];
                continue;
            }

            logger.error("Missing directory parameter for %s argument (use \".\" for current directory)", argv[i - 1]);
            return EXIT_FAILURE;
        }
        if (strcmp(argv[i], "--steamdir") == 0 || strcmp(argv[i], "-s") == 0)
        {
            ++i;
            if (i < argc)
            {
                if (std::filesystem::is_directory(argv[i]))
                {
                    options.steamDir = argv[i];
                    continue;
                }
                logger.error("%s was not a directory", argv[i]);
                return EXIT_FAILURE;
            }

            logger.error("Missing directory parameter for %s argument", argv[i - 1]);
            return EXIT_FAILURE;
        }
        if (strcmp(argv[i], "--mod") == 0 || strcmp(argv[i], "-m") == 0)
        {
            ++i;
            if (i < argc)
            {
                options.mod = argv[i];
                continue;
            }

            logger.error("Missing mod parameter for %s argument", argv[i - 1]);
            return EXIT_FAILURE;
        }

        if (strncmp(argv[i], "-", 1) == 0)
        {
            logger.error("Unknown argument '%s'", argv[i]);
            printUsage();
            return EXIT_FAILURE;
        }

        options.textures.emplace_back(argv[i]);
    }

    if (options.textures.empty())
    {
        logger.error("Texture name(s) must be provided");
        printUsage();
        return EXIT_FAILURE;
    }

    for (const std::string& texture : options.textures)
    {
        if (texture == "*")
        {
            std::cout << Styling::style(Styling::bold) << "'*' will match everything. Are you sure? (y/N) " << Styling::style();

            if (!confirm_dialogue(false))
            {
                std::cout << "Exiting..." << std::endl;
                return EXIT_SUCCESS;
            }

            options.everything = true;
            break;
        }
    }

    if (options.steamDir.empty())
        options.steamDir = getSteamDir();

    return whichwad(options);
}
