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
from .HashData import HashData
from .IndexData import IndexData
from .VGRemapData import VGRemapData
from .IniParseBuilderData import IniParseBuilderData
from .IniFixBuilderData import IniFixBuilderData
from .PositionEditorData import PositionEditorData
##### EndLocalImports


##### Script
class ModData(Enum):
    """
    Raw data used by the software

    .. danger::
        Modifying these data may change how the software fixes mods. If you do
        not want this side-effect, please make a deep-copy of the data before
        editting the data

    :raw-html:`<br />`

    Attributes
    ----------
    Hashes: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]] 
        Hash data for the mods  :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version
        * The second outer key is the name of the mod
        * The inner key is the name of the type of hash
        * The inner value is the hexadecimal hash

    Indices: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]] 
        Index data for the mods :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version
        * The second outer key is the name of the mod
        * The inner key is the name of the mod object
        * The inner value is starting index for the mod object

    VGRemapData: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`VGRemap`]]]
        Vertex group remaps to change the Blend.buf files of the mods :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version
        * The second outer key is the name of the mod to fix from
        * The inner key is the name of the mod to fix to
        * The inner value is vertex group remap

    PositionEditors: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, Optional[:class:`BaseBufEditor`]]]]
        Position editors for changing the Position.buf files of the mods :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version
        * The second outer key is the name of the mod to fix from
        * The inner key is the name of the mod to fix to
        * The inner value is the editor that will edit the Position.buf files

    IniParseBuilderArgs: Dict[:class:`float`, Dict[:class:`str`, Callable[[], Tuple[:class:`BaseIniParser`, List[Any], Dict[:class:`str`, Any]]]]]
        The functions that create the arguments/keyword arguments for :class:`IniParseBuilder` to build the correct .ini parser

        * The outer key is the game version
        * The second outer key is the name of the mod to fix from
        * The inner value is the function that will create the arguments/keyword arguments

    IniFixBuilderArgs: Dict[:class:`float`, Dict[:class:`str`, Callable[[], Tuple[:class:`BaseIniFixer`, List[Any], Dict[:class:`str`, Any]]]]]
        The functions that create the arguments/keyword arguments for :class:`IniFixBuilder` to build the correct .ini fixer

        * The outer key is the game version
        * The second outer key is the name of the mod to fix from
        * The inner value is the function that will create the arguments/keyword arguments
    """

    Hashes = HashData
    Indices = IndexData
    VGRemapData = VGRemapData
    PositionEditorData = PositionEditorData
    IniParseBuilderArgs = IniParseBuilderData
    IniFixBuilderArgs = IniFixBuilderData
##### EndScript
