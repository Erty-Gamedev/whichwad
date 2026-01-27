#include <set>
#include <ranges>
#include <cstring>
#include <iostream>
#include <source_location>
#include "utils.h"
#include "whichwad.h"
#include "logging.h"
#include "bmp8bpp.h"

using namespace Styling;
namespace fs = std::filesystem;

static Logging::Logger& logger = Logging::Logger::getLogger("whichwad");

// Return, clear line, previous line, clear line
static const char* c_resetTwoLines = "\r\033[0K\033[1F\033[0K";

Options g_options{};
std::atomic<int> g_receivedSignal = -1;


TextureTest::TextureTest(const std::string& _filter) : filter(toLowerCase(_filter))
{
    wildcardPos = filter.find('*');
    wildcardRPos = filter.rfind('*');
    hasWildcard = wildcardPos != std::string::npos;

    if (!hasWildcard)
        return;  // Early return, the next checks are for if it has wildcard only

    if (wildcardPos + 1 == wildcardRPos)
        throw std::runtime_error("Contains filter cannot be empty");

    if (wildcardPos > 0 && wildcardPos < filter.size() - 1)
        throw std::runtime_error("Wildcard ('*') is only allowed at start and end of a search term");
}

bool TextureTest::test(const std::string_view& textureName) const
{
    size_t texNameLength = textureName.length();

    if (!hasWildcard)
        return filter.length() == texNameLength && textureName == filter;

    if (wildcardPos != wildcardRPos)
        return filter.length() <= texNameLength + 2 && textureName.find(filter.substr(wildcardPos + 1, wildcardRPos - 1)) != std::string::npos;

    if (filter.length() > (texNameLength + 1))
        return false;

    if (wildcardPos == 0)
    {
        const size_t searchLength = filter.length() - 1;
        return textureName.compare(texNameLength - searchLength, searchLength, filter.substr(1, searchLength)) == 0;
    }

    return textureName.compare(0, wildcardPos, filter.substr(0, wildcardPos)) == 0;
}



void Options::findGlobsInDir(const fs::path& dir)
{
    if (bsp)
    {
        for (const auto& entry : fs::directory_iterator(dir))
        {
            if (const fs::path& entryPath = entry.path(); toLowerCase(entryPath.extension().string()) == ".bsp")
            {
                if (g_options.absoluteDir)
                {
                    globs.insert(entryPath);
                    continue;
                }
                fs::path shortGlob = entryPath.parent_path().parent_path().parent_path().stem()
                    / entryPath.parent_path().parent_path().stem() / entryPath.parent_path().stem() / entryPath.filename();
                globs.insert(shortGlob);
            }
        }

        return;
    }

    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (const fs::path& entryPath = entry.path(); toLowerCase(entryPath.extension().string()) == ".wad")
        {
            if (std::ranges::find(c_WadSkipList, toLowerCase(entryPath.stem().string())) != c_WadSkipList.end())
                continue;

            if (g_options.absoluteDir)
            {
                globs.insert(entryPath);
                continue;
            }

            fs::path shortGlob = entryPath.parent_path().parent_path().stem()
                / entryPath.parent_path().stem() / entryPath.filename();
            globs.insert(shortGlob);
        }
    }
}

void Options::findGlobsInPipes(fs::path modDir)
{
    const std::string baseMod = modDir.stem().string();
    gamePath = modDir.parent_path();

    if (bsp)
    {
        if (fs::is_directory(modDir / "maps"))
            findGlobsInDir(modDir / "maps");

        for (const auto& pipe : c_SteamPipes)
        {
            modDir = gamePath / (baseMod + pipe);
            if (fs::is_directory(modDir / "maps"))
                findGlobsInDir(modDir / "maps");
        }
        return;
    }

    if (fs::is_directory(modDir))
        findGlobsInDir(modDir);

    for (const auto& pipe : c_SteamPipes)
    {
        modDir = gamePath / (baseMod + pipe);
        if (fs::is_directory(modDir))
            findGlobsInDir(modDir);
    }
}

void Options::findAllMods()
{
    if (fs::is_directory(steamCommonDir / "Sven Co-op/svencoop"))
        modDirs.emplace_back(steamCommonDir / "Sven Co-op/svencoop");

    if (!fs::is_directory(steamCommonDir / "Half-Life"))
        return;

    for (const auto& entry : fs::directory_iterator(steamCommonDir / "Half-Life"))
    {
        if (!fs::exists(steamCommonDir / "Half-Life" / entry / "liblist.gam"))
            continue;

        modDirs.emplace_back(steamCommonDir / "Half-Life" / entry);
    }
}

void Options::findGlobs()
{
    if (fs::is_directory(g_options.steamDir) && !fs::is_directory(g_options.steamCommonDir))
    {
        g_options.absoluteDir = true;
        findGlobsInDir(g_options.steamDir);
        return;
    }

    if (mods.empty())
        findAllMods();
    else
    {
        for (const auto& mod : mods)
        {
            if (mod == "svencoop")
                gamePath = g_options.steamCommonDir / "Sven Co-op";
            else
                gamePath = g_options.steamCommonDir / "Half-Life";

            fs::path modDir = gamePath / mod;
            if (!std::filesystem::is_directory(modDir))
            {
                logger.warning("\"" + modDir.string() + "\" is not a directory");
                continue;
            }
            modDirs.push_back(std::move(modDir));
        }
    }

    for (const auto& modDir : modDirs)
        findGlobsInPipes(modDir);
}

void Options::checkGlobs() const
{
    for (const auto& glob : globs)
    {
        std::cout << "Reading "
            << (g_options.absoluteDir ? glob.filename() : glob).string() << "\nFound " << g_options.foundMatches;

        try
        {
            if (bsp)
                BspReader reader{ glob };
            else
                Wad3Reader reader{ glob };
        }
        catch (const std::runtime_error& e)
        {
            if (logger.getLevel() > Logging::LogLevel::Debug)
                continue;
            std::cerr << c_resetTwoLines;  // Insert before WARNING prefix by logger
            logger.warning("Could not read " + glob.string() + ". Reason: " + e.what(), std::source_location());
        }

        if ((g_receivedSignal != -1))
            break;

        std::cout << c_resetTwoLines;
    }
}


Wad3Reader::Wad3Reader(const std::filesystem::path& filepath) : BaseReader(filepath)
{
    using namespace WAD3Format;

    open();

    m_file.read(reinterpret_cast<char*>(&m_header), sizeof(Wad3Header));

    if (strncmp(m_header.szMagic, "WAD3", 4) != 0)
    {
        m_file.close();
        throw std::runtime_error("Unexpected WAD format");
    }

    Wad3Reader::parse();
    m_file.close();
}

void Wad3Reader::open()
{
    m_file.open(g_options.steamCommonDir / m_filepath, std::ios::binary);
    if (!m_file.is_open() || !m_file.good())
    {
        m_file.close();
        throw std::runtime_error("Could not open " + m_filepath.string());
    }
}

bool Wad3Reader::contains(const std::string_view& textureName) const
{
    return std::ranges::find_if(m_dirEntries, [textureName](const WAD3Format::Wad3DirEntry& dirEntry) {
        return toLowerCase(dirEntry.szName) == textureName;
        }) != m_dirEntries.end();
}

bool Wad3Reader::extract(const std::string& textureName, const std::filesystem::path& outPath)
{
    using namespace WAD3Format;
    using namespace BMP;

    // Read texture data from WAD

    const Wad3DirEntry* dirEntry = getDirEntry(textureName);
    if (dirEntry == nullptr)
    {
        std::cerr << style(warning) << "Could not extract \"" + textureName + "\" from " + m_filepath.string() << std::endl;
        return false;
    }

    if (dirEntry->nType != MIPTEX)
    {
        std::cerr << style(warning) << "Texture \"" + textureName + "\" is not a MipTex type" << std::endl;
        return false;
    }

    open();  // Make sure file is opened

    m_file.seekg(dirEntry->nFilePos);

    Wad3MipTex miptex{};
    m_file.read(reinterpret_cast<char*>(&miptex), sizeof(Wad3MipTex));

    const std::ifstream::off_type width = miptex.nWidth;
    const std::ifstream::off_type height = miptex.nHeight;
    const std::ifstream::off_type textureSize = width * height;
    std::vector<unsigned char> data(textureSize, {});

    m_file.seekg(dirEntry->nFilePos + miptex.nOffsets[0]);

    m_file.read(reinterpret_cast<char*>(data.data()), textureSize);  // Read mipmap 0

    m_file.seekg((width >> 1) * (height >> 1), std::ios::cur);  // Skip mipmap 1
    m_file.seekg((width >> 2) * (height >> 2), std::ios::cur);  // Skip mipmap 2
    m_file.seekg((width >> 3) * (height >> 3), std::ios::cur);  // Skip mipmap 3
    m_file.seekg(sizeof(int16_t), std::ios::cur); // Skip colours used (always 256 here)

    unsigned char palette[c_PALETTESIZE]{};
    m_file.read(reinterpret_cast<char*>(&palette[0]), c_PALETTESIZE);

    m_file.close();


    // Prepare data for BMP

    BMP8Bpp bmp(static_cast<int>(width), static_cast<int>(height));
    bmp.m_data = std::vector<unsigned char>(textureSize);

    // Vertically flip data

    size_t currentPos = (height - 1) * width;
    for (int i = 0; i < height; ++i)
    {
        std::copy_n(&data[currentPos], width, &bmp.m_data[width * i]);
        currentPos -= width;
    }

    // Convert palette from RGB to BGRA

    bmp.m_palette = std::vector<unsigned char>(c_BMPPALETTESIZE * 4);
    BGRA bgra{};
    for (int i = 0; i < c_BMPPALETTESIZE; ++i)
    {
        bgra = {
            palette[i * 3 + 2],
            palette[i * 3 + 1],
            palette[i * 3],
            0x00
        };
        std::copy_n(reinterpret_cast<char*>(&bgra), sizeof(BGRA), &bmp.m_palette[i * sizeof(BGRA)]);
    }


    // Save BMP

    const std::filesystem::path filepath = outPath / (std::string{ miptex.szName } + ".bmp");
    return bmp.save(filepath);
}

void Wad3Reader::parse()
{
    using namespace WAD3Format;

    m_file.seekg(m_header.nDirOffset, std::ios::beg);
    m_dirEntries.assign(m_header.nDir, {});
    for (int i = 0; i < m_header.nDir; ++i)
    {
        m_file.read(reinterpret_cast<char*>(&(m_dirEntries[i])), sizeof(Wad3DirEntry));
        for (auto& test : g_options.tests)
        {
            std::string texName = toLowerCase(m_dirEntries[i].szName);
            if (test.test(texName))
            {
                test.matches[texName].push_back(m_filepath);
                ++g_options.foundMatches;
            }
        }
    }
}

const WAD3Format::Wad3DirEntry* Wad3Reader::getDirEntry(const std::string& textureName) const
{
    using namespace WAD3Format;

    for (const Wad3DirEntry& dirEntry : m_dirEntries)
    {
        if (toLowerCase(textureName) == toLowerCase(dirEntry.szName))
            return &dirEntry;
    }

    return nullptr;
}


BspReader::BspReader(const std::filesystem::path& filepath) : BaseReader(filepath)
{
    using namespace BSPFormat;

    open();

    m_file.read(reinterpret_cast<char*>(&m_header), sizeof(BspHeader));

    if (m_header.version != 30 && m_header.version != 29)
    {
        m_file.close();
        throw std::runtime_error("Unexpected BSP version: " + std::to_string(m_header.version));
    }

    BspReader::parse();
    m_file.close();
}

bool BspReader::contains(const std::string_view& textureName) const
{
    return std::ranges::find_if(m_textures, [textureName](const WAD3Format::Wad3MipTex& miptex) {
        return toLowerCase(miptex.szName) == textureName;
    }) != m_textures.end();
}

void BspReader::open()
{
    m_file.open(g_options.steamCommonDir / m_filepath, std::ios::binary);
    if (!m_file.is_open() || !m_file.good())
    {
        m_file.close();
        throw std::runtime_error("Could not open " + m_filepath.string());
    }
}

const WAD3Format::Wad3MipTex* BspReader::getMipTex(const std::string& textureName) const
{
    using namespace WAD3Format;

    for (const Wad3MipTex& dirEntry : m_textures)
    {
        if (toLowerCase(textureName) == toLowerCase(dirEntry.szName))
            return &dirEntry;
    }

    return nullptr;
}

bool BspReader::extract(const std::string& textureName, const std::filesystem::path& outPath)
{
    using namespace BSPFormat;
    using namespace WAD3Format;
    using namespace BMP;

    // Read texture data from WAD

    const Wad3MipTex* miptex = getMipTex(textureName);
    if (miptex == nullptr)
    {
        std::cerr << style(warning) << "Could not extract \"" + textureName + "\" from " + m_filepath.string() << std::endl;
        return false;
    }

    open();  // Make sure file is opened

    const std::ifstream::off_type width = miptex->nWidth;
    const std::ifstream::off_type height = miptex->nHeight;
    const std::ifstream::off_type textureSize = width * height;

    std::vector<unsigned char> data(textureSize);
    std::vector<unsigned char> palette(c_PALETTESIZE);

    m_file.seekg(miptex->nOffsets[0]);
    m_file.read(reinterpret_cast<char*>(&data[0]), textureSize);  // Read mipmap 0
    m_file.seekg((width >> 1) * (height >> 1), std::ios::cur);  // Skip mipmap 1
    m_file.seekg((width >> 2) * (height >> 2), std::ios::cur);  // Skip mipmap 2
    m_file.seekg((width >> 3) * (height >> 3), std::ios::cur);  // Skip mipmap 3
    m_file.seekg(sizeof(int16_t), std::ios::cur); // Skip colours used (always 256 here)

    m_file.read(reinterpret_cast<char*>(&palette[0]), c_PALETTESIZE);

    m_file.close();


    // Prepare data for BMP

    BMP8Bpp bmp(static_cast<int>(width), static_cast<int>(height));
    bmp.m_data = std::vector<unsigned char>(textureSize);

    // Vertically flip data

    size_t currentPos = (height - 1) * width;
    for (int i = 0; i < height; ++i)
    {
        std::copy_n(&data[currentPos], width, &bmp.m_data[width * i]);
        currentPos -= width;
    }

    // Convert palette from RGB to BGRA

    bmp.m_palette = std::vector<unsigned char>(c_BMPPALETTESIZE * 4);
    BGRA bgra{};
    for (int i = 0; i < c_BMPPALETTESIZE; ++i)
    {
        bgra = {
            palette[i * 3 + 2],
            palette[i * 3 + 1],
            palette[i * 3],
            0x00
        };
        std::copy_n(reinterpret_cast<char*>(&bgra), sizeof(BGRA), &bmp.m_palette[i * sizeof(BGRA)]);
    }


    // Save BMP

    const std::filesystem::path filepath = outPath / (std::string{ miptex->szName } + ".bmp");
    return bmp.save(filepath);
}

void BspReader::parse()
{
    using namespace BSPFormat;

    BspLump& textureLump = m_header.lumps[Textures];
    m_file.seekg(textureLump.offset, std::ios::beg);

    std::uint32_t countMipTextures{};
    m_file.read(reinterpret_cast<char*>(&countMipTextures), sizeof(std::uint32_t));
    m_textures.assign(countMipTextures, {});


    std::int32_t mipTexOffset{};
    for (unsigned int i = 0; i < countMipTextures; ++i)
    {
        m_file.read(reinterpret_cast<char*>(&mipTexOffset), sizeof(std::int32_t));
        std::streampos temp = m_file.tellg();
        m_file.seekg(static_cast<std::ifstream::off_type>(textureLump.offset) + mipTexOffset, std::ios::beg);
        m_file.read(reinterpret_cast<char*>(&m_textures[i]), sizeof(WAD3Format::Wad3MipTex));

        if (m_textures[i].nOffsets[0] == 0)
        {
            m_file.seekg(temp, std::ios::beg);
            continue;
        }

        for (unsigned int & nOffset : m_textures[i].nOffsets)
            nOffset += static_cast<size_t>(textureLump.offset) + mipTexOffset;
        m_file.seekg(temp, std::ios::beg);

        for (auto& test : g_options.tests)
        {
            std::string texName = toLowerCase(m_textures[i].szName);
            if (test.test(texName))
            {
                test.matches[texName].push_back(m_filepath);
                ++g_options.foundMatches;
            }
        }
    }
}
