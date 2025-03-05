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
from typing import Dict, Optional, Set
##### EndExtImports

##### LocalImports
from ...data.VGRemapData import VGRemapData
from .ModDoubleDictAssets import ModDoubleDictAssets
from ..VGRemap import VGRemap
##### EndLocalImports


##### Script
class VGRemaps(ModDoubleDictAssets[VGRemap]):
    """
    This class inherits from :class:`ModMappedAssets`

    Class to handle Vertex Group Remaps fsor a mod

    Parameters
    ----------
    repo: Optional[Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`VGRemap`]]]]
        The original source for the vertex group remaps :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The second outer key is the name of the asset to fix from
        * The inner key is the name of the asset to fix to
        * The inner value contains the vertex group remap :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, will use the default vertex group remaps provided by the software :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, repo: Optional[Dict[float, Dict[str, Dict[str, VGRemap]]]] = None, map: Optional[Dict[str, Set[str]]] = None):
        if (repo is None):
            repo = VGRemapData

        super().__init__(repo, map = map)
##### EndScript