#include <iostream>
#include "utils.h"
#include "whichwad.h"
#include "logging.h"

using namespace WAD3;
using namespace Styling;

static Logging::Logger& logger = Logging::Logger::getLogger("whichwad");



static inline std::vector<std::string> filterTextureMap(const Wad3Reader& reader, std::string filter)
{
    filter = toLowerCase(filter);

	std::vector<std::string> matches;
    for (auto& dirEntry : reader.m_dirEntries)
    {
        std::string entryName = toLowerCase(dirEntry.szName);
		if (wildcardCompare(filter, entryName))
			matches.emplace_back(dirEntry.szName);
    }
	return matches;
}

wadPathMap findTextureInWads(
	const std::vector<std::filesystem::path>& globs,
	const std::string& filter,
	wadReaderMap& readers
)
{
	wadPathMap matchMap;

	for (auto& glob : globs)
	{
		if (!readers.contains(glob))
			readers.insert(std::pair{ glob, std::make_shared<Wad3Reader>(glob) });

		std::shared_ptr<Wad3Reader> reader = readers.at(glob);

		std::vector<std::string> matches = filterTextureMap(*reader, filter);

		for (auto& match : matches)
		{
			if (!matchMap.contains(match))
				matchMap.insert(std::pair{ match, std::vector<std::shared_ptr<Wad3Reader>>{} });

			matchMap.at(match).push_back(reader);
		}
	}

	return matchMap;
}

int whichwad(Options options)
{
    std::vector<std::filesystem::path> globs = findWadFilesPipes(options.modpath);
    std::vector<std::string> textures = splitString(std::stringstream(options.texture), ';');
    std::unordered_map<std::string, wadPathMap> matchingWads;
    wadPathMap matches;
    wadReaderMap readers;

    for (std::string const& tex : textures)
    {
        matches = findTextureInWads(globs, tex, readers);

        if (matches.size() == 0)
        {
            std::cout << style(error) << "No texture names matching " << style()
                << style(cyan|bold) << tex << style()
                << style(error) << " not found in any WAD in " << style()
                << style(red) << options.modpath << style() << std::endl;
            continue;
        }

        matchingWads[tex] = matches;

        if (options.everything)
        {
            std::cout << style(success) << matches.size() << style()
                << style(info) << " textures found" << std::endl;
            continue;
        }

        std::cout << style(success) << matches.size() << style()
            << style(info) << " texture names matching " << style()
            << style(magenta) << tex << style()
            << style(info) << " found:" << style() << std::endl;

        for (const auto& kv : matches)
        {
            std::cout << style(warning) << "\t" << toUpperCase(kv.first) << style()
                << " found in " << kv.second.size() << " WADS:" << std::endl;

            for (const auto& reader : kv.second)
                std::cout << style(brightBlack) << "\t" << reader->m_filepath.string() << std::endl;
        }
    }

    if (!options.extract || matchingWads.size() == 0)
        return EXIT_SUCCESS;

    std::cout << "\n";

    // Check if output dir exists, or create it
    std::filesystem::path outputPath{ options.outputDir };
    if (!std::filesystem::exists(options.outputDir) && !std::filesystem::is_directory(outputPath))
    {
        std::cout << style(warning)
            << std::filesystem::absolute(outputPath).string() + " does not exist. Create it? (Y/n) "
            << style();

        if (!confirm_dialogue(true))
        {
            std::cout << "Output dir not created, aborted\n";
            return EXIT_SUCCESS;
        }

        if (std::filesystem::create_directories(outputPath))
        {
            printSuccess(std::filesystem::absolute(outputPath).string() + " created\n");
        }
        else
        {
            logger.error("Could not create directory '%s'", std::filesystem::absolute(outputPath).string());
            exit(EXIT_FAILURE);
        }
    }

    std::string outputFile;
    bool chosenMultiWad;
    for (const auto& kv : matchingWads)
    {
        for (auto& matchReaders : kv.second)
        {
            outputFile = (outputPath / matchReaders.first).string() + ".bmp";

            if (matchReaders.second.size() == 1)
            {
                std::shared_ptr<Wad3Reader> reader = matchReaders.second[0];

                std::cout << style(info) << "Saving texture from "
                    << std::filesystem::path{ reader->m_filepath }.filename().string() << " to " << style()
                    << style(info|bold) << outputFile << style() << std::endl;

                reader->extract(matchReaders.first, outputPath);
                continue;
            }

            std::cout << toUpperCase(matchReaders.first) << style(green)
                << " found in " << matchReaders.second.size()
                << " WADs. It's time to choose:" << style() << std::endl;

            chosenMultiWad = false;
            for (const std::shared_ptr<Wad3Reader> reader : matchReaders.second)
            {
                std::string readerFilename = std::filesystem::path{ reader->m_filepath }.filename().string();
                std::cout << "Extract from " + readerFilename + "? (Y/n) ";

                if (confirm_dialogue(true))
                {
                    chosenMultiWad = true;
                    std::cout << style(info) << "Saving texture from " << readerFilename << " to " << style()
                        << style(info|bold) << outputFile << style() << std::endl;

                    reader->extract(matchReaders.first, outputPath);
                    break;
                }
            }

            if (!chosenMultiWad)
                std::cout << style(warning) << toUpperCase(matchReaders.first) << " was not extracted" << style() << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
