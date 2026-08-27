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
from collections import defaultdict
from typing import List, Optional, Tuple, TYPE_CHECKING, Callable, Dict, Set, Union, DefaultDict
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
from .....core import IniGraphGroup
##### EndCppLocalImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from .....core import SectionIterData
from .....core import IniSectionGraph
from .GraphGroupRemap import GraphGroupRemap
from .ResEdit import BaseResEdit
from .....tools.DictTools import DictTools

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class ResRegCollect(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Creates the :class:`IniSectionGraph` for a particular resource

    Parameters
    ----------
    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

        The keys in the dictionary are the location of which :class:`IniSectionGraph` to search for, which contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    resEdits: Dict[:class:`str`, :class:`BaseResEdit`]
        Describes for how a resource should be built :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names for the subtype of the resource and the values are the edit for each type of resource

    partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`bool`]]]
        The preciates for which particular order indices to process some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        The keys are the location in which :class:`IniSectionGraph` the predicate applies to and the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    resPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`
        
        The keys are the location in which :class:`IniSectionGraph` the predicate applies to 
        when searching for the corresponding resource from :attr:`srcRegs` and the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

        Each predicate takes in:

        #. The register name that holds the reference
        #. The name of the resource reference
        #. The data that contains info on the part and its `section`_

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    remaps: Optional[Dict[:class:`str`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
        Whether to remap the remap the graphs searched from :attr:`srcRegs`. The values follow the same format as :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the subtype for the resource
        * The inner keys are the location of the :class:`IniSectionGraph`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Whether to track the `KVPs`_ in the .ini file when searching for particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the dictionary contain:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values of the dictionary are the values of the flag
        
        :raw-html:`<br />`

        **Default**: ``False`` 

    keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Specific keys to track in the .ini file when searching particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the dictionary contain:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the keys to track for each graph. If the value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart` for that graph

        :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

        The keys in the dictionary are the location of which :class:`IniSectionGraph` to search for, which contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    resEdits: Dict[:class:`str`, :class:`BaseResEdit`]
        Describes for how a resource should be built :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names for the type of resource and the values are the edit for each type of resource

    partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`bool`]]]
        The preciates for which particular order indices to process some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        The keys are the location in which :class:`IniSectionGraph` the predicate applies to and the values are the predicates

    resPredicates: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`
        
        The keys are the location in which :class:`IniSectionGraph` the predicate applies to 
        when searching for the corresponding resource from :attr:`srcRegs` and the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

        Each predicate takes in:

        #. The register name that holds the reference
        #. The name of the resource reference
        #. The data that contains info on the part and its `section`_

    remaps: Dict[:class:`str`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]
        Whether to remap the remap the graphs searched from :attr:`srcRegs`. The values follow the same format as :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the subtype for the resource
        * The inner keys are the location of the :class:`IniSectionGraph`

    resCalls: DefaultDict[Tuple[:class:`int`, :class:`str`, :class:`str`], DefaultDict[:class:`str`, DefaultDict[:class:`int`, Dict[Tuple[:class:`str`, :class:`int`], Tuple[:class:`int`, :class:`str`]]]]]
        The calls to the resource

        * The outer keys are tuples that contain the index of the ini file, the name of the component and the name of the mod object
        * The second outer keys are the names of the `sections`_
        * The third outer keys contains the id of the part within the `section`_
        * The inner keys are tuples that contain the name of the register that called the resource and the occurence index of the register in the part
        * The values are tuples that contain the index the resource call is found in the part and the name of the resource `section`_

    trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Whether to track the `KVPs`_ in the .ini file when searching for particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the dictionary contain:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values of the dictionary are the values of the flag

    keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Specific keys to track in the .ini file when searching particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the dictionary contain:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the keys to track for each graph. If the value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart` for that graph
    """

    def __init__(self, srcRegs: Dict[Tuple[int, str, str], str], resEdits: Dict[str, BaseResEdit],
                 partPredicates: Optional[Dict[Tuple[int, str, str], Callable[[SectionIterData], Ranges]]] = None,
                 resPredicates: Optional[Dict[Tuple[int, str, str], Callable[[str, str, SectionIterData], bool]]] = None,
                 remaps: Optional[Dict[Tuple[int, str, str], Dict[str, Union[Tuple[int, str, str], Tuple[int, str, str, Callable[[str], str]]]]]] = None,
                 trackKeys: Union[bool, Dict[Tuple[int, str, str], bool]] = False, keysToTrack: Optional[Dict[Tuple[int, str, str], Optional[Set[str]]]] = None):

        self.srcRegs = srcRegs
        self.partPredicates = partPredicates if (partPredicates is not None) else {}
        self.resPredicates = resPredicates if (resPredicates is not None) else {}
        self.resEdits = resEdits
        self.remaps = remaps
        self.trackKeys = trackKeys
        self.keysToTrack = {} if (keysToTrack is None) else keysToTrack

        self.resCalls: DefaultDict[Tuple[int, str, str], DefaultDict[str, DefaultDict[int, Dict[int, str]]]] = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: {})))

    def clear(self):
        self.resCalls.clear()
        
        for resSubType in self.resEdits:
            self.resEdits[resSubType].clear()

    def _collectFromGraphGroup(self, graphGroups: List[IniGraphGroup], srcModObj: Tuple[int, str, str], srcReg: str):
        graph = self.getGraph(graphGroups, srcModObj, errorOnNotFound = False)
        if (graph is None):
            return graphGroups

        partPredicate = self.partPredicates.get(srcModObj)
        resPredicate = self.resPredicates.get(srcModObj)
        keysToTrack = self.keysToTrack.get(srcModObj)

        trackKeys = self.trackKeys
        if (not isinstance(trackKeys, bool)):
            trackKeys = self.trackKeys.get(srcModObj, False)
        
        for iterData in graph.iterByContentPart(colour = trackKeys, colourKeys = keysToTrack):
            part = iterData.part
            partRanges = None if (partPredicate is None) else partPredicate(iterData)

            regVals = part.get(srcReg, default = [], withInds = True, ranges = partRanges)
            sectionName = iterData.sectionName

            for ind, val in regVals:
                if (resPredicate is not None and not resPredicate(srcReg, val, iterData)):
                    continue

                self.resCalls[srcModObj][sectionName][part.id][ind] = val

        return graphGroups
    
    def _remapGraph(self, fromGraph: IniSectionGraph, fromModObj: Tuple[int, str, str], toModObj: Tuple[int, str, str], remappedGraphs: DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str]]]], 
                    resTypes: DefaultDict[Tuple[int, str, str], List[str]], fromModObjOccurences: DefaultDict[Tuple[int, str, str], int], 
                    renameFunc: Optional[Callable[[str], str]] = None):
        result = fromGraph.deepcopy(newPartIds = False)

        resTypeInd = fromModObjOccurences[fromModObj]
        resType = resTypes[fromModObj][resTypeInd]
        remappedGraphs[fromModObj][resType] = (result, renameFunc, True)

        fromModObjOccurences[fromModObj] += 1

        return result
    
    def _remapGraphs(self, graphGroups: List[IniGraphGroup], remappedGraphs: Optional[DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str]]]]]) -> List[IniGraphGroup]:
        if (self.remaps is None):
            return graphGroups
        
        remap = defaultdict(lambda: [])
        resTypes = defaultdict(lambda: [])
        srcModObjOccurences = defaultdict(lambda: 0)

        for keys, values in DictTools.iterDict(self.remaps, ["srcModObj", "resSubType"]):
            srcModObj = keys["srcModObj"]
            resType = keys["resSubType"]
            toModObj = values["resSubType"]

            remap[srcModObj].append(toModObj)
            resTypes[srcModObj].append(resType)

        graphGroupRemap = GraphGroupRemap(remap)
        graphGroups = graphGroupRemap.remapGraphs(graphGroups, lambda fromGraph, fromObj, toObj, renameFunc: self._remapGraph(fromGraph, fromObj, toObj, remappedGraphs, resTypes, srcModObjOccurences, renameFunc = renameFunc))
        return graphGroups

    def _collectResourceNames(self, resType: str, resEdit: BaseResEdit, collectedSections: Dict[str, str], graphGroups: List[IniGraphGroup], modType: "ModType", 
                              remappedGraphs: Optional[DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str]]]]] = None, 
                              editGraph: bool = False, modName: str = ""):

        hasRemappedGraphs = remappedGraphs is not None
        
        for fromModObj in self.resCalls:
            graphResCalls = self.resCalls[fromModObj]
            renameFunc = None
            partIdRefreshRequired = False
            toGraph = remappedGraphs.get(fromModObj) if (hasRemappedGraphs) else self.getGraph(graphGroups, fromModObj, errorOnNotFound = False)

            if (hasRemappedGraphs and toGraph is not None):
                toGraph, renameFunc, partIdRefreshRequired = toGraph.get(resType, (None, None, False))

            for keys, values in DictTools.iterDict(graphResCalls, ["sectionName", "partId", "regId"]):
                val = values["regId"]
                newVal = resEdit.getFixResourceName(val, modType, modName = modName)

                if (newVal is not None):
                    if (editGraph and toGraph is not None):
                        section = toGraph.getSection(keys["sectionName"])
                        part = section.partsById[keys["partId"]]

                        orderInd = keys["regId"]
                        part.setValByInd(orderInd, newVal)

                    val, newVal = resEdit.collectResourceName(val, newVal)
                else:
                    val, newVal = resEdit.collectResourceName(val, val)

                collectedSections[val] = newVal

            if (renameFunc is not None):
                toGraph.rename(renameFunc)

            if (toGraph is not None and partIdRefreshRequired):
                toGraph.refreshPartIds()

    def _buildResource(self, resEdit: BaseResEdit, graphGroups: List[IniGraphGroup], modType: "ModType", collectedSections: Dict[str, str], modName: str = "", ini: Optional["IniFile"] = None, copySections: bool = False):
        if (ini is not None):
            resEdit.buildResources(collectedSections, modType, ini, graphGroups, modName = modName, copySections = copySections)

    def _edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "", ini: Optional["IniFile"] = None):
        self.clear()
        hasIni = ini is not None
        resEditsLen = len(self.resEdits)
        copySections = resEditsLen > 1

        for srcModObj in self.srcRegs:
            srcReg = self.srcRegs[srcModObj]
            graphGroups = self._collectFromGraphGroup(graphGroups, srcModObj, srcReg)

        remappedGraphs = defaultdict(lambda: {}) if (self.remaps is not None) else None
        graphGroups = self._remapGraphs(graphGroups, remappedGraphs)

        for resSubType in self.resEdits:
            resEdit = self.resEdits[resSubType]
            collectedSections = {}

            self._collectResourceNames(resSubType, resEdit, collectedSections, graphGroups, modType, remappedGraphs = remappedGraphs, editGraph = hasIni, modName = modName)
            self._buildResource(resEdit, graphGroups, modType, collectedSections, modName = modName, ini = ini, copySections = copySections)

        return graphGroups

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self._edit(graphGroups, modType, modName = modName, ini = None)

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        graphGroups = self._edit(graphGroups, modType, modName = modName, ini = ini)
        self.clear()
        return graphGroups
##### EndScript