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
class BufFormatNames(Enum):
    """
    Names for the different 3dmigoto types for the elements within a .buf file :raw-html:`<br />` :raw-html:`<br />`

    For more info on the types, please see the type definitions here:
    https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
    """

    Float32RG = "R32G32_FLOAT"
    """
    A type with 2 channels of `floating point`_ numbers
    """

    Float32RGB = "R32G32B32_FLOAT"
    """
    A type with 3 channels of `floating point`_ numbers
    """

    Float32RGBA = "R32G32B32A32_FLOAT"
    """
    A type with 4 channels of `floating point`_ numbers
    """

    Int32RGBA = "R32G32B32A32_SINT"
    """
    A type with 4 channels of signed integers
    """

    UNORM8RGBA = "R8G8B8A8_UNORM"
    """
    A type with 4 channels of `unsigned normalized integers`_ with 8 bits per integer
    """
##### EndScript