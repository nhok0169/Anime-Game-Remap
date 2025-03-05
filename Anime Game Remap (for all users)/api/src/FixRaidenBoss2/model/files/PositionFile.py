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
from typing import Union
##### EndExtImports

##### LocalImports
from ...constants.BufElementTypes import BufElementTypes
from .BufFile import BufFile
##### EndLocalImports


##### Script
class PositionFile(BufFile):
    """
    This class inherits from :class:`BufFile`

    Used for handling position.buf files

    .. note::
        We observe that a Position.buf file is a binary file defined as:

        * a line corresponds to the data for a particular vertex in the mod
        * each line contains 40 bytes (320 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 12 bytes of a line are the coordinate position of a vertex in an R3 vector space, each scaler value in the coordinate is 4 bytes or 32 bits (3 scalar values/line)
        * the next 12 bytes of a line corresponds to the normal vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (3 scalar values/line)
        * the last 16 bytes of a line corresponds to the tangent vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (4 scalar values/line)
        * all scalar values in the file are `floating point`_ values

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the .buf file
    """

    def __init__(self, src: Union[str, bytes]):
        super().__init__(src, [BufElementTypes.PositionFloatRGB.value, BufElementTypes.NormalFloatRGB.value, BufElementTypes.TangentFloatRGBA.value], fileType = "Position.buf")
##### EndScript