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

##### CppLocalImports
from ...core import CppBlendFile
##### EndCppLocalImports


##### Script
class BlendFile(CppBlendFile):
    """
    This class inherits from :class:`CppBlendFile`

    Used for handling blend.buf files

    .. note::
        We observe that a Blend.buf file is a binary file defined as:

        * a line corresponds to the data for a particular vertex in the mod
        * each line contains 32 bytes (256 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
        * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
        * the blend weights are floating points while the blend indices are unsigned integers

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the blend file

    elements: Optional[List[:class:`BufElementType`]]
        The sequence of elements within the .buf file :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, then will use the elements specified for some GIMI character, which is:
        ``[BufElementTypes.BlendWeightFloatRGBA.value, BufElementTypes.BlendIndicesIntRGBA.value]`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """
##### EndScript
