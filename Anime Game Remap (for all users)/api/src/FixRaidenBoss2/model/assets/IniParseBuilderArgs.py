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
from typing import Callable, List, Dict, Any, Tuple, Optional
##### EndExtImports

##### LocalImports
from ...data.IniParseBuilderData import IniParseBuilderData
from ..strategies.iniParsers.BaseIniParser import BaseIniParser
from .ModDictAssets import ModDictAssets
##### EndLocalImports


##### Script
class IniParseBuilderArgs(ModDictAssets[Callable[[], Tuple[BaseIniParser, List[Any], Dict[str, Any]]]]):
    """
    This class inherits from :class:`ModDictAssets`
    
    Class for managing functions that create the arguments/keyword arguments for an :class:`IniParseBuilder`

    Parameters
    ----------
    repo: Optional[Dict[:class:`str`, Dict[:class:`str`, Callable[[], Tuple[:class:`BaseIniParser` , List[Any], Dict[:class:`str`, Any]]]]]]
        The original source for any the function that create arguments :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value contains the functions that create arguments/keyword arguments for an :class:`IniParseBuilder`  :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, will use the default functions provided by the software :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, repo: Optional[Dict[str, Dict[str, Callable[[], Tuple[BaseIniParser, List[Any], Dict[str, Any]]]]]] = None):
        if (repo is None):
            repo = IniParseBuilderData

        super().__init__(repo)
##### EndScript
