#pragma pack(1)
#include <cstring>
#include <string>
#include <cstdint>
#include <stdio.h>
#include <fstream>
#include <cctype>
#include <algorithm>
#include "wad3reader.h"


Wad3Reader* ReadWad(const char* filepath)
{
    Wad3Header header{};
    Wad3MipTex miptex{};
    Wad3DirEntry currentDirEntry{};
    textureMap textures;

    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open() || !file.good())
    {
        file.close();
        throw std::runtime_error("Could not open '" + std::string(filepath) + "'.");
    }

    file.seekg(0, std::ios::beg);
    file.read((char*)&header, sizeof(Wad3Header));

    if (strncmp(header.szMagic, "WAD3", 4))
    {
        file.close();
        throw std::runtime_error("Invalid file type");
    }

    textures.reserve(header.nDir);

    Wad3DirEntry* dirEntries = new Wad3DirEntry[header.nDir];

    file.seekg(header.nDirOffset, std::ios::beg);
    file.read((char*)dirEntries, sizeof(Wad3DirEntry) * header.nDir);

    std::string name;
    name.reserve(MAXTEXTURENAME);

    for (int i = 0; i < header.nDir; i++)
    {
        currentDirEntry = dirEntries[i];
        name = std::string(currentDirEntry.szName);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        if (currentDirEntry.nType != ENTRY_TYPE::MIPTEX)
        {
            continue;
        }

        file.seekg(currentDirEntry.nFilePos, std::ios::beg);
        file.read((char*)&miptex, sizeof(Wad3MipTex));

        if (static_cast<size_t>(miptex.nWidth * miptex.nHeight) > MAXMIPTEXSIZE)
        {
            throw std::runtime_error("Unexpected size for texture '" + std::string(currentDirEntry.szName) + "'");
        }


        textures[name] = new MMData;
        (*textures[name]).width = miptex.nWidth;
        (*textures[name]).height = miptex.nHeight;
        (*textures[name]).data = new unsigned char[miptex.nWidth * miptex.nHeight];

        file.read((char*)(*textures[name]).data, static_cast<std::streamsize>(miptex.nWidth) * miptex.nHeight);

        file.seekg(static_cast<std::streamsize>(miptex.nWidth >> 1) * (miptex.nHeight >> 1), std::ios::cur);
        file.seekg(static_cast<std::streamsize>(miptex.nWidth >> 2) * (miptex.nHeight >> 2), std::ios::cur);
        file.seekg(static_cast<std::streamsize>(miptex.nWidth >> 3) * (miptex.nHeight >> 3), std::ios::cur);

        file.seekg(sizeof(int16_t), std::ios::cur); // Skip padding
        file.read((char*)(*textures[name]).palette, PALETTESIZE);
    }

    file.close();

    delete[] dirEntries;

    return new Wad3Reader{filepath, textures};
}


// sizeof(BMPHeader) + sizeof(BMPInfoHeader) + Palette (256 * 4)
constexpr std::uint32_t SIZEHEADER = 1078;

bool SaveTexture(MMData* texture, const char* filepath)
{
    std::uint32_t width = (*texture).width, height = (*texture).height;
    std::uint32_t size = width * height;


    // Reverse rows of bitmap data:
    unsigned char* bmpdata = new unsigned char[size];
    unsigned char* pmmdata = (*texture).data + (unsigned int)(((*texture).height - 1) * (*texture).width);
    for (unsigned int i = 0; i < (*texture).height; i++)
    {
        std::copy(pmmdata, pmmdata + (*texture).width, bmpdata + (*texture).width * i);
        pmmdata -= (*texture).width;
    }


    //Convert palette to 4-byte BGRA
    BGRA bmppalette[BMPPALETTESIZE]{};
    unsigned char* pmmpalette = (*texture).palette;
    BGRA bgra;
    for (int i = 0, j = 0; i < BMPPALETTESIZE; i++)
    {
        j = i * 3;
        bgra = {
            *(pmmpalette + j + 2),
            *(pmmpalette + j + 1),
            *(pmmpalette + j),
            0x00
        };
        memcpy(&bmppalette[i], &bgra, sizeof(BGRA)); // TODO: Replace with std::copy()
    }


    // Start writing to file
    std::ofstream file(filepath, std::ios::out | std::ios::binary | std::ios::ate | std::ios::trunc);
    if (!file.is_open() || !file.good())
    {
        file.close();
        throw std::runtime_error("Could not open '" + std::string(filepath) + "' for writing.");
    }


    BMPHeader header = {0x42, 0x4D, SIZEHEADER + size, 0, SIZEHEADER};
    file.write((char*)&header, sizeof(BMPHeader));


    BMPInfoHeader infoHeader = {
        uint32_t(40), (*texture).width, (*texture).height,
        uint16_t(1), uint16_t(8), uint32_t(0), size,
        3780, // Horizontal pixels per meter
        3780, // Vertical pixels per meter
        256,  // Number of colours used (always 256 here)
        256   // Number of important colours (always 256 here)
    };


    file.write((char*)&infoHeader, sizeof(infoHeader));
    file.write((char*)&bmppalette, sizeof(bmppalette));
    file.write((char*)bmpdata, size);

    bool res = file.good();

    file.close();

    return res;
}
