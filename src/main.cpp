#include <iostream>
#include "utils.h"
#include "whichwad.h"

int _CRT_glob = 0;

#ifndef WHICHWAD_NAME_VERSION
#define WHICHWAD_NAME_VERSION="Which Wad v0.0.0"
#endif
static void printVersion() { std::cout << WHICHWAD_NAME_VERSION << std::endl; }


int main(int argc, char** argv)
{
    // Eager args
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            printVersion();
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
        exitError("Mod path must be provided");
    // Check texture
    if (argc < 3)
        exitError("Texture name must be provided");

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
            exitError("Missing directory parameter for " + std::string(argv[i - 1])
                + " argument (use \".\" for current directory)", false);
        }

        if (i > 2 || strncmp(argv[i], "-", 1) == 0)
            exitError("Unknown argument '" + std::string(argv[i]) + "'");
    }

    if (!std::filesystem::is_directory(argv[1]))
        exitError("'" + std::string(argv[1]) + "' is not a directory");
    options.modpath = unsteampipe(argv[1]);

    options.texture = argv[2];


    if (options.texture == "*")
    {
        Styling::printWarning("'*' will match everything. Are you sure? (y/N) ");
        
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
