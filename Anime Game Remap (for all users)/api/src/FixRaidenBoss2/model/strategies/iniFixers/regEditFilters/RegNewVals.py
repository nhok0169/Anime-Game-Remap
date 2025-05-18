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
from typing import Optional, Dict, TYPE_CHECKING, Union, Callable
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
class RegNewVals(RegEditFilter):
    """
    This class inherits from :class:`RegEditFilter`

    Class for assigning new values to specific registers for some :class:`IfContentPart`

    Parameters
    ----------
    vals: Optional[Dict[:class:`str`, Dict[:class:`str`,Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`], :class:`bool`]]]]]]
        Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the new mod objects where the registers are found
        * The inner keys are the new names of the registers to have their values changed
        * The inner values contains either

            * A string representing the new changed values for all instances of the register OR
            * A tuple containing a string and a predicate, representing the new changed values for only certain instances of the registers.
              The predicate takes the old value of the register as the argument.

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": "newVal"}, "body": {"ps-t3": "newVal2", "ps-t0": "newVal3"}, "dress": {"ps-t0": ("newVal4", lambda val: val == "replaceMe")}}`` :raw-html:`<br />` :raw-html:`<br />`


        **Default**: ``None``

    Attributes
    ----------
    vals: Dict[:class:`str`, Dict[:class:`str`,Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`], :class:`bool`]]]]]
       Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the new mod objects where the registers are found
        * The inner keys are the new names of the registers to have their values changed
        * The inner values contains either

            * A string representing the new changed values for all instances of the register OR
            * A tuple containing a string and a predicate, representing the new changed values for only certain instances of the registers.
              The predicate takes the old value of the register as the argument.

    _regUpdates: Optional[Dict[:class:`str`, :class:`str`]]
        The value updates to do on the current :class:`IfContentPart` being parsed :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the registers and the values are the corresponding values to the registers
    """

    def __init__(self, vals: Optional[Dict[str, Dict[str, Union[str, Callable[[str], bool]]]]] = None):
        self.vals = {} if (vals is None) else vals
        self._regUpdates: Optional[Dict[str, str]] = None

    def clear(self):
        self._regUpdates = None
    
    def _editReg(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "BaseIniFixer") -> IfContentPart:
        try:
            self._regUpdates = self.vals[obj]
        except KeyError:
            return part

        part.replaceVals(self._regUpdates, addNewKVPs = False)
        return part
    
    def handleTexAdd(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regUpdates is not None):
            fixer._currentTexAddsRegs = fixer._currentTexAddsRegs.difference(set(self._regUpdates.keys()))

    def handleTexEdit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        if (self._regUpdates is not None):
            fixer._currentTexEditRegs = fixer._currentTexEditRegs.difference(set(self._regUpdates.keys()))
##### EndScript
