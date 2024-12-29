##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from enum import Enum
##### EndExtImports

##### LocalImports
from ..constants.ColourConsts import ColourConsts
from ..model.textures.Colour import Colour
from ..model.textures.ColourRange import ColourRange
##### EndLocalImports


##### Script
class Colours(Enum):
    """
    Some common colours used

    Attributes
    ----------
    White: :class:`Colour` (255, 255, 255, 255)
        white

    LightMapGreenMin: :class:`Colour` (0, 125, 0, 0)
        Minimum range for the green colour usually in the LightMap.dds

    LightMapGreenMax: :class:`Colour` (50, 150, 50, 255)
        Maximum range for the green colour usually in the LightMap.dds

    NormalMapYellow: :class:`Colour` (128, 128, 0, 255)
        The yellow that usually appears in the NormalMap.dds
    """

    White = Colour(ColourConsts.MaxColourValue.value, ColourConsts.MaxColourValue.value, ColourConsts.MaxColourValue.value)
    LightMapGreenMin = Colour(0, 125, 0, 0)
    LightMapGreenMax = Colour(50, 160, 50, ColourConsts.MaxColourValue.value)
    NormalMapYellow = Colour(128, 128, 0)

class ColourRanges(Enum):
    """
    Some common colour ranges used

    Attributes
    ----------
    LightMapGreen: :class:`ColourRange`(:attr:`Colours.LightMapGreenMin`, :attr:`Colours.LightMapGreenMax`)
        The colour range for the green usually present in LightMap.dds
    """
    LightMapGreen = ColourRange(Colours.LightMapGreenMin.value, Colours.LightMapGreenMax.value)
##### EndScript