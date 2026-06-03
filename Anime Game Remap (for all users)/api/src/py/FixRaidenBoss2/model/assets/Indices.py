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
from typing import Optional, Dict
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import OrderedSetType
from .ModMappedAssets import ModMappedAssets
from .ModDictAssets import ModDictAssets
from ...data.BaseDataBuilder import BaseDataBuilder
from ...data.IndexData import IndexData
##### EndLocalImports


##### Script
class IndexDataBuilder(BaseDataBuilder):
    def _buildData(self) -> ModDictAssets:
        return ModDictAssets(IndexData, [ModDictAssets.VersionKey, ModDictAssets.NameKey, "component", "type"], versionIndex = ModDictAssets.VersionKey)


indexDataBuilder = IndexDataBuilder()


class Indices(ModMappedAssets):
    """
    This class inherits from :class:`ModMappedAssets`
    
    Class for managing indices for a mod :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Names of the available indices used for querying with the :meth:`get` method are:

        * version (version index)
        * name
        * component
        * type

    Parameters
    ----------
    map: Optional[Dict[:class:`str`, `OrderedSet`_[:class:`str`]]]
        The `adjacency list`_  that maps the indices to fix from to the indices to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, map: Optional[Dict[str, OrderedSetType[str]]] = None):
        super().__init__(indexDataBuilder.build(), map = map)
##### EndScript
