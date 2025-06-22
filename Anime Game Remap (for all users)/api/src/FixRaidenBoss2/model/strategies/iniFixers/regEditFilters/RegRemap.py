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
from typing import Optional, Dict, List, Set, TYPE_CHECKING, Any, Union, Callable, Tuple
##### EndExtImports

##### LocalImports
from .RegEditFilter import RegEditFilter
from ....iftemplate.IfContentPart import IfContentPart

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ..BaseIniFixer import BaseIniFixer
    from ..GIMIObjReplaceFixer import GIMIObjReplaceFixer
##### EndLocalImports


##### Script
class RegRemap(RegEditFilter):
    """
    This class inherits from :class:`RegEditFilter`

    Class for remapping the register keys for some :class:`IfContentPart`

    Parameters
    ----------
    remap: Optional[Dict[:class:`str`, Dict[:class:`str`, List[Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`, :class:`str`], :class:`bool`]]]]]]]
        Defines how the register values in the parts of an :class:`IfTemplate` are mapped to a new register in the remapped mod for particular mod objects :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the name of the mod object to have their registers remapped
        * The inner keys are the names of the registers that hold the register values to be remapped
        * The inner values are the new names of the registers that will hold the register values

            * If given a string, will remap the register to the new name
            * If given a tuple containing a string and a predicate, will remap the register to the new name only if the predicate returns ``True`` for the register value

              The predicate takes in:

              #. The old register key
              #. The correspondnig value for the old register key

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ["new_ps-t2", "new_ps-t3"]}, "body": {"ps-t3": [ps-t0"], "ps-t0": [], "ps-t1": [("ps-t8", lambda reg, val: val.find("NormalMap") != -1)]}}`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    keepKeysWithoutRemap: :class:`bool`
        Whether to keep the keys that do not get remapped :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    remap: Dict[:class:`str`, Dict[:class:`str`, List[Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`, :class:`str`], :class:`bool`]]]]]]
        Defines how the register values in the parts of an :class:`IfTemplate` are mapped to a new register in the remapped mod for particular mod objects :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the name of the mod objects to have its registers remapped
        * The inner keys are the names of the registers that hold the register values to be remapped
        * The inner values are either:
         
            * the new names of the registers that will hold the register values OR
            * a tuple of the new name of the register and a callable that checks if the register should be remapped

              The predicate takes in:

              #. The old register key
              #. The correspondnig value for the old register key

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ["new_ps-t2", "new_ps-t3"]}, "body": {"ps-t3": [ps-t0"], "ps-t0": [], "ps-t1": [("ps-t8", lambda reg, val: val.find("NormalMap") != -1)]}}`` :raw-html:`<br />` :raw-html:`<br />`

    keepKeysWithoutRemap: :class:`bool`
        Whether to keep the keys that do not get remapped

    _regRemap: Optional[Dict[:class:`str`, List[:class:`str`]]]
        The register remap to do on the current :class:`IfContentPart` being parsed :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the registers and the values are the newly mapped registers
    """

    def __init__(self, remap: Optional[Dict[str, Dict[str, List[Union[str, Tuple[str, Callable[[str, str], bool]]]]]]] = None, keepKeysWithoutRemap: bool = False):
        self.remap = {} if (remap is None) else remap
        self._regRemap: Optional[Dict[str, List[str]]] = None
        self.keepKeysWithoutRemap = keepKeysWithoutRemap

    def clear(self):
        self._regRemap = None
    
    def _editReg(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "BaseIniFixer") -> IfContentPart:
        try:
            self._regRemap = self.remap[obj]
        except KeyError:
            return part

        part.remapKeys(self._regRemap, keepKeysWithoutRemap = self.keepKeysWithoutRemap)
        return part
    
    def _handleTex(self, currentTexRegs: Set[str], currentTexRegData: Optional[Dict[str, Any]] = None):
        if (self._regRemap is None):
            return

        for reg in self._regRemap:
            if (reg in currentTexRegs):
                currentTexRegs.remove(reg)
                newTexRegs = set(map(lambda newReg: newReg if (isinstance(newReg, str)) else newReg[0], self._regRemap[reg]))
                currentTexRegs.update(newTexRegs)

            if (currentTexRegData is None or reg not in currentTexRegData):
                continue

            newRegs = self._regRemap[reg]
            for newRegKey in newRegs:
                newReg = newRegKey if (isinstance(newRegKey, str)) else newRegKey[0]
                currentTexRegData[newReg] = currentTexRegData[reg]
    
    def handleTexAdd(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        addedTextures = None
        try:
            addedTextures = fixer.addedTextures[obj]
        except KeyError:
            pass

        self._handleTex(fixer._currentTexAddsRegs, addedTextures)

    
    def handleTexEdit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        self._handleTex(fixer._currentTexEditRegs, fixer._currentRegTexEdits)
##### EndScript
