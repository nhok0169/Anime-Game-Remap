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
from typing import TYPE_CHECKING, List, Tuple, Optional
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
class RegAdd(BaseRegEdit):
    """
    This class inherits from :class:`BaseRegEdit`

    Bulk adds some `KVPs`_ into some :class:`IfContentPart`

    Parameters
    ----------
    vals: List[Tuple[:class:`str`, :class:`str`]]
        The `KVPs`_ to add, in the order given

    latest: :class:`bool`
        Whether to add :attr:`vals` at the end of the :class:`IfContentPart` (or, if 'partRanges' is
        provided to :meth:`edit`, at the end of that window), instead of at the beginning :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    vals: List[Tuple[:class:`str`, :class:`str`]]
        The `KVPs`_ to add, in the order given

    latest: :class:`bool`
        Whether to add :attr:`vals` at the end of the :class:`IfContentPart` (or, if 'partRanges' is
        provided to :meth:`edit`, at the end of that window), instead of at the beginning
    """

    def __init__(self, vals: List[Tuple[str, str]], latest: bool = True):
        self.vals = vals
        self.latest = latest

    def edit(self, part: IfContentPart, sectionName: str, modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        if (not self.vals):
            return part

        # no window restriction -- add straight to the true beginning/end of 'part'
        if (partRanges is None):
            if (self.latest):
                part.addKVPs(self.vals)
            else:
                part.addKVPsToFront(self.vals)

            return part

        ranges = partRanges.ranges
        if (not ranges):
            return part

        # add right after the last valid index of the window (unbounded -> the true end of 'part')
        if (self.latest):
            bound = ranges[-1][1]
            insertInd = len(part) if (bound is None) else bound

        # add right before the first valid index of the window (unbounded -> the true beginning of 'part')
        else:
            bound = ranges[0][0]
            insertInd = 0 if (bound is None) else bound

        for i, (key, val) in enumerate(self.vals):
            part.addKVPAt(insertInd + i, key, val)

        return part
##### EndScript
