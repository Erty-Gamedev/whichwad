#include <iostream>
#include <set>
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
    for (const auto& dirEntry : reader.m_dirEntries)
    {
        std::string entryName = toLowerCase(dirEntry.szName);
		if (wildcardCompare(filter, entryName))
			matches.emplace_back(dirEntry.szName);
    }
	return matches;
}

wadPathMap findTextureInWads(
	const std::set<std::filesystem::path>& globs,
	const std::string& filter,
	wadReaderMap& readers
)
{
	wadPathMap matchMap;

	for (const auto& glob : globs)
	{
        if (!readers.contains(glob))
        {
            try
            {
                readers.insert_or_assign(glob, std::make_unique<Wad3Reader>(glob));
            }
            catch (const std::runtime_error& e)
            {
                logger.debug(e.what());
                continue;
            }
        }

        Wad3Reader& reader = *readers.at(glob);

		std::vector<std::string> matches = filterTextureMap(reader, filter);

		for (const auto& match : matches)
		{
            if (!matchMap.contains(match))
                matchMap[match] = {};

            matchMap.at(match).push_back(readers.at(glob).get());
		}
	}

	return matchMap;
}


static inline void findMods(std::set<std::filesystem::path>& modDirs, const std::filesystem::path& gameDir, const std::string& mod)
{
    if (!std::filesystem::is_directory(gameDir)) return;

    if (!mod.empty())
    {
        for (const auto &dirEntry : std::filesystem::directory_iterator(gameDir))
        {
            std::filesystem::path entryPath = dirEntry.path();
            if (entryPath.stem().string() == mod)
            {
                modDirs.insert(entryPath.parent_path() / unsteampipe(entryPath.stem().string()));
                return;
            }
        }
        return;
    }

    for (const auto& dirEntry : std::filesystem::directory_iterator(gameDir))
    {
        std::filesystem::path entryPath = dirEntry.path();
        if (!std::filesystem::is_directory(entryPath) || !std::filesystem::exists(entryPath / "liblist.gam"))
            continue;

        modDirs.insert(entryPath.parent_path() / unsteampipe(entryPath.stem().string()));
    }
}

static inline std::set<std::filesystem::path> findModDirs(const std::filesystem::path& steamDir, const std::string& mod = "")
{
    std::set<std::filesystem::path> modDirs;
    std::filesystem::path common{ steamDir / "steamapps" / "common" };

    if (!std::filesystem::is_directory(common))
    {
        modDirs.insert(steamDir);
        return modDirs;
    }

    findMods(modDirs, common / "Half-Life", mod);
    findMods(modDirs, common / "Sven Co-op", mod);

    return modDirs;
}


int whichwad(const Options& options)
{
    std::set<std::filesystem::path> globs;

    for (const std::filesystem::path& modDir : findModDirs(options.steamDir, options.mod))
        findWadFilesPipes(modDir, globs);

    logger.debug("Found %i WAD files", globs.size());


    std::unordered_map<std::string, wadPathMap> matchingWads;
    wadPathMap matches;
    wadReaderMap readers;

    for (std::string const& tex : options.textures)
    {
        matches = findTextureInWads(globs, tex, readers);

        if (matches.size() == 0)
        {
            std::cout << style(error) << "No texture names matching " << style()
                << style(info|bold) << tex << style()
                << style(error) << " not found in any WAD in the search path" << style() << std::endl;
            continue;
        }

        matchingWads[tex] = matches;

        if (options.everything)
        {
            std::cout << style(info|bold) << matches.size() << style()
                << style(info) << " textures found" << std::endl;
            continue;
        }

        std::cout << style(info|bold) << matches.size()
            << style(info) << " texture names matching "
            << style(info|bold) << tex
            << style(info) << " found:" << style() << std::endl;

        for (const auto& kv : matches)
        {
            std::cout << style(warning) << "  " << toUpperCase(kv.first) << style()
                << " found in " << kv.second.size() << " WADS:" << std::endl;

            for (const auto& reader : kv.second)
                std::cout << style(brightBlack) << "    " << reader->m_filepath.string() << std::endl;
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
                Wad3Reader& reader = *matchReaders.second[0];

                std::cout << style(info) << "Saving texture from "
                    << reader.m_filepath.filename().string() << " to " << style()
                    << style(info|bold) << outputFile << style() << std::endl;

                reader.extract(matchReaders.first, outputPath);
                continue;
            }

            std::cout << toUpperCase(matchReaders.first) << style(green)
                << " found in " << matchReaders.second.size()
                << " WADs. It's time to choose:" << style() << std::endl;

            chosenMultiWad = false;
            for (const auto& reader : matchReaders.second)
            {
                std::string readerFilename = reader->m_filepath.filename().string();
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
