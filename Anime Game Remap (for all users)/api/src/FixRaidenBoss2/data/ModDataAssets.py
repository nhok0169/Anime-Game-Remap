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
from ..model.assets.IniParseBuilderArgs import IniParseBuilderArgs
from ..model.assets.IniFixBuilderArgs import IniFixBuilderArgs
from ..model.assets.VertexCounts import VertexCounts
##### EndLocalImports


##### Script
class ModDataAssets(Enum):
    """
    Refined data used by the software, grouped by version of the game

    .. danger::
        Modifying these data may change how the software fixes mods. If you do
        not want this side-effect, please make a deep-copy of the data before
        editting the data

    :raw-html:`<br />`

    Attributes
    ----------
    IniParseBuilderArgs: :class:`IniParseBuilderArgs`
        The functions that create the arguments/keyword arguments for :class:`IniParseBuilder` to build the correct .ini parser

    IniFixBuilderArgs: :class:`IniFixBuilderArgs`
        The functions that create the arguments/keyword arguments for :class:`IniFixBuilder` to build the correct .ini fixer

    VertexCounts: :class:`VertexCounts`
        The total # of vertices for each mod
    """

    IniParseBuilderArgs = IniParseBuilderArgs()
    IniFixBuilderArgs = IniFixBuilderArgs()
    VertexCounts = VertexCounts()
##### EndScript