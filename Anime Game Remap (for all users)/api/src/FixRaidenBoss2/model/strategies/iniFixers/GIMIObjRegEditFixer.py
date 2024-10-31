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
from typing import Dict, Set, Optional
##### EndExtImports

##### LocalImports
from .GIMIObjSplitFixer import GIMIObjSplitFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
##### EndLocalImports


##### Script
class GIMIObjRegEditFixer(GIMIObjSplitFixer):
    """
    This class inherits from :class:`GIMIObjSplitFixer`

    Fixes a .ini file used by a GIMI related importer where particular mod objects (head, body, dress, etc...) in the mod to remap
    needs to have their registers remapped or removed

    Parameters
    ----------
    regRemap: Optional[Dict[:class:`str`, Dict[:class:`str`, List[:class:`str`]]]]
        Defines how the register values in the parts of an :class:`IfTemplate` are mapped to a new register in the remapped mod for particular mod objects :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys is the name of the mod object to have its registers remapped for the fixed mod
        * The inner keys are the names of the registers that hold the register values to be remapped
        * The inner values are the new names of the registers that will hold the register values

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ["new_ps-t2", "new_ps-t3"]}, "body": {"ps-t3": [ps-t0"], "ps-t0": [], "ps-t1": ["ps-t8"]}}`` :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            For this class, :attr:`GIMIObjReplaceFixer.regEditOldObj` is set to ``False``

        :raw-html:`<br />`

        .. note::
            This parameter is preceded by :meth:`GIMIObjSplitFixer.regRemove`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    regRemove: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        Defines whether some register assignments should be removed from the `sections`_ of some mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects to have their registers removed and the values are the names of the register to be removed :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1", "ps-t2"}, "body": {"ps-t3", "ps-t0"}}`` :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            For this class, :attr:`GIMIObjReplaceFixer.regEditOldObj` is set to ``False``

        :raw-html:`<br />`

        .. note::
            This parameter takes precedence over :meth:`GIMIObjSplitFixer.regRemap`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, parser: GIMIObjParser, regRemap: Optional[Dict[str, Dict[str, str]]]= None, regRemove: Optional[Dict[str, Set[str]]] = None):
        super().__init__(parser, {}, regRemap, regRemove)
        self._setupObjFromRegRemap()


    # _setupObjFromRegRemap(): Setup the objects to be split
    # Note: For this class the mod object to be fixed only splits to itself
    def _setupObjFromRegRemap(self):
        self._objs = {}
        for modObj in self._regRemap:
            try:
                self._objs[modObj]
            except KeyError:
                self._objs[modObj] = [modObj]

        for modObj in self._regRemove:
            try:
                self._objs[modObj]
            except KeyError:
                self._objs[modObj] = [modObj]
##### EndScript