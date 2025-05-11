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

##### LocalImports
from .ByteSize import ByteSize
from .BufTypeNames import BufDataTypeNames
from ..model.buffers.BufInt import BufSignedInt, BufUnSignedInt
from ..model.buffers.BufFloat import BufFloat
from ..model.buffers.BufUnorm import BufUnorm
##### EndLocalImports


##### Script
class BufDataTypes(Enum):
    """
    Different elementary data types within a .buf file
    """

    Float32 = BufFloat()
    """
    `Floating point`_ number
    """

    Int32 = BufSignedInt()
    """
    A signed integer
    """

    UInt32 = BufUnSignedInt()
    """
    An unsigned integer
    """

    UNorm8 = BufUnorm(BufDataTypeNames.UNorm8.value, ByteSize.UNorm8.value)
    """
    An `unsigned normalized integer`_
    """
##### EndScript