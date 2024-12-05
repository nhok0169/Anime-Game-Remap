##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from typing import Optional, Dict, Set, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .RegEditFilter import RegEditFilter
from ....iftemplate.IfContentPart import IfContentPart
from ...ModType import ModType

if (TYPE_CHECKING):
    from ..GIMIObjReplaceFixer import GIMIObjReplaceFixer
##### EndLocalImports


##### Script
class RegRemove(RegEditFilter):
    """
    This class inherits from :class:`RegEditFilter`

    Class for removing keys from a :class:`IfContentPart`

    Parameters
    ----------
    remove: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        Defines whether some register assignments should be removed from the `sections`_ from the mod objects :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects to have their registers removed and the values are the names of the register to be removed :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1", "ps-t2"}, "body": {"ps-t3", "ps-t0"}}`` :raw-html:`<br />` :raw-html:`<br />`s

        **Default**: ``None``

    Attributes
    ----------
    remove: Dict[:class:`str`, Set[:class:`str`]]
        Defines whether some register assignments should be removed from the `sections`_ of the remapped mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects to have their registers removed and the values are the names of the register to be removed :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1", "ps-t2"}, "body": {"ps-t3", "ps-t0"}}``

    _regRemove: Optional[Set[:class:`str`]]
        The register removal to do on the current :class:`IfContentPart` being parsed
    """

    def __init__(self, remove: Optional[Dict[str, Set[str]]] = None):
        self.remove = {} if (remove is None) else remove
        self._regRemove: Optional[Set[str]] = None

    def clear(self):
        self._regRemove = None
    
    def _editReg(self, part: IfContentPart, modType: ModType, fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer") -> IfContentPart:
        try:
            self._regRemove = self.remove[obj]
        except KeyError:
            return part

        part.removeKeys(self._regRemove)
        return part
    
    def handleTexAdd(self, part: IfContentPart, modType: ModType, fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regRemove is not None):
            fixer._currentTexAddsRegs = fixer._currentTexAddsRegs.difference(self._regRemove)
    
    def handleTexEdit(self, part: IfContentPart, modType: ModType, fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regRemove is not None):
            fixer._currentTexEditRegs = fixer._currentTexEditRegs.difference(self._regRemove)
##### EndScript
