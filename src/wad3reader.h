#pragma pack(1)
#ifndef WHICHWAD_WAD3READER_H
#define WHICHWAD_WAD3READER_H


#include <vector>
#include <unordered_map>

constexpr size_t MAXTEXTURENAME = 16;
constexpr size_t MIPLEVELS = 4;
constexpr size_t PALETTESIZE = 768;
constexpr size_t BMPPALETTESIZE = 256;
constexpr size_t MAXMIPTEXSIZE = 16384 * 16384;


enum ENTRY_TYPE: std::int8_t
{
    QPIC = 0x42,
    MIPTEX = 0x43,
    FONT = 0x45,
    SPRAYDECAL = 0x40,
};

typedef struct
{
    char szMagic[4];         // should be WAD3
    std::int32_t nDir;       // number of directory entries
    std::int32_t nDirOffset; // offset into directory
} Wad3Header;

typedef struct
{
    std::int32_t nFilePos;       // offset in WAD
    std::int32_t nDiskSize;      // size in file
    std::int32_t nSize;          // uncompressed size
    std::int8_t nType;           // type of entry
    bool bCompression;           // 0 if none
    std::int16_t nDummy;         // not used
    char szName[MAXTEXTURENAME]; // must be null terminated
} Wad3DirEntry;

typedef struct
{
    char szName[MAXTEXTURENAME];       // Name of texture
    std::uint32_t nWidth, nHeight;     // Extends of the texture
    std::uint32_t nOffsets[MIPLEVELS]; // Offsets to texture mipmaps
} Wad3MipTex;


struct MMData
{
    std::uint32_t width, height;
    unsigned char palette[PALETTESIZE];
    unsigned char* data;
};

typedef std::unordered_map<std::string, MMData*> textureMap;
struct Wad3Reader
{
    std::string filepath;
    textureMap textures;
};

typedef std::unordered_map<
    std::string, std::vector<std::reference_wrapper<Wad3Reader*>>
> matchTextureMap;
typedef std::unordered_map<std::string, matchTextureMap> foundMap;


Wad3Reader* ReadWad(const char* filepath);
bool SaveTexture(MMData *texture, const char* filepath);


typedef struct
{
    unsigned char b, g, r, a;
} BGRA;

constexpr size_t BMPSIZEHEADER = 1078;

typedef struct
{
    unsigned char signature[2];
    std::uint32_t filesize;
    std::uint32_t reserved;
    std::uint32_t dataOffset;
} BMPHeader;

typedef struct
{
    std::uint32_t size;
    std::uint32_t width;
    std::uint32_t height;
    std::uint16_t planes;
    std::uint16_t bitsPerPixel;
    std::uint32_t compression;
    std::uint32_t imageSize;
    std::uint32_t horizontalPixelsPerM;
    std::uint32_t verticalPixelsPerM;
    std::uint32_t coloursUsed;
    std::uint32_t importantColours;
} BMPInfoHeader;

#endif
