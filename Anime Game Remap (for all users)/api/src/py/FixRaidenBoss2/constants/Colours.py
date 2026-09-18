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

    LightMapGreen: :class:`Colour` (0, 128, 0, 255)
        The usual colour for the green in the LightMap.dds

    NormalMapYellow: :class:`Colour` (128, 128, 0, 255)
        The yellow that usually appears in the NormalMap.dds

    NormalMapBlue: :class:`Colour` (128, 128, 255, 255)
        The light blue that usually appears in the NormalMap.dds

    NormalMapPurple1: :class:`Colour` (128, 98, 128, 255)
        The light purple with rgb(128, 98, 128) that appears in the NormalMap.dds

    NormalMapPurple2: :class:`Colour` (128, 114, 128, 255)
        The light purple with rgb(128, 114, 128) that appears in the NormalMap.dds

    NormalMapPurpleMin: :class:`Colour` (128, 0, 128)
        Minimum range for the purple colour usually in NormalMap.dds
    """

    White = Colour(ColourConsts.MaxColourValue.value, ColourConsts.MaxColourValue.value, ColourConsts.MaxColourValue.value)
    LightMapGreenMin = Colour(0, 125, 0, 0)
    LightMapGreenMax = Colour(50, 160, 50, ColourConsts.MaxColourValue.value)
    LightMapGreen = Colour(0, 128, 0, ColourConsts.MaxColourValue.value)
    NormalMapYellow = Colour(128, 128, 0)
    NormalMapBlue = Colour(128, 128, 255)
    NormalMapPurple1 = Colour(128, 98, 128)
    NormalMapPurple2 = Colour(128, 114, 128)
    NormalMapPurpleMin = Colour(128, 0, 128)

class ColourRanges(Enum):
    """
    Some common colour ranges used

    Attributes
    ----------
    LightMapGreen: :class:`ColourRange` (:attr:`Colours.LightMapGreenMin`, :attr:`Colours.LightMapGreenMax`)
        The colour range for the green usually present in LightMap.dds

    NormalMapPurple1: :class:`ColourRange` (:attr:`Colours.NormalMapPurpleMin`, :attr:`Colours.NormalMapPurple1`)
        The colour range for the colour :class:`Colour.NormalMapPurple1` that usually appears in NormalMap.dds
    """
    LightMapGreen = ColourRange(Colours.LightMapGreenMin.value, Colours.LightMapGreenMax.value)
    NormalMapPurple1 = ColourRange(Colours.NormalMapPurpleMin.value, Colours.NormalMapPurple1.value)
    NormalMapPurple2 = ColourRange(Colours.NormalMapPurpleMin.value, Colours.NormalMapPurple2.value)
##### EndScript