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
from typing import Dict, List, Union, Set, Optional, Tuple
##### EndExtImports

##### LocalImports
from ....constants.FileSuffixes import FileSuffixes
from ....tools.DictTools import DictTools
from .GIMIObjReplaceFixer import GIMIObjReplaceFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
from .regEditFilters.BaseRegEditFilter import BaseRegEditFilter
from .regEditFilters.RegEditFilter import RegEditFilter
##### EndLocalImports


##### Script
class GIMIObjMergeFixer(GIMIObjReplaceFixer):
    """
    This class inherits from :class:`GIMIObjReplaceFixer`

    Fixes a .ini file used by a GIMI related importer where particular mod objects (head, body, dress, etc...) in the mod to remap are merged to a single mod object

    eg. 

    .. code-block::

        Keqing's "body" and "dress" are merged into KeqingOpulent's "body"

           Keqing             KeqingOpulent
       ===============       =================
       *** objects ***       **** objects ****
           body      -----+---->   body
           head           |        head
           dress     -----+  

    .. note::
        This class takes advantage of GIMI's bug/feature of overlapping mods from loading multiple mods of the same character by creating different variations of the original .ini file

    Parameters
    ----------
    parser: :class:`GIMIObjParser`
        The associated parser to retrieve data for the fix

    objs: Dict[:class:`str`, List[:class:`str`]]
        The mod objects to be merged to a single mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the merged objects and the values are the names of the mod objects to be merged :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            The dictionary values should align with the defined object names at :meth:`GIMIObjParser.objs` for your parser

    copyPreamble: :class:`str`
        Any text we want to put before the text of the newly generated .ini file variations :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``""``

    preRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart`. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`

        Whether these filters reference the mod objects to be fixed of the new mod objects of the fixed mods 
        is determined by :attr:`GIMIObjMergeFixer.preRegEditOldObj` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    postRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the new mod objects of the fixed mods. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            These filters are preceded by the filters at :class:`GIMIObjReplaceFixer.preRegEditFilters`

        :raw-html:`<br />`

        **Default**: ``None``

    iniPostModelRegEditFilters: Optional[List[List[:class:`RegEditFilter`]]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the `sections`_ related to the .VB or .IB of a mod for each .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _targetObjs: Dict[:class:`str`, :class:`str`]
        Which original mod objects to show for each merged mod object in the current .ini file :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the original mod objects to display on the current .ini file and the values are the names of the merged objects.

    copyPreamble: :class:`str`
        Any text we want to put before the text of the newly generated .ini file variations

    iniPostModelRegEditFilters: List[List[:class:`RegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the `sections`_ related to the .VB or .IB of a mod for each .ini file

    postModelRegEditFilters: List[:class:`RegEditFilter`]
        The filters used to edit the registers of a certain :class:`IfContentPart` for the `sections`_ related to the .VB or .IB of a mod for the current .ini file being generated
    """

    def __init__(self, parser: GIMIObjParser, objs: Dict[str, List[str]], copyPreamble: str = "", 
                 preRegEditFilters: Optional[List[BaseRegEditFilter]] = None, postRegEditFilters: Optional[List[BaseRegEditFilter]] = None,
                 iniPostModelRegEditFilters: Optional[List[List[RegEditFilter]]] = None):

        self.iniPostModelRegEditFilters = [] if (iniPostModelRegEditFilters is None) else iniPostModelRegEditFilters

        super().__init__(parser, preRegEditFilters = preRegEditFilters, postRegEditFilters = postRegEditFilters)
        self._targetObjs: List[Tuple[str, str]]
        self._maxObjsToMergeLen = 0
        self._sectionsToIgnore: Set[str] = set()
        self.objs = objs

        self.copyPreamble = copyPreamble

    @property
    def objs(self):
        """
        The mod objects to be merged to a single mod object :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the merged objects and the values are the names of the mod objects to be merged

        :getter: Retrieves the mod objects to be merged
        :setter: Set the new mod objects to be merged
        :type: Dict[:class:`str`, List[:class:`str`]]
        """

        return self._objs
    
    @objs.setter
    def objs(self, newObjs: Dict[str, List[str]]):
        self._objs = {}

        for mergedObj in newObjs:
            objsToMerge = newObjs[mergedObj]
            self._objs[mergedObj] = objsToMerge
            self._maxObjsToMergeLen = max(self._maxObjsToMergeLen, len(objsToMerge))


    def _fixOtherHashIndexCommands(self, modName: str, fix: str = ""):
        nonBlendCommandTuples = self._parser.otherHashIndexCommandsGraph.runSequence

        for commandTuple in nonBlendCommandTuples:
            section = commandTuple[0]
            ifTemplate = commandTuple[1]

            if (section in self._sectionsToIgnore):
                continue
            
            self._iniFile._remappedSectionNames.add(section)
            commandName = self._getRemapName(section, modName, sectionGraph = self._parser.otherHashIndexCommandsGraph)
            fix += self.fillIfTemplate(modName, commandName, ifTemplate, self._fillOtherHashIndexSections)
            fix += "\n"

        # retrieve the fix for all the merged mod objects
        for objToFix, fixedObj in self._targetObjs:
            objGraph = self._parser.objGraphs[objToFix]

            if (not objGraph.sections):
                continue
            
            objGraphTuples = objGraph.runSequence
            for commandTuple in objGraphTuples:
                section = commandTuple[0]
                ifTemplate = commandTuple[1]
                commandName = self.getObjRemapFixName(section, modName, objToFix, fixedObj)
                self._iniFile._remappedSectionNames.add(section)
                fix += self.fillIfTemplate(modName, commandName, ifTemplate, lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: self.fillObjOtherHashIndexSection(modName, sectionName, part, partIndex, linePrefix, origSectionName, objToFix, fixedObj))
                fix += "\n"

        if (fix and fix[-1] == "\n"):
            fix = fix[:-1]

        return fix

    
    # _getCurrentTargetObjs(ind): Retrieves the current mod objects to show in the current .ini file for each merged mod object
    def _getCurrentTargetObjs(self, ind: int):
        self._targetObjs = []
        for mergedObj in self._objs:
            objsToMerge = self._objs[mergedObj]
            if (ind <= len(objsToMerge) - 1):
                objToMerge = objsToMerge[ind]
                self._targetObjs.append((objToMerge, mergedObj))

    # _getCurrentModelRegEditFilters(ind): Retrieves the current register editting filters to edit the sections to the .ib/.vb of the mod
    def _getCurrentModelRegEditFilters(self, ind: int):
        if (len(self.iniPostModelRegEditFilters) <= ind):
            self.postModelRegEditFilters = []
            return
        
        self.postModelRegEditFilters = self.iniPostModelRegEditFilters[ind]

    # _getIgnoredSections(): Retrieves which sections to ignore when performing the normal part of the fix
    def _getIgnoredSections(self):
        objsToFix = copy.deepcopy(self._parser.objs)
        ignoredObjs = set()
        self._sectionsToIgnore = set()

        # get which section to ignore
        for mergedObj in self._objs:
            objsToFix = self._objs[mergedObj]

            for objToFix in objsToFix:
                if (objToFix in ignoredObjs):
                    continue

                ignoredObjs.add(objToFix)
                objGraph = None
                try:
                    objGraph = self._parser.objGraphs[objToFix]
                except:
                    continue

                self._sectionsToIgnore = self._sectionsToIgnore.union(objGraph.sections)


    def _fix(self, keepBackup: bool = True, fixOnly: bool = False, update: bool = False, hideOrig: bool = False, withBoilerPlate: bool = True, withSrc: bool = True, fixId: int = 0) -> Union[str, List[str]]:
        result = []
        iniFilePath = self._iniFile.filePath
        iniBaseName = iniFilePath.baseName
        self._getIgnoredSections()
        self._iniFile._remappedSectionNames.update(self._sectionsToIgnore)

        texEditModels = {}
        for i in range(self._maxObjsToMergeLen):
            self._getCurrentTargetObjs(i)
            self._getCurrentModelRegEditFilters(i)

            if (i > 0 and iniFilePath is not None):
                iniFilePath.baseName = f"{iniBaseName}{FileSuffixes.RemapFixCopy.value}{i}"

            currentResult = super()._fix(keepBackup = keepBackup, fixOnly = fixOnly, update = update, hideOrig = hideOrig, withBoilerPlate = withBoilerPlate, withSrc = withSrc, fixId = fixId + i)
            currentTexEditModels = DictTools.update(texEditModels, self._iniFile.texEditModels, lambda modelName, resModels, curResModels: DictTools.combine(resModels, curResModels, lambda sectionName, model, curModel: curModel))

            if (i > 0 and withSrc and self.copyPreamble != ""):
                currentResult = f"{self.copyPreamble}\n\n{currentResult}"

            self._iniFile.write(txt = currentResult)
            result.append(currentResult)

        self._iniFile.texEditModels = currentTexEditModels
        iniFilePath.baseName = iniBaseName
        if (len(result) == 1):
            result = result[0]

        return result
##### EndScript
