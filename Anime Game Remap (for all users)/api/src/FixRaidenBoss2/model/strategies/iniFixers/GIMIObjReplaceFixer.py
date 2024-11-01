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
from typing import Dict, Any, Optional, Set, List
##### EndExtImports

##### LocalImports
from ....constants.IniConsts import IniKeywords
from ....tools.TextTools import TextTools
from .GIMIFixer import GIMIFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
##### EndLocalImports


##### Script
class GIMIObjReplaceFixer(GIMIFixer):
    """
    This class inherits from :class:`GIMIFixer`

    Base class to fix a .ini file used by a GIMI related importer where particular mod objects (head, body, dress, etc...) in the mod to remap are replaced by other mod objects

    Parameters
    ----------
    parser: :class:`GIMIObjParser`
        The associated parser to retrieve data for the fix

    regRemap: Optional[Dict[:class:`str`, Dict[:class:`str`, List[:class:`str`]]]]
        Defines how the register values in the parts of an :class:`IfTemplate` are mapped to a new register in the remapped mod for particular mod objects :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the name of the mod object to have their registers remapped
        * The inner keys are the names of the registers that hold the register values to be remapped
        * The inner values are the new names of the registers that will hold the register values

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ["new_ps-t2", "new_ps-t3"]}, "body": {"ps-t3": [ps-t0"], "ps-t0": [], "ps-t1": ["ps-t8"]}}`` :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            See :attr:`GIMIObjReplaceFixer.regEditOldObj` for whether the mod objects refer to the mod to be fixed or the fixed mods

        :raw-html:`<br />`

        .. note::
            This parameter is preceded by :meth:`GIMIObjSplitFixer.regRemove`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    regRemove: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        Defines whether some register assignments should be removed from the `sections`_ from the mod objects :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects to have their registers removed and the values are the names of the register to be removed :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1", "ps-t2"}, "body": {"ps-t3", "ps-t0"}}`` :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            See :attr:`GIMIObjReplaceFixer.regEditOldObj` for whether the mod objects refer to the mod to be fixed or the fixed mod

        :raw-html:`<br />`

        .. note::
            This parameter takes precedence over :meth:`GIMIObjSplitFixer.regRemap`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    regEditOldObj: :class:`bool`
        Whether the register editting attributes such as :meth:`GIMIObjReplaceFixer.regRemap` or :meth:`GIMIObjReplaceFixer.regRemove` have their mod objects
        reference the original mod objects of the mod to be fixed or the new mod objects of the fixed mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``true``

    Attributes
    ----------
    regEditOldObj: :class:`bool`
        Whether the register editting attributes such as :meth:`GIMIObjReplaceFixer.regRemap` or :meth:`GIMIObjReplaceFixer.regRemove` have their mod objects
        reference the original mod objects of the mod to be fixed or the new mod objects of the fixed mods
    """

    def __init__(self, parser: GIMIObjParser, regRemap: Optional[Dict[str, Dict[str, str]]] = None, regRemove: Optional[Dict[str, Set[str]]] = None, regEditOldObj: bool = True):
        super().__init__(parser)
        self.regEditOldObj = regEditOldObj

        if (regRemap is None):
            regRemap = {}

        if (regRemove is None):
            regRemove = {}

        self.regRemove = regRemove
        self.regRemap = regRemap

    @property
    def regRemap(self):
        """
        Defines how the register values in the parts of an :class:`IfTemplate` are mapped to a new register in the remapped mod for particular mod objects :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the name of the mod objects to have its registers remapped
        * The inner keys are the names of the registers that hold the register values to be remapped
        * The inner values are the new names of the registers that will hold the register values

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ["new_ps-t2", "new_ps-t3"]}, "body": {"ps-t3": [ps-t0"], "ps-t0": [], "ps-t1": ["ps-t8"]}}``

        .. note::
            See :attr:`GIMIObjReplaceFixer.regEditOldObj` for whether the mod objects refer to the mod to be fixed or the fixed mod

        .. note::
            This attribute is preceded by :meth:`GIMIObjSplitFixer.regRemove`

        :getter: Retrieves the remap of the registers for the mod objects
        :setter: Sets the new remap of the registers
        :type: Dict[:class:`str`, Dict[:class:`str`, Set[:class:`str`]]]
        """

        return self._regRemap
    
    @regRemap.setter
    def regRemap(self, newRegRemap: Dict[str, Dict[str, List[str]]]):
        self._regRemap = {}

        for modObj in newRegRemap:
            objRegRemap = newRegRemap[modObj]
            cleanedObjRegRemap = {}

            for oldReg in objRegRemap:
                newRegs = list(map(lambda reg: reg.lower(), objRegRemap[oldReg]))
                cleanedObjRegRemap[oldReg.lower()] = newRegs

            self._regRemap[modObj.lower()] = cleanedObjRegRemap

    @property
    def regRemove(self):
        """
        Defines whether some register assignments should be removed from the `sections`_ of the remapped mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects to have their registers removed and the values are the names of the register to be removed :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1", "ps-t2"}, "body": {"ps-t3", "ps-t0"}}``

        .. note::
            See :attr:`GIMIObjReplaceFixer.regEditOldObj` for whether the mod objects refer to the mod to be fixed or the fixed mod

        .. note::
            This attribute takes precedence over :meth:`GIMIObjSplitFixer.regRemap`

        :getter: Retrieves the registers to be removed for the mod objects
        :setter: Sets the new registers to be removed
        :type: Dict[:class:`str`, Set[:class:`str`]]
        """

        return self._regRemove
    
    @regRemove.setter
    def regRemove(self, newRegRemove: Dict[str, Set[str]]):
        self._regRemove = {}
        for modObj in newRegRemove:
            cleanedObjRegRemap = set(map(lambda reg: reg.lower(), newRegRemove[modObj]))
            self._regRemove[modObj.lower()] = cleanedObjRegRemap

    def getObjRemapFixName(self, name: str, modName: str, objName: str, newObjName: str) -> str:
        """
        Retrieves the new name of the `section`_ for a new mod object

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to be fixed

        objName: :class:`str`
            The name of the original mod object for the `section`_

        newObjName: :class:`str`
            The name of the new mod object for the `section`_
        """

        name = name[:-len(objName)] + TextTools.capitalize(newObjName.lower())
        return self._iniFile.getRemapFixName(name, modName = modName)

    def getObjHashType(self):
        return "ib"

    def remapReg(self, regName: str, regVal: str, objName: str, linePrefix: str = "") -> str:
        """
        Retrieves the new text with 'regVal' remapped to a new register

        Parameters
        ----------
        regName: :class:`str`
            The old name of the register to be remapped

        regVal: :class:`str`
            The value that corresponds to the old name of the register

        objName: :class:`str`
            The name of the mod object where this remap will happend

        linePrefix: :class:`str`
            Any text to prefix the new text created :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The new text with the register value remapped to a new register
        """

        objRegRemap = None
        try:
            objRegRemap = self._regRemap[objName]
        except KeyError:
            return f"{linePrefix}{regName} = {regVal}"
        
        newRegNames = None
        try:
            newRegNames = objRegRemap[regName]
        except KeyError:
            return f"{linePrefix}{regName} = {regVal}"
        
        result = []
        for newReg in newRegNames:
            result.append(f"{linePrefix}{newReg} = {regVal}")
        return "\n".join(result)
    
    def fillObjNonBlendSection(self, modName: str, sectionName: str, part: Dict[str, Any], partIndex: int, linePrefix: str, origSectionName: str, objName: str, newObjName: str):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix that are not related to the ``[TextureOverride.*Blend.*]`` `sections`_
        of some mod object, where the original `section` comes from a different mod object

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: Dict[:class:`str`, Any]
            The content part of the :class:`IfTemplate` of the original [TextureOverrideBlend] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        objName: :class:`str`
            The name of the original mod object

        newObjName: :class:`str`
            The name of the mod object to fix to

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""
        regEditObj = objName if (self.regEditOldObj) else newObjName

        regRemove = None
        try:
            regRemove = self._regRemove[regEditObj]
        except KeyError:
            pass

        for varName in part:
            varValue = part[varName]

            if (regRemove is not None and varName in regRemove):
                continue

            # filling in the hash
            if (varName == IniKeywords.Hash.value):
                hashType = self.getObjHashType()
                newHash = self._getHash(hashType, modName)
                addFix += f"{linePrefix}{IniKeywords.Hash.value} = {newHash}\n"

            # filling in the subcommand
            elif (varName == IniKeywords.Run.value):
                subCommand = self.getObjRemapFixName(varValue, modName, objName, newObjName)
                subCommandStr = f"{IniKeywords.Run.value} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the index
            elif (varName == IniKeywords.MatchFirstIndex.value):
                newIndex = self._getIndex(newObjName.lower(), modName)
                addFix += f"{linePrefix}{IniKeywords.MatchFirstIndex.value} = {newIndex}\n"

            else:
                newLine = self.remapReg(varName, varValue, regEditObj, linePrefix = linePrefix)
                if (newLine != ""):
                    newLine += "\n"
                addFix += newLine

        return addFix
##### EndScript