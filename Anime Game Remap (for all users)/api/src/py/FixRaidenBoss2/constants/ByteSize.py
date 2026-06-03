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
class ByteSize(Enum):
    """
    Different byte sizes for particular elements in the binary files of mods
    """

    Int32 = 4
    """
    Number of bytes in a `signed integer`_
    """

    UInt32 = 4
    """
    Number of bytes in an `unsigned integer`_
    """

    Float16 = 2
    """
    Number of bytes in a `half precision floating point`_
    """

    Float32 = 4
    """
    Number of bytes in a `floating point`_
    """

    UNorm8 = 1
    """
    Number of bytes in an 8-bit `unsigned normalized integer`_
    """
##### EndScript