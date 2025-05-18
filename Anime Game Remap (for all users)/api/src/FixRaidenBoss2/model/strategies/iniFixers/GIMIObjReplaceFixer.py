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
import copy
import os
import re
from typing import Dict, Optional, Set, List, Tuple
##### EndExtImports

##### LocalImports
from ....constants.FileExt import FileExt
from ....constants.IniConsts import IniKeywords
from ....tools.TextTools import TextTools
from ....tools.DictTools import DictTools
from ....tools.HashTools import HashTools
from ....tools.files.FileService import FileService
from .GIMIFixer import GIMIFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
from ...iftemplate.IfContentPart import IfContentPart
from ...iftemplate.IfTemplate import IfTemplate
from .regEditFilters.BaseRegEditFilter import BaseRegEditFilter
from .regEditFilters.RegEditFilter import RegEditFilter
from .regEditFilters.RegTexAdd import RegTexAdd
from ..texEditors.TexCreator import TexCreator
from ...IniSectionGraph import IniSectionGraph
##### EndLocalImports


##### Script
class GIMIObjReplaceFixer(GIMIFixer):
    """
    This class inherits from :class:`GIMIFixer`

    Base class to fix a .ini file used by a GIMI related importer where particular mod objects (head, body, dress, etc...) in the mod to remap are replaced by other mod objectss

    Parameters
    ----------
    parser: :class:`GIMIObjParser`
        The associated parser to retrieve data for the fix

    preRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart`. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`

        Whether these filters reference the mod objects to be fixed of the new mod objects of the fixed mods 
        is determined by :attr:`GIMIObjReplaceFixer.preRegEditOldObj` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    postRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the new mod objects of the fixed mods. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            These filters are preceded by the filters at :attr:`GIMIObjReplaceFixer.preRegEditFilters`

        :raw-html:`<br />`

        **Default**: ``None``

    preRegEditOldObj: :class:`bool`
        Whether the register editting filters at :attr:`GIMIObjReplaceFixer.preRegEditFilters`
        reference the original mod objects of the mod to be fixed or the new mod objects of the fixed mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    postModelRegEditFilters: Optional[List[:class:`RegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the `sections`_ related to the .VB or .IB of a mod
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    preRegEditOldObj: :class:`bool`
        Whether the register editting filters at :attr:`GIMIObjReplaceFixer.preRegEditFilters`
        reference the original mod objects of the mod to be fixed or the new mod objects of the fixed mods

    addedTextures: Dict[:class:`str`, Dict[:class:`str`, Tuple[:class:`str`, :class:`TexCreator`]]]
        The textures to be newly created :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the name of the mod objects
        * The inner keys are the name of the registers
        * The inner values is a tuple that contains:

            # The name of the texture
            # The texture creator for making the new texture

        eg. :raw-html:`<br />`
        ``{"head": {"ps-t1": ("EmptyNormalMap", :class:`TexCreator`(4096, 1024))}, "body": {"ps-t3": ("NewLightMap", :class:`TexCreator`(1024, 1024, :class:`Colour`(0, 128, 0, 255))), "ps-t0": ("DummyShadowRamp", :class:`Colour`())}}``
    """

    def __init__(self, parser: GIMIObjParser, preRegEditFilters: Optional[List[BaseRegEditFilter]] = None, postRegEditFilters: Optional[List[BaseRegEditFilter]] = None,
                 preRegEditOldObj: bool = True, postModelRegEditFilters: Optional[List[RegEditFilter]] = None):
        super().__init__(parser, postModelRegEditFilters = postModelRegEditFilters)
        self._texInds: Dict[str, Dict[str, int]] = {}
        self._texEditRemapNames: Dict[str, Dict[str, str]] = {}
        self._texAddRemapNames: Dict[str, Dict[str, str]] = {}
        self.preRegEditOldObj = preRegEditOldObj

        self.addedTextures: Dict[str, Dict[str, Tuple[str, TexCreator]]] = {}
        self.preRegEditFilters = [] if (preRegEditFilters is None) else preRegEditFilters
        self.postRegEditFilters = [] if (postRegEditFilters is None) else postRegEditFilters

        self._currentTexAddsRegs: Set[str] = set()
        self._currentTexEditRegs: Set[str] = set()
        self._currentRegTexEdits: Dict[str, Tuple[str, str]] = {}

        self._referencedTexEditSections: Dict[str, Set[str]] = {}
        self._referencedTexAdds: Set[str] = set()


    def _combineAddedTextures(self, filters: List[BaseRegEditFilter]):
        for filter in filters:
            if (isinstance(filter, RegTexAdd)):
                self.addedTextures = DictTools.combine(self.addedTextures, copy.deepcopy(filter.textures), 
                                                       lambda modObj, srcObjTextures, currentObjTextures: DictTools.combine(srcObjTextures, currentObjTextures, 
                                                                                                                    lambda reg, srcTexData, currentTexData: currentTexData))

    @property
    def preRegEditFilters(self):
        """
        Filters used to edit the registers of a certain :class:`IfContentPart` for the original mod objects to be fixed. Filters are executed based on the order specified in the list.

        :getter: Retrieves all the sequence of filters
        :setter: Sets the new sequence of filters
        :type: List[:class:`BaseRegEditFilter`]
        """
        
        return self._preRegEditFilters
    
    @preRegEditFilters.setter
    def preRegEditFilters(self, newRegEditFilters: List[BaseRegEditFilter]):
        self._preRegEditFilters = newRegEditFilters
        self._combineAddedTextures(self._preRegEditFilters)
                
    @property
    def postRegEditFilters(self):
        """
        Filters used to edit the registers of a certain :class:`IfContentPart` for the new mod objects of the fixed mods. Filters are executed based on the order specified in the list.

        :getter: Retrieves all the sequence of filters
        :setter: Sets the new sequence of filters
        :type: List[:class:`BaseRegEditFilter`]
        """

        return self._postRegEditFilters
    
    @postRegEditFilters.setter
    def postRegEditFilters(self, newRegEditFilters: List[BaseRegEditFilter]):
        self._postRegEditFilters = newRegEditFilters
        self._combineAddedTextures(self._postRegEditFilters)

    def clear(self):
        """
        Clears all the saved states
        """

        self._texInds = {}
        self._texEditRemapNames = {}
        self._texAddRemapNames = {}

        self._currentTexAddsRegs = set()
        self._currentTexEditRegs = set()
        self._currentRegTexEdits = {}

        self._referencedTexEditSections = {}
        self._referencedTexAdds = set()

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

        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """

        name = TextTools.reverse(name)
    
        nameParts = re.split(TextTools.reverse(objName), name, flags = re.IGNORECASE, maxsplit = 1)
        name = TextTools.reverse(TextTools.capitalizeOnlyFirstChar(newObjName)).join(nameParts)
    
        name = TextTools.reverse(name)

        return self._iniFile.getRemapFixName(name, modName = modName)
    
    def getTexResourceRemapFixName(self, texTypeName: str, oldModName: str, newModName: str, objName: str, addInd: bool = False) -> str:
        """
        Retrieves the new name of the `section`_ for a texture resource that is created/editted

        Parameters
        ----------
        texTypeName: :class:`str`
            The name of the type of texture file

        oldModName: :class:`str`
            The name of the mod to fix from

        newModName: :class:`str`
            The name of the mod to fix to

        objName: :class:`str`
            The mod object the texture resource refereces

        addInd: :class:`bool`
            Whether to add a unique numbered index to the end of the name to distingusih the name
            from other previously created names of the same texture type :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """

        nameParts = [oldModName, objName, texTypeName]
        nameParts = list(map(lambda namePart: TextTools.capitalize(namePart), nameParts))
        nameParts = "".join(nameParts)

        result = self._iniFile.getRemapTexResourceName(nameParts, modName = newModName)

        if (not addInd):
            return result
        
        # retrieve the occurence index of the type of texture resource
        texInd = 0
        try:
            self._texInds[texTypeName]
        except KeyError:
            self._texInds[texTypeName] = {}

        try:
            texInd = self._texInds[texTypeName][objName]
        except KeyError:
            self._texInds[texTypeName][objName] = 0

        self._texInds[texTypeName][objName] += 1
        return f"{result}{texInd}"

    def getObjHashType(self):
        return "ib"
    
    def editTexRegisters(self, modName: str, part: IfContentPart, obj: str, sectionName: str, filters: List[BaseRegEditFilter]):
        """
        Edits the registers for a :class:`IfContentPart` in the texture related `section`_

        .. note::
            For details on steps of how the registers are editted, see :class:`GIMIObjReplaceFixer`

        Parameters
        ----------
        modName: :class:`str`
            The name of the mod to fix to

        part: :class:`IfContentPart`
            The part that is being editted

        obj: :class:`str`
            The name of the mod object for the corresponding part

        sectionName: :class:`str`
            The name of the `section`_ the part belongs to

        filters: List[:class:`BaseRegEditFilter`]
            The filters used for editting the registers
        """

        modType = self._iniFile.availableType
        if (modType is None):
            return
        
        self._currentRegTexEdits = {}
        self._currentTexAddsRegs = set()
        self._currentTexEditRegs = set()

        for filter in filters:
            part = filter.edit(part, modType, modName, obj, sectionName, self)

        texAdds = None
        try:
            texAdds = self.addedTextures[obj]
        except KeyError:
            pass

        # get the referenced texture add resources
        if (texAdds is not None):
            for reg in texAdds:
                if (reg not in self._currentTexAddsRegs):
                    continue
                
                texAddData = texAdds[reg]
                texName = texAddData[0]
                self._referencedTexAdds.add(texName)

        # get the referenced texture edit resources
        for reg in self._currentTexEditRegs:
            texEditData = None
            try:
                texEditData = self._currentRegTexEdits[reg]
            except KeyError:
                continue
            
            texName = texEditData[0]
            texEditSection = texEditData[1]

            texEditSections = None
            try:
                texEditSections = self._referencedTexEditSections[texName]
            except KeyError:
                texEditSections = set()
                self._referencedTexEditSections[texName] = texEditSections

            texEditSections.add(texEditSection)
        
    
    def fillObjOtherHashIndexSection(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, origSectionName: str, objName: str, newObjName: str):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix where the `sections`_ reference some hash or index and the `section`_ is not
        explictely captured by the fixer. The original `sections`_ may come from a different mod object.

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: :class:`IfContentPart`
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
        preRegEditObj = objName if (self.preRegEditOldObj) else newObjName

        newPart = copy.deepcopy(part)
        self.editTexRegisters(modName, newPart, preRegEditObj, sectionName, self._preRegEditFilters)

        for varName, varValue, keyInd, orderInd in newPart:
            # filling in the hash
            if (varName == IniKeywords.Hash.value):
                hashType = self.getObjHashType()
                newHash = self._getHash(hashType, modName)
                newPart.src[varName][keyInd] = (orderInd, f"{newHash}")

            # filling in the subcommand
            elif (varName == IniKeywords.Run.value and varValue != IniKeywords.ORFixPath.value and not varValue.startswith(IniKeywords.TexFxFolder.value)):
                subCommand = self.getObjRemapFixName(varValue, modName, objName, newObjName)
                newPart.src[varName][keyInd] = (orderInd, f"{subCommand}")

            # filling in the index
            elif (varName == IniKeywords.MatchFirstIndex.value):
                newIndex = self._getIndex(newObjName.lower(), modName)
                newPart.src[varName][keyInd] = (orderInd, f"{newIndex}")

        self.editTexRegisters(modName, newPart, newObjName, sectionName, self._postRegEditFilters)
        
        addFix = newPart.toStr(linePrefix = linePrefix)
        if (addFix != ""):
            addFix += "\n"

        return addFix
    
    # fill the attributes for the sections related to the resources
    def _fillTexResource(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, 
                         origSectionName: str, texName: str, oldModName: str, modObjName: str, texGraph: IniSectionGraph):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new `sections`_ created by this fix related to the ``[Resource.*]`` `sections`_
        of a texture file

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the `section`_

        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` of the original ``[Resource.*Blend.*]`` `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        texName: :class:`str`
            The name of the type of texture file

        oldModName: :class:`str`
            The name of the type of mod to fix froms

        modObjName: :class:`str`
            The name of the type of mod object associated to the `section`_

        texGraph: :class:`IniSectionGraph`
            The graph where the `section`_ belongs to

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName, varValue, keyInd, _ in part:
            # filling in the subcommand
            if (varName == IniKeywords.Run.value):
                subCommand = self._getRemapName(sectionName, modName, sectionGraph = texGraph, remapNameFunc = lambda sectionName, modName: self.getTexResourceRemapFixName(texName, oldModName, modName, modObjName))
                subCommandStr = f"{IniKeywords.Run.value} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # add in the file
            elif (varName == "filename"):
                texModel = self._iniFile.texEditModels[texName][origSectionName]
                fixedTexFile = texModel.fixedPaths[partIndex][modName][keyInd]
                addFix += f"{linePrefix}filename = {fixedTexFile}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"

        return addFix
    
    def getTexEditFile(self, file: str, texName: str, modObj: str, modName: str = "") -> str:
        """
        Makes the file path for an editted texture

        Parameters
        ----------
        texFile: :class:`str`
            The file path to the original .dds file

        texName: :class:`str`
            The name for the type of texture

        modObj: :class:`str`
            The name of the mod object the texture file belongs to

        modName: :class:`str`
            The name of the mod to fix to

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapTex.dds file
        """

        basename = os.path.basename(file)
        ind = f"{HashTools.base64DeterministicShortUniqueHash(basename)} {HashTools.base64DeterministicShortUniqueHash(texName)}"

        texFolder = os.path.dirname(file)
        modName = f"{modName}{TextTools.capitalize(modObj)}"
        return os.path.join(texFolder, f"{self._iniFile.getRemapTexName('', modName = modName)}{ind}{FileExt.DDS.value}")
    
    # _fixEdittedTextures(modName, fix): get the fix string for editted textures
    def _fixEdittedTextures(self, modName: str, fix: str = ""):
        self._iniFile.texEditModels.clear()

        # rebuild all the models and the section graphs
        for texName in self._referencedTexEditSections:
            referencedSections = list(self._referencedTexEditSections[texName])
            referencedSections.sort()

            texGraph = self._parser.getTexGraph(texName)
            if (texGraph is None):
                continue

            texGraph.build(newTargetSections = referencedSections, newAllSections = self._iniFile.sectionIfTemplates)
            texEditor = self._parser.getTexEditor(texName)
            if (texEditor is None):
                continue
            
            modObjName = self._parser.texEditRegs[texName][0]
            self._parser._makeTexModels(texName, texGraph, texEditor, getFixedFile = lambda file, modName: self.getTexEditFile(file, texName, modObjName, modName = modName))

        texEditInd = 0
        referencedTexEditLen = len(self._referencedTexEditSections)
        modType = self._iniFile.availableType

        # fix the sections
        for texName in self._referencedTexEditSections:
            texGraph = self._parser.getTexGraph(texName)
            if (texGraph is None):
                continue

            texCommandTuples = texGraph.runSequence
            texCommandsLen = len(texCommandTuples)
            modObjName = self._parser.texEditRegs[texName][0]

            for i in range(texCommandsLen):
                commandTuple = texCommandTuples[i]
                section = commandTuple[0]
                ifTemplate = commandTuple[1]

                resourceName = ""
                try:
                    resourceName = self._texEditRemapNames[section][texName]
                except KeyError:
                    resourceName = self._getRemapName(section, modName, sectionGraph = texGraph, remapNameFunc = lambda sectionName, modName: self.getTexResourceRemapFixName(texName, modType.name, modName, modObjName, addInd = True))

                fix += self.fillIfTemplate(modName, resourceName, ifTemplate, lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: self._fillTexResource(modName, sectionName, part, partIndex, linePrefix, origSectionName, texName, modType.name, modObjName, texGraph), origSectionName = section)

                if (i < texCommandsLen - 1):
                    fix += "\n"

            if (texEditInd < referencedTexEditLen - 1):
                fix += "\n"

            texEditInd += 1

        if (fix and fix[-1] == "\n"):
            fix = fix[:-1]

        return fix
    
    # _makeTexAddResourceIfTemplate(texName, modName, oldModName, modObj): Creates the IfTemplate for an added texture
    def _makeTexAddResourceIfTemplate(self, texName: str, modName: str, oldModName: str, modObj: str) -> IfTemplate:
        sectionName = ""
        try: 
            self._texAddRemapNames[texName]
        except KeyError:
            self._texAddRemapNames[texName] = {}

        try:
            sectionName = self._texAddRemapNames[texName][modObj]
        except KeyError:
            sectionName = self.getTexResourceRemapFixName(texName, oldModName, modName, modObj)
            self._texAddRemapNames[texName][modObj] = sectionName

        filePartName = f"{modName}{TextTools.capitalize(modObj)}{TextTools.capitalize(texName)}"
        filename = f"{self._iniFile.getRemapTexName(filePartName)}{FileExt.DDS.value}"

        return IfTemplate([
            IfContentPart({"filename": [(0, filename)]}, 0)
        ], name = sectionName)

    # _fixAddedTextures(modName, fix): get the fix string for added textures
    def _fixAddedTextures(self, modName: str, fix: str = "") -> str:
        modType = self._iniFile.availableType

        # retrieve the added textures
        for modObj in self.addedTextures:
            objAddedTexs = self.addedTextures[modObj]

            fixedAddedTextures = set()

            # create the needed model and add the new resource
            for reg in objAddedTexs:
                texData = objAddedTexs[reg]
                texName = texData[0]
                texEditor = texData[1]

                if (texName in fixedAddedTextures or texName not in self._referencedTexAdds):
                    continue

                ifTemplate = self._makeTexAddResourceIfTemplate(texName, modName, modType.name, modObj)
                sectionName = ifTemplate.name
                texModel = self._iniFile.makeTexModel(ifTemplate, self._parser._modsToFix, texEditor, getFixedFile = lambda file, modName: file)

                try:
                    self._iniFile.texAddModels[texName]
                except KeyError:
                    self._iniFile.texAddModels[texName] = {}

                self._iniFile.texAddModels[texName][modObj] = texModel

                fix += self.fillIfTemplate(modName, sectionName, ifTemplate, lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: f"{part.toStr(linePrefix = linePrefix)}\n")
                fix += "\n"

                fixedAddedTextures.add(texName)

        if (fix and fix[-1] == "\n"):
            fix = fix[:-1]

        return fix
    
    # _fixDownloadResources(fix): get the fix string for downloaded files
    def _fixDownloadedResources(self, fix: str = "", includeEndNewLine = False):
        fix = super()._fixDownloadedResources(fix = fix, includeEndNewLine = True)

        downloadAdded = False
        referencedDownloads = self._parser._objReferencedDownloads

        for section in referencedDownloads:
            registers = referencedDownloads[section]

            for reg in registers:
                modObj, sectionName = registers[reg]

                ifTemplate = self._iniFile.sectionIfTemplates.get(sectionName)
                fix += self.fillIfTemplate("", sectionName, ifTemplate, lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: f"{part.toStr(linePrefix = linePrefix)}\n")
                fix += "\n"

                if (not downloadAdded):
                    downloadAdded = True

        if (not includeEndNewLine and downloadAdded and fix and fix[-1] == "\n"):
            fix = fix[:-1]

        return fix

    def fixMod(self, modName: str, fix: str = "") -> str:
        self._texEditRemapNames = {}
        self._referencedTexEditSections = {}

        fix = super().fixMod(modName, fix = fix)

        if (self._referencedTexAdds):
            fix += "\n"

        fix = self._fixAddedTextures(modName, fix = fix)

        if (not self._referencedTexAdds and self._referencedTexEditSections):
            fix += "\n"

        if (self._referencedTexEditSections):
            fix += "\n"

        fix = self._fixEdittedTextures(modName, fix = fix)

        if (fix and fix[-1] != "\n"):
            fix += "\n"

        return fix
##### EndScript