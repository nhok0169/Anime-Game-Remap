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
from collections import defaultdict, deque
import copy
import itertools
from typing import List, Tuple, TYPE_CHECKING, Dict, Union, Set, Callable, Optional, DefaultDict, Deque
##### EndExtImports

##### CppLocalImports
from .....core import CppListTools, IfTemplatePart, IfContentPart, Ranges
##### EndCppLocalImports

##### LocalImports
from .....constants.GenericTypes import SympBooleanType
from .....constants.GlobalPackageManager import GlobalPackageManager
from .....constants.Packages import PackageModules
from .....constants.IniConsts import IniKeywords
from .....constants.IfPredPartType import IfPredPartType
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from ....IniGraphGroup import IniGraphGroup
from ....SectionIterData import SectionIterData
from ....iftemplate.IfPredPart import IfPredPart
from ....iniresources.IniResource import IniGroupedResource, IniResource
from ....IniSectionGraph import IniSectionGraph
from ....iniresources.IniGroupedResBuilder import IniGroupedResBuilder
from .....tools.DictTools import DictTools
from .....tools.ListTools import ListTools
from .....tools.parsing.ParseContext import ParseContext
from .ResEdit import BaseResEdit
from .GraphGroupRemap import GraphGroupRemap

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
ResGroupCollectAutoId = -1

class ResGroupCollect(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Creates the :class:`IniSectionGraph` for a particular group of resources

    Parameters
    ----------
    resGroupTypes: List[:class:`str`]
        The unique names for the type of resource groups

    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to search for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the source registers

    resEdits: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, :class:`BaseResEdit`]]
        Describes for how each resource in a resource group should be built :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The inner keys ar the names for the type of resource groups
        * The values are the edits for the type of resource

    groupedResBuilders: Dict[:class:`str`, :class:`IniGroupedResBuilder`]
        The builders used to construct a type of grouped resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names for the type of grouped resources and the values are the associated builders

    partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]]]
        The preciates for which particular order indices to process some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the predicates

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    resPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the predicates

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    remaps: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
        Whether to remap the remap the graphs searched from :attr:`srcRegs`. The values follow the same format as :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The inner keys are the types of the resource group

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]]
        Whether to track the `KVPs`_ in the .ini file when searching for particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values of the dictionary are the values of the flag
        
        :raw-html:`<br />`

        **Default**: ``False`` 

    keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]]
        Specific keys to track in the .ini file when searching particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the keys to track for each graph. If the value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart` for that graph

        :raw-html:`<br />`

        **Default**: ``None``

    resGroupTypesSameTopology: :class:`bool`
        A flag used to enable an optimization to reduce the number of `satisfiable (SAT) problems`_ needed to be computed when there are multiple types of resource groups.
        The flag assumes that each resource type for all type of resource groups have the same topology. For this case, we define a :class:`IniSectionGraph` to have
        the same topology when two graphs have exactly the same `sections`_ with the same structure. The names for the sections are also the same.

        The state of the graphs for this flag to be applicable are the graphs taken from :meth:`BaseResEdit.getResGraph` with the 'rename' flag set to ``False`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    id: Optional[:class:`int`]
        The unique id for this object. If this value is ``None``, then will autogenerate an id :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    resGroupTypes: List[:class:`str`]
        The unique names for the type of resource groups

    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to search for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the source registers

    resEdits: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, :class:`BaseResEdit`]]
        Describes for how each resource in a resource group should be built :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The inner keys ar the names for the type of resource groups
        * The values are the edits for the type of resource

    groupedResBuilders: Dict[:class:`str`, :class:`IniGroupedResBuilder`]
        The builders used to construct a type of grouped resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names for the type of grouped resources and the values are the associated builders

    partPredicates: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]]
        The preciates for which particular order indices to process some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the predicates

    resPredicates: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the predicates

    remaps: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
        Whether to remap the remap the graphs searched from :attr:`srcRegs`. The values follow the same format as :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The inner keys are the types of the resource group

    trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]]
        Whether to track the `KVPs`_ in the .ini file when searching for particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values of the dictionary are the values of the flag

    keysToTrack: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Specific keys to track in the .ini file when searching particular resources :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys in the dictionary are the mod object for a particular type of resource in a resource group, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        * The inner keys in the dictionary are the location of which :class:`IniSectionGraph` to apply the predicate for, which contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        * The values are the keys to track for each graph. If the value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart` for that graph

    resGroupTypesSameTopology: :class:`bool`
        A flag used to enable an optimization to reduce the number of `satisfiable (SAT) problems`_ needed to be computed when there are multiple types of resource groups.
        The flag assumes that each resource type for all type of resource groups have the same topology. For this case, we define a :class:`IniSectionGraph` to have
        the same topology when two graphs have exactly the same `sections`_ with the same structure. The names for the sections are also the same.

        The state of the graphs for this flag to be applicable are the graphs taken from :meth:`BaseResEdit.getResGraph` with the 'rename' flag set to ``False``

    id: :class:`int`
        The unique id for this object. If this value is ``None``, then will autogenerate an id

    resCalls: DefaultDict[Tuple[:class:`int`, :class:`str`, :class:`str`], DefaultDict[Tuple[:class:`int`, :class:`str`, :class:`str`], DefaultDict[:class:`str`, DefaultDict[:class:`int`, Dict[Tuple[:class:`str`, :class:`int`], Tuple[:class:`int`, :class:`str`]]]]]]
        The calls to the resource

        * The outer keys are the tuples to the type of resource that contain the index of the ini file, the name of the component and the name of the mod object
        * The second outer keys are tuples that contain the index of the ini file, the name of the component and the name of the mod object
        * The third outer keys are the names of the `sections`_
        * The second inner keys contains the id of the part within the `section`_
        * The inner keys are tuples that contain the name of the register that called the resource and the occurence index of the register in the part
        * The values are tuples that contain the index the resource call is found in the part and the name of the resource `section`_
    """

    def __init__(self, resGroupTypes: List[str], 
                 srcRegs: Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], str]],
                 resEdits: Dict[Tuple[int, str, str], Dict[str, BaseResEdit]],
                 groupedResBuilders: Dict[str, IniGroupedResBuilder],
                 partPredicates: Optional[Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], Callable[[SectionIterData], Ranges]]]] = None,
                 resPredicates: Optional[Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], Callable[[str, str, SectionIterData], bool]]]] = None,
                 remaps: Optional[Dict[Tuple[int, str, str], Dict[str, Union[Tuple[int, str, str], Tuple[int, str, str, Callable[[str], str]]]]]] = None,
                 trackKeys: Union[bool, Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], bool]]] = False,
                 keysToTrack: Optional[Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], Optional[Set[str]]]]] = None,
                 resGroupTypesSameTopology: bool = False,
                 id: Optional[int] = None):

        self.resGroupTypes = resGroupTypes
        self.srcRegs = srcRegs
        self.resEdits = resEdits
        self.partPredicates = partPredicates if (partPredicates is not None) else {}
        self.resPredicates = resPredicates if (resPredicates is not None) else {}
        self.resGroupTypesSameTopology = resGroupTypesSameTopology

        self.groupedResBuilders = groupedResBuilders
        self.trackKeys = trackKeys
        self.keysToTrack = keysToTrack if (keysToTrack is not None) else {}
        self.remaps = remaps

        self.resCalls: DefaultDict[Tuple[int, str, str], DefaultDict[Tuple[int, str, str], DefaultDict[str, DefaultDict[int, Dict[int, Tuple[str, Union[bool, SympBooleanType]]]]]]] = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: {}))))
        self.id = id if (id is not None) else self._generate_id()

    @property
    def resGroupTypes(self) -> List[str]:
        """
        The unique names for the type of resource groups

        :getter: Returns the unique types for the resource groups
        :setter: Sets the new types for the resource groups
        :type: List[:class:`str`]
        """

        return self._resGroupTypes

    @resGroupTypes.setter
    def resGroupTypes(self, newResGroupTypes: List[str]):
        self._resGroupTypes = ListTools.getDistinct(newResGroupTypes, keepOrder = True)

    def _generate_id(self) -> int:
        global ResGroupCollectAutoId

        ResGroupCollectAutoId += 1
        return ResGroupCollectAutoId

    def clear(self):
        self.resCalls.clear()

        for resEdit in DictTools.iterDict(self.resEdits, ["resModObj", "resGroupType"], leafOnly = True, ordered = False):
            resEdit.clear()

    def _collectFromGraphGroup(self, graphGroups: List[IniGraphGroup], resModObj: Tuple[int, str, str], srcModObj: Tuple[int, str, str], srcReg: str):
        graph = self.getGraph(graphGroups, srcModObj, errorOnNotFound = False)
        if (graph is None):
            return graphGroups

        partPredicate = DictTools.getVal(self.partPredicates, [resModObj, srcModObj])
        resPredicate = DictTools.getVal(self.resPredicates, [resModObj, srcModObj])
        keysToTrack = DictTools.getVal(self.keysToTrack, [resModObj, srcModObj])

        trackKeys = self.trackKeys
        if (not isinstance(trackKeys, bool)):
            trackKeys = DictTools.getVal(self.trackKeys, [resModObj, srcModObj])
        
        for iterData in graph.iterByQuery(colour = trackKeys, colourKeys = keysToTrack):
            sectionName = iterData.sectionName
            part = iterData.part
            query = iterData.query

            partRanges = None if (partPredicate is None) else partPredicate(iterData)
            regVals = part.get(srcReg, default = [], withInds = True, ranges = partRanges)

            for ind, val in regVals:
                if (resPredicate is not None and not resPredicate(srcReg, val, iterData)):
                    continue

                self.resCalls[resModObj][srcModObj][sectionName][part.id][ind] = (val, query)

        return graphGroups

    def _remapGraph(self, fromGraph: IniSectionGraph, fromModObj: Tuple[int, str, str], toModObj: Tuple[int, str, str], remappedGraphs: DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str], bool]]], 
                    resTypes: DefaultDict[Tuple[int, str, str], List[str]], fromModObjOccurences: DefaultDict[Tuple[int, str, str], int], 
                    renameFunc: Optional[Callable[[str], str]] = None):
        result = fromGraph.deepcopy(newPartIds = False)

        resTypeInd = fromModObjOccurences[fromModObj]
        resGroupType = resTypes[fromModObj][resTypeInd]
        remappedGraphs[fromModObj][resGroupType] = (result, renameFunc, True)

        fromModObjOccurences[fromModObj] += 1

        return result

    def _remapGraphs(self, graphGroups: List[IniGraphGroup], remappedGraphs: Optional[DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str], bool]]]]) -> List[IniGraphGroup]:
        if (self.remaps is None):
            return graphGroups
        
        remap = defaultdict(lambda: [])
        resTypes = defaultdict(lambda: [])
        srcModObjOccurences = defaultdict(lambda: 0)

        for keys, values in DictTools.iterDict(self.remaps, ["srcModObj", "resGroupType"]):
            srcModObj = keys["srcModObj"]
            resGroupType = keys["resGroupType"]
            toModObj = values["resGroupType"]

            remap[srcModObj].append(toModObj)
            resTypes[srcModObj].append(resGroupType)

        graphGroupRemap = GraphGroupRemap(remap)
        graphGroups = graphGroupRemap.remapGraphs(graphGroups, lambda fromGraph, fromObj, toObj, renameFunc: self._remapGraph(fromGraph, fromObj, toObj, remappedGraphs, resTypes, srcModObjOccurences, renameFunc = renameFunc))
        return graphGroups

    def _isValidResGroupType(self, resGroupType: str) -> List[Union[bool, Set[Tuple[int, str, str]]]]:
        if (resGroupType not in self.groupedResBuilders):
            return [False, set()]

        commonResTypes = DictTools.getCommonKeys([self.resEdits, self.srcRegs])
        for resType in commonResTypes:
            resEdits = self.resEdits.get(resType)
            if (resEdits is None):
                return [False, set()]

            if (resGroupType not in resEdits):
                return [False, set()]

        return [True, commonResTypes]

    def _getResCallNewNames(self, resModObj: Tuple[int, str, str], resGroupType: str, resRootQueries: Dict[str, Union[bool, SympBooleanType]], 
                            resRootLocations: Dict[str, Tuple[Tuple[int, str, str], Tuple[int, str, str], str, int, int]], modType: "ModType", modName: str = "") -> Dict[str, str]:
        resCalls = self.resCalls.get(resModObj, {})
        if (not resCalls):
            return {}

        resEdit = DictTools.getVal(self.resEdits, [resModObj, resGroupType])
        if (resEdit is None):
            return {}

        result = {}
        for keys, values in DictTools.iterDict(resCalls, ["srcModObj", "sectionName", "partId", "orderInd"]):
            resCall, query = values["orderInd"]
            newResCall = resEdit.getFixResourceName(resCall, modType, modName = modName)
            currentResCall, newResCall = resEdit.collectResourceName(resCall, resCall if (newResCall is None) else newResCall)

            result[currentResCall] = newResCall
            resRootQueries[currentResCall] = query
            resRootLocations[currentResCall] = (resModObj, keys["srcModObj"], keys["sectionName"], keys["partId"], keys["orderInd"])

        return result

    def _getResGraph(self, graphGroups: List[IniGraphGroup], resGroupType: str, resModObj: Tuple[int, str, str], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]],
                     resRootQueries: Dict[str, Union[bool, SympBooleanType]], resRootLocations: Dict[str, Tuple[Tuple[int, str, str], Tuple[int, str, str], str, int, int]], 
                     modType: "ModType", ini: Optional["IniFile"] = None, modName: str = "") -> IniSectionGraph:
        resEdit = DictTools.getVal(self.resEdits, [resModObj, resGroupType])
        if (resEdit is None):
            return None

        currentResCallNewNames = self._getResCallNewNames(resModObj, resGroupType, resRootQueries, resRootLocations, modType, modName = modName)
        resCallNewNames[resModObj] = currentResCallNewNames
        graph = resEdit.getResGraph(currentResCallNewNames, modType, ini, graphGroups, modName = modName, rename = False, copySections = True)
        return graph

    def _collectAllResources(self, graphGroups: List[IniGraphGroup], resGroupType: str, resModObj: Tuple[int, str, str], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]],
                             groupedResBuilder: IniGroupedResBuilder, resGroups: List[Tuple[IniGroupedResource, Union[bool, SympBooleanType]]],
                             collectedResTypes: Set[str], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = "") -> IniSectionGraph:
        resRootQueries = {}
        resRootLocations = {}

        graph = self._getResGraph(graphGroups, resGroupType, resModObj, resCallNewNames, resRootQueries, resRootLocations, modType, ini = ini, modName = modName)
        if (graph is None):
            return

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)
        for iterData in graph.iterByQuery():
            part = iterData.part
            partDepth = part.depth
            sectionName = iterData.sectionName
            rootSectionName = iterData.rootSectionName

            fileVals = part.get(IniKeywords.Filename.value, default = [], withInds = True)
            if (not fileVals):
                continue

            resRootLocation = resRootLocations[rootSectionName]
            newQuery = resRootQueries[rootSectionName]
            newQuery = sympy.And(newQuery, iterData.query)

            for ind, val in fileVals:
                if (val == IniKeywords.Null.value):
                    continue

                fileKey = BaseResEdit.getFileId(resModObj, sectionName, part, ind, val)
                groupedResource = groupedResBuilder.build(isBuilt = False)
                groupedResource.resources[resModObj] = (fileKey, rootSectionName, resRootLocation, partDepth)
                resGroups.append((groupedResource, newQuery))

        collectedResTypes.add(resModObj)
        return graph

    def _collectSatisfyingResources(self, graphGroups: List[IniGraphGroup], resGroupType: str, resModObj: Tuple[int, str, str], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]],
                                    groupedResBuilder: IniGroupedResBuilder, resGroups: List[Tuple[IniGroupedResource, Union[bool, SympBooleanType]]], 
                                    collectedResTypes: Set[str], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = "") -> IniSectionGraph:
        resRootQueries = {}
        resRootLocations = {}

        graph = self._getResGraph(graphGroups, resGroupType, resModObj, resCallNewNames, resRootQueries, resRootLocations, modType, ini = ini, modName = modName)
        if (graph is None):
            return

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)
        sympyLogicInference = GlobalPackageManager.get(PackageModules.Sympy_Logic_Inference.value)
        resGroupsLen = len(resGroups)

        for iterData in graph.iterByQuery():
            part = iterData.part
            partDepth = part.depth
            sectionName = iterData.sectionName
            rootSectionName = iterData.rootSectionName

            fileVals = part.get(IniKeywords.Filename.value, default = [], withInds = True)
            if (not fileVals):
                continue

            resRootLocation = resRootLocations[rootSectionName]
            newQuery = resRootQueries[rootSectionName]
            newQuery = sympy.And(newQuery, iterData.query)

            for ind, val in fileVals:
                if (val == IniKeywords.Null.value):
                    continue

                added = False
                fileKey = BaseResEdit.getFileId(resModObj, sectionName, part, ind, val)

                for i in range(resGroupsLen):
                    resGroup, resGroupQuery = resGroups[i]

                    newResGroupQuery = sympy.And(newQuery, resGroupQuery)
                    newResGroupQuery = newResGroupQuery.replace(sympy.Ne, lambda a, b: sympy.Or(sympy.Lt(a, b), sympy.Gt(a, b)))

                    if (not sympyLogicInference.satisfiable(newResGroupQuery, use_lra_theory=True)):
                        continue

                    newResGroup = copy.deepcopy(resGroup)
                    newResGroup.resources[resModObj] = (fileKey, rootSectionName, resRootLocation, partDepth)
                    resGroups.append((newResGroup, newResGroupQuery))

                    if (not added):
                        added = True

                if (not added):
                    resGroup = groupedResBuilder.build(isBuilt = False)
                    resGroup.resources[resModObj] = (fileKey, rootSectionName, resRootLocation)

                    if (not resGroup.isMissing(collectedResTypes)):
                        resGroups.append((resGroup, newQuery))

        collectedResTypes.add(resModObj)
        ListTools.filterInPlace(resGroups, lambda resGroupData: not resGroupData[0].isMissing(collectedResTypes))
        return graph

    def _collectResGroups(self, graphGroups: List[IniGraphGroup], resGroupType: str, resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]], 
                          commonResTypes: Set[Tuple[int, str, str]], resGroups: List[Tuple[IniGroupedResource, Union[bool, SympBooleanType]]], resGraphs: Dict[Tuple[int, str, str], IniSectionGraph],
                          collectedResTypes: Set[str], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = ""):
        groupedResBuilder = self.groupedResBuilders.get(resGroupType)
        if (groupedResBuilder is None):
            return
        
        firstCollected = False

        for resType in commonResTypes:
            if (not firstCollected):
                resGraphs[resType] = self._collectAllResources(graphGroups, resGroupType, resType, resCallNewNames, groupedResBuilder, resGroups, collectedResTypes, modType, ini = ini, modName = modName)
                firstCollected = True
            else:
                resGraphs[resType] = self._collectSatisfyingResources(graphGroups, resGroupType, resType, resCallNewNames, groupedResBuilder, resGroups, collectedResTypes, modType, ini = ini, modName = modName)

    def _collectResGraphNewNames(self, resGroupType: str, commonResTypes: Set[Tuple[int, str, str]], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]], modType: "ModType", modName: str = ""):
        for resType in commonResTypes:
            resRootQueries = {}
            resRootLocations = {}

            currentResCallNewNames = self._getResCallNewNames(resType, resGroupType, resRootQueries, resRootLocations, modType, modName = modName)
            resCallNewNames[resType] = currentResCallNewNames

    def _fileKeyExists(self, fileKey: str, fileFreqs: DefaultDict[str, int]):
        result = fileKey in fileFreqs
        if (not result):
            return result

        freq = fileFreqs[fileKey]
        if (freq <= 1):
            del fileFreqs[fileKey]
        else:
            fileFreqs[fileKey] -= 1

        return result

    def _getResGraphId(self, resGroupTypeId: int, resTypeId: int, graphCopyId: int) -> str:
        return f"{self.id}_{resGroupTypeId}_{resTypeId}_{graphCopyId}"

    def _replicateResGraphs(self, resGroupType: str, resGroupTypeId: int, sectionFreqs: DefaultDict[Tuple[int, str, str], DefaultDict[str, int]], fileFreqs: DefaultDict[Tuple[int, str, str], DefaultDict[str, int]], 
                            commonResTypes: Set[Tuple[int, str, str]], resGraphs: Dict[Tuple[int, str, str], IniSectionGraph], collectedResources: Dict[str, Deque[Tuple[IniResource, str]]],
                            combinedResGraphs: DefaultDict[Tuple[int, str, str], List[Tuple[IniSectionGraph, str]]], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = ""):
        currentSectionFreqs = copy.deepcopy(sectionFreqs)
        currentFileFreqs = copy.deepcopy(fileFreqs)
        resTypeId = 0

        for resType in commonResTypes:
            resEdit = DictTools.getVal(self.resEdits, [resType, resGroupType])
            if (resEdit is None):
                continue

            graph = resGraphs[resType]
            currentCombinedGraphs = combinedResGraphs[resType]
            resFileFreqs = currentFileFreqs[resType]
            resSectionFreqs = currentSectionFreqs[resType]
            graphCopyId = 0

            resourceFilter = lambda file, fileKey: self._fileKeyExists(fileKey, resFileFreqs)

            while (resSectionFreqs):
                graphId = self._getResGraphId(resGroupTypeId, resTypeId, graphCopyId)
                newGraph = graph.deepcopy(newPartIds = False)

                if (len(resSectionFreqs) < len(newGraph.targetSectionNames)):
                    newGraph.build(targetSectionNames = list(resSectionFreqs.keys()))

                currentResources = {}
                resEdit.buildResModels(newGraph, ini, modType, modName = modName, resources = currentResources, resourceFilter = resourceFilter, graphId = graphId, resModObj = resType)

                for fileKey in currentResources:
                    currentResources[fileKey] = deque(map(lambda resource: (resource, graphId), currentResources[fileKey]))

                DictTools.combine(collectedResources, currentResources, combineDuplicate = lambda fileKey, srcFileRes, newFileRes: srcFileRes + newFileRes, makeNewCopy = False)
                currentCombinedGraphs.append((newGraph, graphId))
                graphCopyId += 1

                for sectionName in resSectionFreqs:
                    resSectionFreqs[sectionName] -= 1

                resSectionFreqs = DictTools.filter(resSectionFreqs, lambda sectionName, freq: freq > 0)

            resTypeId += 1

    def _countAndReplicateResGraphs(self, resGroupType: str, resGroupTypeId: int, resGroups: List[Tuple[IniGroupedResource, Union[bool, SympBooleanType]]], 
                                    sectionFreqs: DefaultDict[Tuple[int, str, str], DefaultDict[str, int]], fileFreqs: DefaultDict[Tuple[int, str, str], DefaultDict[str, int]], 
                                    commonResTypes: Set[Tuple[int, str, str]], resGraphs: Dict[Tuple[int, str, str], IniSectionGraph], collectedResources: Dict[str, Deque[Tuple[IniResource, str]]],
                                    combinedResGraphs: DefaultDict[Tuple[int, str, str], List[Tuple[IniSectionGraph, str]]], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = ""):
        for resGroup, resGroupQuery in resGroups:
            resources = resGroup.resources

            for resType in resources:
                fileKey, rootResSectionName, rootResLocation, partDepth = resources[resType]
                sectionFreqs[resType][rootResSectionName] += 1
                fileFreqs[resType][fileKey] += 1

        self._replicateResGraphs(resGroupType, resGroupTypeId, sectionFreqs, fileFreqs, commonResTypes, resGraphs, collectedResources, combinedResGraphs, modType, ini = ini, modName = modName)

    @classmethod
    def _getNewSectionName(cls, sectionName: str, resNewCalls: Dict[str, str], graphId: str) -> str:
        result = resNewCalls.get(sectionName, sectionName)
        return f"{result}{graphId}"

    def _connectResGroups(self, resGroups: List[Tuple[IniGroupedResource, Union[bool, SympBooleanType]]], collectedResources: Dict[str, Deque[Tuple[IniResource, str]]], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]], 
                          ini: Optional["IniFile"] = None) -> DefaultDict[Tuple[int, str, str], DefaultDict[Tuple[int, str, str], DefaultDict[str, DefaultDict[int, Dict[int, List[Union[int, List[Tuple[str, Union[bool, SympBooleanType]]]]]]]]]]:
        resCallConnData = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: {}))))
        resGroupsLen = len(resGroups)

        for i in range(resGroupsLen):
            resourceGroup, query = resGroups[i]
            resources = resourceGroup.resources
            isBuilt = True
            resGroupConnData = []

            for resModObj in resources:
                fileKey, rootSectionName, rootLocation, partDepth = resources[resModObj]
                fileKeyResources = collectedResources.get(fileKey)

                if (not fileKeyResources):
                    isBuilt = False
                    break

                currentResource, graphId = fileKeyResources.popleft()
                resources[resModObj] = currentResource
                resGroupConnData.append((rootLocation, rootSectionName, graphId, query, partDepth, resModObj))

            if (not isBuilt):
                continue

            for rootLocation, rootSectionName, graphId, query, partDepth, resModObj in resGroupConnData:
                newSectionName = self._getNewSectionName(rootSectionName, resCallNewNames.get(resModObj, {}), graphId)
                currentConnData = DictTools.getVal(resCallConnData, rootLocation)
                if (currentConnData is None):
                    DictTools.setVal(resCallConnData, rootLocation, [partDepth, [(newSectionName, query)]])
                else:
                    currentConnData[1].append((newSectionName, query))

            resourceGroup.isBuilt = True
            if (ini is not None):
                ini.resources.append(resourceGroup)

        return resCallConnData

    def _buildResIfCalls(self, resCallers: List[Tuple[str, Union[bool, SympBooleanType]]], srcReg: str, depth: int) -> List[IfTemplatePart]:
        queries = {}
        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)

        for resCaller in resCallers:
            sectionName, query = resCaller
            if (sectionName not in queries):
                queries[sectionName] = query
            else:
                queries[sectionName] = sympy.Or(queries[sectionName])

        result = []
        count = 0

        for resSectionName in queries:
            query = sympy.simplify(queries[resSectionName])
            parseCtx = ParseContext(str(query))

            queryStr = IfPredPart.getIfPredStr(parseCtx)
            if (queryStr is None):
                continue

            ifPredPartSpace = "\t" * depth

            result.append(IfPredPart(f"{ifPredPartSpace}{IfPredPartType.If.value} {queryStr}", IfPredPartType.If, parseCtx, query = query))
            result.append(IfContentPart({srcReg: [(0, resSectionName)]}, depth + 1))
            result.append(IfPredPart(IfPredPartType.EndIf.value, IfPredPartType.EndIf))

            count += 1

        return result

    @classmethod
    def _splitIfContentPart(cls, part: IfContentPart, ifPartsToAdd: Dict[int, List[IfTemplatePart]]) -> List[IfTemplatePart]:
        parts = part.splitByInds(inds = list(ifPartsToAdd.keys()), includeSplitKVP = False, includeEmptyParts = True)
        result = ListTools.interleave(parts, list(ifPartsToAdd.values()))
        
        resultLen = len(result)
        for i in range(resultLen):
            if (not isinstance(result[i], list)):
                result[i] = [result[i]]

        result = list(itertools.chain(*result))
        result = list(filter(lambda part: (not isinstance(part, IfContentPart) or not part.empty()), result))

        return result

    def _connectResCalls(self, resGroupType: str, graphGroups: List[IniGraphGroup], resCallConnData: DefaultDict[Tuple[int, str, str], DefaultDict[Tuple[int, str, str], DefaultDict[str, DefaultDict[int, DefaultDict[int, List[Tuple[str, Union[bool, SympBooleanType]]]]]]]],
                         remappedGraphs: Optional[DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str], bool]]]] = None):
        resCallConnected = defaultdict(lambda: False)
        pathMinLen = 5
        commonResCallPaths = DictTools.getCommonPaths([self.resCalls, resCallConnData])
        newCallParts = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: {}))))

        for resCallPath in commonResCallPaths:
            if (len(resCallPath) < pathMinLen):
                continue

            resModObj = resCallPath[0]
            srcModObj = resCallPath[1]

            srcReg = self.srcRegs[resModObj][srcModObj]
            partDepth, resCallers = DictTools.getVal(resCallConnData, resCallPath)

            resCallers = self._buildResIfCalls(resCallers, srcReg, depth = partDepth)
            DictTools.setVal(newCallParts, resCallPath, resCallers)
            resCallConnected[tuple(resCallPath)] = True

        unConnectedResCalls = DictTools.filter(resCallConnected, lambda path, isConnected: not isConnected)
        for resCallPath in unConnectedResCalls:
            DictTools.setVal(newCallParts, resCallPath, [])

        hasRemappedGraphs = remappedGraphs is not None

        for keys, values in DictTools.iterDict(newCallParts, ["resModObj", "srcModObj"]):
            resModObj = keys["resModObj"]
            srcModObj = keys["srcModObj"]
            graphResCallParts = values["srcModObj"]

            toGraph = remappedGraphs.get(srcModObj) if (hasRemappedGraphs) else self.getGraph(graphGroups, srcModObj, errorOnNotFound = False)
            if (hasRemappedGraphs and toGraph is not None):
                toGraph, _, _ = toGraph.get(resGroupType, [None, None, None])

            if (toGraph is None):
                continue

            for sectionName in graphResCallParts:
                section = toGraph.getSection(sectionName, raiseException = False)
                if (section is None):
                    continue

                sectionResCallParts = graphResCallParts[sectionName]
                partInds = section.find(pred = lambda ifTemplate, ind, part: part.id in sectionResCallParts, postProcessor = lambda section, partInd, part: part.id)

                for partId in sectionResCallParts:
                    part = section.partsById.get(partId)
                    if (part is None):
                        continue

                    sectionResCallParts[partId] = self._splitIfContentPart(part, sectionResCallParts[partId])

                sectionNewParts = {}

                for partInd in partInds:
                    partId = partInds[partInd]
                    splitParts = sectionResCallParts[partId]
                    if (not isinstance(splitParts, list)):
                        continue

                    sectionNewParts[partInd] = splitParts
                    section.parts[partInd] = None

                section.parts = CppListTools.addLstsByInds(section.parts, sectionNewParts)
                section.parts = list(filter(lambda part: part is not None, section.parts))
                section.rebuild()

    def _cleanResCallGraphs(self, remappedGraphs: Optional[DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str], bool]]]] = None):
        if (remappedGraphs is None):
            return
        
        visitedToGraphs = set()
        for toGraph, renameFunc, partIdRefreshRequired in DictTools.iterDict(remappedGraphs, ["srcModObj", "resGroupType"], ordered = False, leafOnly = True):
            if (toGraph in visitedToGraphs):
                continue

            visitedToGraphs.add(toGraph)

            if (renameFunc is not None):
                toGraph.rename(renameFunc)

            if (toGraph is not None and partIdRefreshRequired):
                toGraph.refreshPartIds()

    def _connectResGraphs(self, resGroupType: str, combinedResGraphs: DefaultDict[Tuple[int, str, str], List[Tuple[IniSectionGraph, str]]], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]],
                          graphGroups: List[IniGraphGroup]):
        for resType in combinedResGraphs:
            resEdit = DictTools.getVal(self.resEdits, [resType, resGroupType])
            if (resEdit is None):
                continue
            
            graphs = combinedResGraphs[resType]
            if (not graphs):
                continue

            for graph, graphId in graphs:
                graph.rename(lambda name: self._getNewSectionName(name, resCallNewNames.get(resType, {}), graphId))
                graph.refreshPartIds()

            graphsLen = len(graphs)
            graph, _ = graphs[0]
            graph.combine([graphs[i][0] for i in range(1, graphsLen)])
            self.addGraph(graphGroups, resEdit.resModObj, graph)

    def _edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = "") -> List[IniGraphGroup]:
        for keys, values in DictTools.iterDict(self.srcRegs, ["resModObj", "srcModObj"]):
            resModObj = keys["resModObj"]
            srcModObj = keys["srcModObj"]
            srcReg = values["srcModObj"]
            graphGroups = self._collectFromGraphGroup(graphGroups, resModObj, srcModObj, srcReg)

        remappedGraphs = defaultdict(lambda: {}) if (self.remaps is not None) else None
        graphGroups = self._remapGraphs(graphGroups, remappedGraphs)

        resGroups = []
        resGroupsCopy = []
        resGraphs = {}
        resCallNewNames = defaultdict(lambda: {})
        resGroupsCollected = False
        collectedResTypes = set()
        sectionFreqs = defaultdict(lambda: defaultdict(lambda: 0))
        fileFreqs = defaultdict(lambda: defaultdict(lambda: 0))
        collectedResources = {}
        combinedResGraphs = defaultdict(lambda: [])
        resGroupTypeId = 0

        for resGroupType in self._resGroupTypes:
            isValidResGroupType, commonResTypes = self._isValidResGroupType(resGroupType)
            if (not isValidResGroupType):
                continue

            resCallNewNames.clear()
            collectedResources.clear()
            combinedResGraphs.clear()

            if (not self.resGroupTypesSameTopology):
                resGraphs.clear()
                resGroups.clear()
                collectedResTypes.clear()
                sectionFreqs.clear()
                fileFreqs.clear()

            if (not resGroupsCollected):
                self._collectResGroups(graphGroups, resGroupType, resCallNewNames, commonResTypes, resGroups, resGraphs, collectedResTypes, modType, ini = ini, modName = modName)
                self._countAndReplicateResGraphs(resGroupType, resGroupTypeId, resGroups, sectionFreqs, fileFreqs, commonResTypes, resGraphs, collectedResources, combinedResGraphs, modType, ini = ini, modName = modName)

                if (self.resGroupTypesSameTopology):
                    resGroupsCopy = copy.deepcopy(resGroups)
            else:
                resGroups = copy.deepcopy(resGroupsCopy)
                self._collectResGraphNewNames(resGroupType, commonResTypes, resCallNewNames, modType, modName = modName)
                self._replicateResGraphs(resGroupType, resGroupTypeId, sectionFreqs, fileFreqs, commonResTypes, resGraphs, collectedResources, combinedResGraphs, modType, ini = ini, modName = modName)

            resCallConnData = self._connectResGroups(resGroups, collectedResources, resCallNewNames, ini = ini)
            self._connectResCalls(resGroupType, graphGroups, resCallConnData, remappedGraphs)
            self._connectResGraphs(resGroupType, combinedResGraphs, resCallNewNames, graphGroups)

            if (self.resGroupTypesSameTopology and not resGroupsCollected):
                resGroupsCollected = True

            resGroupTypeId += 1

        self._cleanResCallGraphs(remappedGraphs = remappedGraphs)
        return graphGroups

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        self.clear()
        return self._edit(graphGroups, modType, modName = modName)
    
    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        self.clear()
        result = self._edit(graphGroups, modType, ini = ini, modName = modName)
        self.clear()
        return result
##### EndScript