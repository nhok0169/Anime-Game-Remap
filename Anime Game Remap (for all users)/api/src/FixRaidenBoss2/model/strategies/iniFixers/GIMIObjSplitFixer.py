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
from typing import Dict, List, Optional
##### EndExtImports

##### LocalImports
from ....tools.ListTools import ListTools
from .GIMIObjReplaceFixer import GIMIObjReplaceFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
from .regEditFilters.BaseRegEditFilter import BaseRegEditFilter
from .regEditFilters.RegEditFilter import RegEditFilter
from .regEditFilters.RegRemap import RegRemap
from .regEditFilters.RegNewVals import RegNewVals
from .regEditFilters.RegRemove import RegRemove
from .regEditFilters.RegTexAdd import RegTexAdd
##### EndLocalImports


##### Script
class GIMIObjSplitFixer(GIMIObjReplaceFixer):
    """
    This class inherits from :class:`GIMIObjReplaceFixer`

    Fixes a .ini file used by a GIMI related importer where particular mod objects (head, body, dress, etc...) in the mod to remap
    are split into multiple mod objects in remapped mod

        
    eg.

    .. code-block::

        KeqingOpulent's "body" is split into Keqing's "body" and "dress"

        KeqingOpulent             Keqing
       ===============       =================
       *** objects ***       **** objects ****
           body  -------+------>   body
           head         |          head
                        +------>   dress    

    .. note::
        For the order of how the registers are fixed, please see :class:`GIMIObjReplaceFixer`

    Parameters
    ----------
    parser: :class:`GIMIObjParser`
        The associated parser to retrieve data for the fix

    objs: Dict[:class:`str`, List[:class:`str`]]
        The mod objects that will be split into multiple new mod objects :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the mod objects to be split and the values are the names of the new mod objects the original mod object will be split into :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            The dictionary keys should align with the defined object names at :meth:`GIMIObjParser.objs` for your parser

        :raw-html:`<br />`

        .. warning::
            If multiple mod objects split into the same object, then the resultant .ini file will contain duplicate `sections`_ for that particular mod object

            eg. :raw-html:`<br />`
            ``{"body": ["dress", "extra"], "head": ["face", "extra"]}``

    preRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart`. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`

        Whether these filters reference the mod objects to be fixed of the new mod objects of the fixed mods 
        is determined by :attr:`GIMIObjSplitFixer.preRegEditOldObj` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    postRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the new mod objects of the fixed mods. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            These filters are preceded by the filters at :class:`GIMIObjReplaceFixer.preRegEditFilters`

        :raw-html:`<br />`

        **Default**: ``None``

    preRegEditOldObj: :class:`bool`
        Whether the register editting filters at :attr:`GIMIObjReplaceFixer.preRegEditFilters`
        reference the original mod objects of the mod to be fixed or the new mod objects of the fixed mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    postModelRegEditFilters: Optional[List[:class:`RegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the `sections`_ related to the .VB or .IB of a mod
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, parser: GIMIObjParser, objs: Dict[str, List[str]], preRegEditFilters: Optional[List[BaseRegEditFilter]] = None, 
                 postRegEditFilters: Optional[List[BaseRegEditFilter]] = None, preRegEditOldObj: bool = False, postModelRegEditFilters: Optional[List[RegEditFilter]] = None):
        super().__init__(parser, preRegEditFilters = preRegEditFilters, postRegEditFilters = postRegEditFilters, preRegEditOldObj = preRegEditOldObj, postModelRegEditFilters = postModelRegEditFilters)
        self.objs = objs


    @property
    def objs(self) -> Dict[str, List[str]]:
        """
        The mods objects that will be split to multiple other mod objects :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the objects in the mod to be remapped and the values are the split objects of the remapped mod

        :getter: Retrieves the mods objects
        :setter: Sets the new objects
        :type: Dict[:class:`str`, List[:class:`str`]]
        """

        return self._objs
    
    @objs.setter
    def objs(self, newObjs: Dict[str, List[str]]):
        self._objs = {}
        for toFixObj in newObjs:
            fixedObjs = newObjs[toFixObj]
            newToFixObj = toFixObj.lower()
            self._objs[newToFixObj] = []

            for fixedObj in fixedObjs:
                newFixedObj = fixedObj.lower()
                self._objs[newToFixObj].append(newFixedObj)

            self._objs[newToFixObj] = ListTools.getDistinct(self._objs[newToFixObj], keepOrder = True)

        # add in the objects that will have their registers editted
        regEditObjs = set()
        for filter in self.preRegEditFilters:
            if (isinstance(filter, RegRemap)):
                regEditObjs.update(set(filter.remap.keys()))
            elif (isinstance(filter, RegRemove)):
                regEditObjs.update(set(filter.remove.keys()))
            elif (isinstance(filter, RegNewVals)):
                regEditObjs.update(set(filter.vals.keys()))
            elif (isinstance(filter, RegTexAdd)):
                regEditObjs.update(set(filter.textures.keys()))

        regEditObjs = regEditObjs.difference(set(self._objs.keys()))
        for obj in regEditObjs:
            cleanedObj = obj.lower()
            self._objs[cleanedObj] = [cleanedObj]


    def _fixOtherHashIndexCommands(self, modName: str, fix: str = ""):
        fixerObjsToFix = set(self.objs.keys())
        objsToFix = list(self._parser.objs.intersection(fixerObjsToFix))
        objsToFix.sort()
        sectionsToIgnore = set()

        # get which section to ignore
        for objToFix in objsToFix:
            objGraph = self._parser.objGraphs[objToFix]
            sectionsToIgnore = sectionsToIgnore.union(objGraph.sections)

        nonBlendCommandTuples = self._parser.otherHashIndexCommandsGraph.runSequence

        for commandTuple in nonBlendCommandTuples:
            section = commandTuple[0]
            ifTemplate = commandTuple[1]

            if (section in sectionsToIgnore):
                continue
            
            self._iniFile._remappedSectionNames.add(section)
            commandName = self._getRemapName(section, modName, sectionGraph = self._parser.otherHashIndexCommandsGraph)
            fix += self.fillIfTemplate(modName, commandName, ifTemplate, self._fillOtherHashIndexSections)
            fix += "\n"

        # retrieve the fix for all the split mod objects
        for objToFix in objsToFix:
            fixedObjs = self.objs[objToFix]
            objGraph = self._parser.objGraphs[objToFix]

            if (not objGraph.sections):
                continue
            
            objGraphTuples = objGraph.runSequence
            for commandTuple in objGraphTuples:
                section = commandTuple[0]
                ifTemplate = commandTuple[1]
                self._iniFile._remappedSectionNames.add(section)

                for fixedObj in fixedObjs:
                    commandName = self.getObjRemapFixName(section, modName, objToFix, fixedObj)
                    fix += self.fillIfTemplate(modName, commandName, ifTemplate, lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: self.fillObjOtherHashIndexSection(modName, sectionName, part, partIndex, linePrefix, origSectionName, objToFix, fixedObj))
                    fix += "\n"

        if (fix and fix[-1] == "\n"):
            fix = fix[:-1]

        return fix
##### EndScript