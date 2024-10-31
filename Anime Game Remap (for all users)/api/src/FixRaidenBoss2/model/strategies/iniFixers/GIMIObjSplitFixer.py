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
from typing import Dict, List, Optional, Set
##### EndExtImports

##### LocalImports
from ....tools.ListTools import ListTools
from .GIMIObjReplaceFixer import GIMIObjReplaceFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
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

    def __init__(self, parser: GIMIObjParser, objs: Dict[str, List[str]], regRemap: Optional[Dict[str, Dict[str, str]]] = None, regRemove: Optional[Dict[str, Set[str]]] = None):
        super().__init__(parser, regRemap = regRemap, regRemove = regRemove, regEditOldObj = False)
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


    def _fixNonBlendHashIndexCommands(self, modName: str, fix: str = ""):
        objsToFix = self._parser.objs.intersection(set(self.objs.keys()))
        sectionsToIgnore = set()

        # get which section to ignore
        for objToFix in objsToFix:
            objGraph = self._parser.objGraphs[objToFix]
            sectionsToIgnore = sectionsToIgnore.union(objGraph.sections)

        nonBlendCommandTuples = self._parser.nonBlendHashIndexCommandsGraph.runSequence
        for commandTuple in nonBlendCommandTuples:
            section = commandTuple[0]
            ifTemplate = commandTuple[1]

            if (section in sectionsToIgnore):
                continue

            commandName = self._getRemapName(section, modName, sectionGraph = self._parser.nonBlendHashIndexCommandsGraph)
            fix += self.fillIfTemplate(modName, commandName, ifTemplate, self._fillNonBlendSections)
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

                for fixedObj in fixedObjs:
                    commandName = self.getObjRemapFixName(section, modName, objToFix, fixedObj)
                    fix += self.fillIfTemplate(modName, commandName, ifTemplate, lambda modName, sectionName, part, partIndex, linePrefix, origSectionName: self.fillObjNonBlendSection(modName, sectionName, part, partIndex, linePrefix, origSectionName, objToFix, fixedObj))
                    fix += "\n"

        return fix  
##### EndScript