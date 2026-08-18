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
from typing import TYPE_CHECKING, Dict, List, Union, Optional
##### EndExtImports

##### CppLocalImports
from .....core import KeyRemapData, RemappedKeyData, Ranges, IfContentPart
##### EndCppLocalImports

##### LocalImports
from .BaseRegEdit import BaseRegEdit

if (TYPE_CHECKING):
    from ...ModType import ModType
##### EndLocalImports


##### Script
class RegRemap(BaseRegEdit):
    """
    This class inherits from :class:`BaseRegEdit`

    Bulk-renames the register keys for some :class:`IfContentPart`

    Parameters
    ----------
    keyRemap: Dict[:class:`str`, Union[List[Union[:class:`str`, :class:`RemappedKeyData`]], :class:`KeyRemapData`]]
        The old key -> remap rules mapping to apply :raw-html:`<br />` :raw-html:`<br />`

        See :meth:`IfContentPart.remapKeys` for the full semantics of how a rule set is evaluated
        for a given key's occurrences

    Attributes
    ----------
    keyRemap: Dict[:class:`str`, Union[List[Union[:class:`str`, :class:`RemappedKeyData`]], :class:`KeyRemapData`]]
        The old key -> remap rules mapping to apply :raw-html:`<br />` :raw-html:`<br />`

        See :meth:`IfContentPart.remapKeys` for the full semantics of how a rule set is evaluated
        for a given key's occurrences
    """

    def __init__(self, keyRemap: Dict[str, Union[List[Union[str, RemappedKeyData]], KeyRemapData]]):
        self.keyRemap = keyRemap

    def edit(self, part: IfContentPart, sectionName: str, modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        part.remapKeys(self.keyRemap, ranges = partRanges)
        return part
##### EndScript