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


    // Check mod path
    if (argc < 2)
    {
        logger.error("Mod path must be provided");
        printUsage();
        return EXIT_FAILURE;
    }

    // Check texture
    if (argc < 3)
    {
        logger.error("Texture name must be provided");
        printUsage();
        return EXIT_FAILURE;
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

        if (i > 2 || strncmp(argv[i], "-", 1) == 0)
        {
            logger.error("Unknown argument '%s'", argv[i]);
            printUsage();
            return EXIT_FAILURE;
        }
    }

    if (!std::filesystem::is_directory(argv[1]))
    {
        logger.error("'%s' is not a directory", argv[1]);
        return EXIT_FAILURE;
    }
    options.modpath = unsteampipe(argv[1]);

    options.texture = argv[2];


    if (options.texture == "*")
    {
        std::cout << Styling::style(Styling::bold) << "'*' will match everything. Are you sure? (y/N) " << Styling::style();
        
        if (confirm_dialogue(false))
        {
            options.everything = true;
        }
        else
        {
            std::cout << "Exiting..." << std::endl;
            return EXIT_SUCCESS;
        }
    }

    return whichwad(options);
}
