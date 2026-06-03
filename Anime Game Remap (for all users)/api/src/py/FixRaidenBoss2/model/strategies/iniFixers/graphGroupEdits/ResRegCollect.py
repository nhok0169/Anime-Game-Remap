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
from typing import List, Optional, Tuple, TYPE_CHECKING, Callable, Dict
##### EndExtImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from ....IniGraphGroup import IniGraphGroup
from ....iftemplate.IfContentPart import IfContentPart
from .ResEdit import BaseResEdit

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class ResRegCollect(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Creates the :class:`IniSectionGraph` for a particular resource

    Attributes
    ----------
    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

        The keys in the dictionary are the location of which :class:`IniSectionGraph` to search for, which contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    resEdit: :class:`BaseResEdit`
        Describes for how a resource should be built

    predicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`IfContentPart`], :class:`bool`]]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`
        
        The keys are the location in which :class:`IniSectionGraph` the predicate applies to 
        when searching for the corresponding resource from :attr:`srcRegs` and the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

        Each predicate takes in:

        #. The register name that holds the reference
        #. The name of the resource reference
        #. The part that contains the reference to the resource

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Parameters
    ----------
    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

        The keys in the dictionary are the location of which :class:`IniSectionGraph` to search for, which contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    resEdit: :class:`BaseResEdit`
        Describes for how a resource should be built

    predicates: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`IfContentPart`], :class:`bool`]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`
        
        The keys are the location in which :class:`IniSectionGraph` the predicate applies to 
        when searching for the corresponding resource from :attr:`srcRegs` and the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

        Each predicate takes in:

        #. The register name that holds the reference
        #. The name of the resource reference
        #. The part that contains the reference to the resource

    collectedSections: List[:class:`str`]
        The names of the found references to the resource
    """


    def __init__(self, srcRegs: Dict[Tuple[int, str, str], str], resEdit: BaseResEdit,
                 predicates: Optional[Dict[Tuple[int, str, str], Callable[[str, str, IfContentPart], bool]]] = None):
        self.srcRegs = srcRegs
        self.predicates = predicates if (predicates is not None) else {}
        self.resEdit = resEdit
        
        self.collectedSections: Dict[str, str] = {}

    def clear(self):
        self.collectedSections.clear()
        self.resEdit.clear()

    def _collectFromGraphGroup(self, graphGroups: List[IniGraphGroup], srcModObj: Tuple[int, str, str], srcReg: str, modType: "ModType", modName: str = ""):
        iniInd, srcComp, srcObj = srcModObj
        
        if (iniInd >= len(graphGroups)):
            return graphGroups
        
        graphGroup = graphGroups[iniInd]
        graph = graphGroup.graphs.get((srcComp, srcObj))

        if (graph is None):
            return graphGroups
        
        predicate = self.predicates.get(srcModObj)
        
        for sectionName, part, state in graph.iterByContentPart():
            regVals = part.get(srcReg, default = [])
            regValsLen = len(regVals)

            for i in range(regValsLen):
                ind, val = regVals[i]
                if (predicate is not None and not predicate(srcReg, val, part)):
                    continue

                newVal = self.resEdit.getFixResourceName(val, modType, modName = modName)
                if (newVal is not None):
                    regVals[i] = (ind, newVal)
                    val, newVal = self.resEdit.collectResourceName(val, newVal)
                else:
                    val, newVal = self.resEdit.collectResourceName(val, val)

                self.collectedSections[val] = newVal

        return graphGroups

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        self.clear()

        for srcModObj in self.srcRegs:
            srcReg = self.srcRegs[srcModObj]
            graphGroups = self._collectFromGraphGroup(graphGroups, srcModObj, srcReg, modType, modName = modName)

        return graphGroups

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        self.edit(graphGroups, modType, modName = modName)
        graphGroups = self.resEdit.buildResources(self.collectedSections, modType, ini, graphGroups, modName = modName)
        self.clear()
        return graphGroups
##### EndScript