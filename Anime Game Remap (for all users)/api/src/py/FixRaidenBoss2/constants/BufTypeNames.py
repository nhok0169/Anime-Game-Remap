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
class BufDataTypeNames(Enum):
    """
    The names of the data types within a .buf file
    """

    Float16 = "Float16"
    """
    `Half precision floating point`_ number
    """

    Float32 = "Float32"
    """
    `floating point`_ number
    """

    Int32 = "SignedInt32"
    """
    Signed integer
    """

    UInt32 = "UnsignedInt32"
    """
    Unsigned integer
    """

    UNorm8 = "UNORM8"
    """
    An 8-bit `unsigned normalized integer`_
    """


class BufElementNames(Enum):
    """
    Usual names for the elements within a .buf file
    """

    Position = "POSITION"
    """
    The coordinate of some vertex of a mod
    """

    Normal = "NORMAL"
    """
    The normal vector of some vertex of a mod
    """

    Tangent = "TANGENT"
    """
    The tangent vector of some vertex of a mod
    """

    BlendWeight = "BLENDWEIGHT"
    """
    The distribution of how much a vertex belongs to a certain vertex group
    """

    BlendIndices = "BLENDINDICES"
    """
    The vertex groups that a vertex belongs to
    """

    Colour = "COLOR"
    """
    The colour at the vertex
    """

    TextureCoordinate = "TEXCOORD"
    """
    The coordinate of the texture file that the vertex is associated with
    """
##### EndScript