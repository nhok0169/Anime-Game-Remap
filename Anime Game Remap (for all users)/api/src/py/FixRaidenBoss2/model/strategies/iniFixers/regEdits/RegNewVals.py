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
from typing import TYPE_CHECKING, Dict, Optional
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
##### EndCppLocalImports

##### LocalImports
from .BaseRegEdit import BaseRegEdit
from ....iftemplate.IfContentPart import IfContentPart

if (TYPE_CHECKING):
    from ...ModType import ModType
##### EndLocalImports


##### Script
class RegNewVals(BaseRegEdit):
    """
    This class inherits from :class:`BaseRegEdit`

    Class for assigning new values to specific registers for some :class:`IfContentPart`

    Parameters
    ----------
    vals: Dict[:class:`str`, :class:`str`]
        Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the register and the values are the new values

    addNewKVPs: :class:`bool`
        Whether to add new `KVPs`_ if the register keys do not exist in the :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    vals: Dict[:class:`str`, :class:`str`]
        Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the register and the values are the new values

    addNewKVPs: :class:`bool`
        Whether to add new `KVPs`_ if the register keys do not exist in the :class:`IfContentPart`
    """

    def __init__(self, vals: Dict[str, str], addNewKVPs: bool = False):
        self.vals = vals
        self.addNewKVPs = addNewKVPs

    def edit(self, part: IfContentPart, sectionName: str, modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        part.replaceVals(self.vals, addNew = self.addNewKVPs, ranges = partRanges)
        return part
##### EndScript