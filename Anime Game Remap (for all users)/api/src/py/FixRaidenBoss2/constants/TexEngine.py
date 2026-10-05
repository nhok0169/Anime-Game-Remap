##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from enum import Enum
##### EndExtImports


##### Script
class TexEngine(Enum):
    """
    The different engines that can be used to read/write a texture (``.dds``) file
    """

    Compressonator = "compressonator"
    """
    Uses `Compressonator`_ to read/write the texture file -- cross-platform, and re-encodes back to
    whichever compressed format (eg. BC7) the file already used. This is the default engine
    """

    Pillow = "pillow"
    """
    Uses `Pillow`_ to read/write the texture file directly, bypassing `Compressonator`_ entirely
    """
##### EndScript
