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
from .BufTypeNames import BufElementNames
from .BufFormatNames import BufFormatNames
from .BufDataTypes import BufDataTypes
from ..model.buffers.BufElementType import BufElementType
##### EndLocalImports


##### Script
class BufElementTypes(Enum):
    """
    Different types for the elements within a .buf file

    Attributes
    ----------
    PositionFloatRGB: :class:`BufElementType`
        The position for the vertex of a mod within an R3 vector space

    NormalFloatRGB: :class:`BufElementType`
        The normal vector for the vertex of a mod

    TangentFloatRGBA: :class:`BufElementType`
        The tangent vector for the vertex of a mod

    BlendWeightFloatRGBA: :class:`BufElementType`
        The distribution for the vertex groups for a particular vertex in a mod

    BlendIndicesIntRGBA: :class:`BufElementType`
        The vertex groups a vertex from a mod belongs to

    ColourRGBA: :class:`BufElementType`
        The colour for a vertex in a mod

    TextureCoordinateRG: :class:`BufElementType`
        The corresponding R2 vector space coordinate from a texture file that is associated to the vertex in a mod
    """

    PositionFloatRGB = BufElementType(BufElementNames.Position.value, BufFormatNames.Float32RGB.value, [BufDataTypes.Float32.value] * 3)
    NormalFloatRGB = BufElementType(BufElementNames.Normal.value, BufFormatNames.Float32RGB.value, [BufDataTypes.Float32.value] * 3)
    TangentFloatRGBA = BufElementType(BufElementNames.Tangent.value, BufFormatNames.Float32RGBA.value, [BufDataTypes.Float32.value] * 4)
    BlendWeightFloatRGBA = BufElementType(BufElementNames.BlendWeight.value, BufFormatNames.Float32RGBA.value, [BufDataTypes.Float32.value] * 4)
    BlendIndicesIntRGBA = BufElementType(BufElementNames.BlendIndices.value, BufFormatNames.Int32RGBA.value, [BufDataTypes.Int32.value] * 4)
    ColourRGBA = BufElementType(BufElementNames.Colour.value, BufFormatNames.UNORM8RGBA.value, [BufDataTypes.UNorm8.value] * 4)
    TextureCoordinateRG = BufElementType(BufElementNames.TextureCoordinate.value, BufFormatNames.Float32RG.value, [BufDataTypes.Float32.value] * 2)
##### EndScript