# Which WAD

A simple CLI utility tool for finding out which WAD of a mod that contains a specific texture.

SteamPipe directories (`_addon`, `_hd` and `_downloads`) are automatically searched within.

## Usage

### Basic usage

To find which WAD in CS 1.6 that *some_texture* exists in:

```cli
whichwad.exe C:/Steam/steamapps/Half-Life/cstrike some_texture
```

### Search for multiple textures

Multiple textures can be searched for at the same time by delimiting the
texture names with a semicolon, e.g.: `my_texture1;+0_other_texture;!water_texture`

### Wildcard search

Entering an asterisk (`*`) in the search term will match any texture name
prefixed by that search term.

This way you can match any texture name beginning with *generic*
by searching for "generic*".

### Extract textures

Textures can also be extracted from the found WAD files using the `--extract` argument.
The `--output` argument can be used to specify where to extract the textures to.

```cli
python whichwad.py C:/Steam/steamapps/Half-Life/cstrike generic1;generic3 --extract --output C:/projects/cs_banana/extracted
```

By default extracted textures will be placed in a subfolder of the script named *extracted*.
