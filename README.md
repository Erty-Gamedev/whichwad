# Which WAD

A simple CLI utility tool for finding out which WAD of a mod that contains a specific texture.

SteamPipe directories (`_addon`, `_hd` and `_downloads`) are automatically searched within.

## Download

Head on over to [Releases](https://github.com/Erty-Gamedev/whichwad/releases/latest) and grab the latest version. No installation required.

## Usage

### Basic usage

To find which WAD in any mod that contains *some_texture*:

```cli
whichwad.exe some_texture
```

The first time you run the tool it will ask you for the path to your Steam installation.
This will be its default search directory and it will automatically find mod folders within it.

If it's not a Steam installation path, it will only search for WADs within this folder.

This setting can be changed by editing whichwad.conf created next to the executable.

### Search within a specific mod

To find which WAD in CS 1.6 that *some_texture* exists in, use the `--mod` argument:

```cli
whichwad.exe some_texture --mod cstrike
```

### Search for multiple textures

Multiple textures can be searched for at the same time by delimiting the
texture names with a space, e.g.: `my_texture1 +2other_texture !water_texture`

### Wildcard search

Entering an asterisk (`*`) in the search term will match any texture name
prefixed by that search term.

This way you can match any texture name beginning with *generic*
by searching for "generic*".

### Extract textures

Textures can also be extracted from the found WAD files using the `--extract` argument.
The `--output` argument can be used to specify where to extract the textures to.

```cli
whichwad.exe generic1 generic3 --mod cstrike --extract --output C:/projects/cs_banana/extracted
```

By default extracted textures will be placed in a subfolder of the script named *extracted*.
