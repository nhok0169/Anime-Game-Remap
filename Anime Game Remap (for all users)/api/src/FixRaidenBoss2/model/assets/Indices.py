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
from typing import Optional, Dict, Set
##### EndExtImports

##### LocalImports
from .ModIdAssets import ModIdAssets
from ...data.IndexData import IndexData
##### EndLocalImports


##### Script
class Indices(ModIdAssets):
    """
    This class inherits from :class:`ModDictStrAssets`
    
    Class for managing indices for a mod

    Parameters
    ----------
    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the indices to fix from to the indices to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(IndexData, map = map)
##### EndScript
