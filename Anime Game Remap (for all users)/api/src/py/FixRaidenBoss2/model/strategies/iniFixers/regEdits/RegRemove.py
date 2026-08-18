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
from typing import TYPE_CHECKING, Optional, Dict, Any, Callable
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
from .....core import IfContentPart
##### EndCppLocalImports

##### LocalImports
from .BaseRegEdit import BaseRegEdit

if (TYPE_CHECKING):
    from ...ModType import ModType
##### EndLocalImports


##### Script
class RegRemove(BaseRegEdit):
    """
    This class inherits from :class:`BaseRegEdit`

    Bulk-removes register keys for some :class:`IfContentPart`

    Parameters
    ----------
    removeKeys: Dict[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]]
        Each key to remove, mapped to its own optional check predicate :raw-html:`<br />` :raw-html:`<br />`

        See :meth:`IfContentPart.removeKeys` for the full semantics of how the predicates decide
        which occurrences of a key actually get removed

    Attributes
    ----------
    removeKeys: Dict[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]]
        Each key to remove, mapped to its own optional check predicate :raw-html:`<br />` :raw-html:`<br />`

        See :meth:`IfContentPart.removeKeys` for the full semantics of how the predicates decide
        which occurrences of a key actually get removed
    """

    def __init__(self, removeKeys: Dict[Any, Optional[Callable[[int, Any], bool]]]):
        self.removeKeys = removeKeys

    def edit(self, part: IfContentPart, sectionName: str, modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        part.removeKeys(self.removeKeys, ranges = partRanges)
        return part
##### EndScript