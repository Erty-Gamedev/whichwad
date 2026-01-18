#pragma once

#include <set>
#include <string>
#include <vector>
#include <cstdint>
#include <atomic>
#include <csignal>
#include <string_view>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "basereader.h"
#include "wad3.h"
#include "bsp.h"


struct TextureTest
{
    using textureMatch = std::string;
    using readerPath = std::filesystem::path;

    std::string filter;
    bool hasWildcard = false;
    size_t wildcardPos = std::string::npos;
    std::unordered_map<textureMatch, std::vector<readerPath>> matches;

    TextureTest(const std::string& _filter);
    bool test(const std::string_view& textureName) const;
};


struct Options
{
    unsigned int foundMatches = 0;

    bool bsp = false;
    bool extract = false;
    bool everything = false;
    std::vector<std::string> mods;
    std::vector<TextureTest> tests;
    std::filesystem::path gamePath;
    std::filesystem::path steamDir;
    std::filesystem::path steamCommonDir;
    std::filesystem::path outputDir = "extracted";
    std::vector<std::filesystem::path> modDirs;
    std::set<std::filesystem::path> globs;

    void findGlobs();
    void checkGlobs();
private:
    void findGlobsInPipes(std::filesystem::path modDir);
    void findGlobsInDir(std::filesystem::path dir);
    void findAllMods();
};
extern Options g_options;
extern std::atomic<int> g_receivedSignal;



class Wad3Reader : public BaseReader
{
public:
    using BaseReader::m_filepath;
    Wad3Reader(const std::filesystem::path& filepath);

    bool contains(const std::string_view& textureName) const override;
    bool extract(const std::string& textureName, const std::filesystem::path& outPath) override;
private:
    WAD3Format::Wad3Header m_header{};
    std::vector<WAD3Format::Wad3DirEntry> m_dirEntries;
    void parse() override;
    void open();

    const WAD3Format::Wad3DirEntry* getDirEntry(const std::string& textureName) const;
};


class BspReader : public BaseReader
{
public:
    using BaseReader::m_filepath;
    BspReader(const std::filesystem::path& filepath);

    bool contains(const std::string_view& textureName) const override;
    bool extract(const std::string& textureName, const std::filesystem::path& outPath) override;

private:
    BSPFormat::BspHeader m_header{};
    std::vector<WAD3Format::Wad3MipTex> m_textures;
    void parse() override;
    void open();

    const WAD3Format::Wad3MipTex* getMipTex(const std::string& textureName) const;
};
