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
import copy
import itertools
from typing import List, Tuple, TYPE_CHECKING, Dict, Union, Set, Callable, Optional, DefaultDict
##### EndExtImports

##### CppLocalImports
from .....core import CppListTools
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
from ....iftemplate.IfTemplate import IfTemplate
from ....iftemplate.IfTemplatePart import IfTemplatePart
from ....iftemplate.IfContentPart import IfContentPart
from ....iftemplate.IfPredPart import IfPredPart
from ....iniresources.IniResource import IniGroupedResource
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
    def __init__(self, resGroupTypes: Set[str], 
                 srcRegs: Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], str]],
                 resEdits: Dict[Tuple[int, str, str], Dict[str, BaseResEdit]],
                 groupedResBuilders: Dict[str, IniGroupedResBuilder],
                 partPredicates: Optional[Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], Callable[[SectionIterData], bool]]]] = None,
                 resPredicates: Optional[Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], Callable[[str, str, SectionIterData], bool]]]] = None,
                 remaps: Optional[Dict[Tuple[int, str, str], Dict[Tuple[int, str, str], Dict[str, Union[Tuple[int, str, str], Tuple[int, str, str, Callable[[str], str]]]]]]] = None,
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

        self.collectedResTypes: Set[str] = set()
        self.collectedSections: Dict[Tuple[int, str, str], Dict[str, str]] = {}
        self.collectedSectQueries: Dict[Tuple[int, str, str], Dict[str, Union[bool, SympBooleanType]]] = {}
        self.resources: List[Tuple[Union[bool, SympBooleanType], IniGroupedResource]] = []

        self.id = id if (id is not None) else self._generate_id()

    def _generate_id(self) -> int:
        global ResGroupCollectAutoId

        ResGroupCollectAutoId += 1
        return ResGroupCollectAutoId

    def clear(self):
        self.collectedResTypes.clear()
        self.collectedSections.clear()
        self.collectedSectQueries.clear()
        self.resources.clear()
        self.resCalls.clear()

        for modObj in self.resEdits:
            self.resEdits[modObj].clear()

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

    def _remapGraph(self, fromGraph: IniSectionGraph, fromModObj: Tuple[int, str, str], toModObj: Tuple[int, str, str], remappedGraphs: DefaultDict[Tuple[int, str, str], DefaultDict[Tuple[str, int, int], Dict[str, Tuple[IniSectionGraph, Callable[[str], str]]]]], 
                    resTypes: DefaultDict[Tuple[int, str, str], List[str]], fromModObjOccurences: DefaultDict[Tuple[int, str, str], int], 
                    renameFunc: Optional[Callable[[str], str]] = None):
        result = fromGraph.deepcopy() if (fromModObj != toModObj) else fromGraph

        resTypeInd = fromModObjOccurences[fromModObj]
        resGroupType, resModObj = resTypes[fromModObj][resTypeInd]
        remappedGraphs[fromModObj][resModObj][resGroupType] = (result, renameFunc)

        fromModObjOccurences[fromModObj] += 1

        return result

    def _remapGraphs(self, graphGroups: List[IniGraphGroup], remappedGraphs: Optional[DefaultDict[Tuple[int, str, str], DefaultDict[Tuple[int, str, str], Dict[str, Tuple[IniSectionGraph, Callable[[str], str]]]]]]) -> List[IniGraphGroup]:
        if (self.remaps is None):
            return graphGroups
        
        remap = defaultdict(lambda: [])
        resTypes = defaultdict(lambda: [])
        srcModObjOccurences = defaultdict(lambda: 0)

        for keys, values in DictTools.iterDict(self.remaps, ["srcModObj", "resModobj", "resGroupType"]):
            srcModObj = keys["srcModObj"]
            resModObj = keys["resModObj"]
            resGroupType = keys["resGroupType"]
            toModObj = values["resGroupType"]

            remap[srcModObj].append(toModObj)
            resTypes[srcModObj].append((resGroupType, resModObj))

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

        return [False, commonResTypes]

    def _collectAllResourcesOld(self, modObj: Tuple[int, str, str], graphGroups: List[IniGraphGroup], modType: "ModType", collectedSections: Dict[str, str], collectedSectQueries: Dict[str, Union[bool, SympBooleanType]], rootFrequencies: DefaultDict[str, Dict[Tuple[str, int], int]], 
                             ifContentPartsToEdit: DefaultDict[IfTemplate, DefaultDict[IfContentPart, List[Tuple[str, int, str]]]], ini: Optional["IniFile"] = None, modName: str = "") -> Optional[IniSectionGraph]:
        resEdit = self.resEdits.get(modObj)
        if (resEdit is None):
            return None

        self._collectSectionQueries(modObj, graphGroups, modType, collectedSections, collectedSectQueries, ifContentPartsToEdit, modName = modName)

        if (ini is None):
            return None

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)
        graph = resEdit.getResGraph(collectedSections, modType, ini, graphGroups, modName = modName, rename = False)
        resType = resEdit.resType
        
        # default build a grouped resource for every resource file encountered
        for iterData in graph.iterByQuery():
            part = iterData.part
            sectionName = iterData.sectionName
            rootSectionName = iterData.rootSectionName

            fileVals = part.get(IniKeywords.Filename.value, default = [])
            if (not fileVals):
                continue

            newQuery = collectedSectQueries[rootSectionName]
            newQuery = sympy.And(newQuery, iterData.query)

            for ind, val in fileVals:
                if (val == IniKeywords.Null.value):
                    continue

                fileKey = BaseResEdit.getFileId(sectionName, part, ind, val)
                groupedResource = self.groupedResBuilders.build(isBuilt = False)
                groupedResource.resources[resType] = (val, rootSectionName, sectionName, modObj, fileKey)
                self.resources.append((newQuery, groupedResource))
                rootFrequencies[rootSectionName][fileKey] = 0

        self.collectedResTypes.add(resType)
        return graph

    def _getResCallNewNames(self, resModObj: Tuple[int, str, str], resGroupType: str, resRootQueries: Dict[str, Union[bool, SympBooleanType]], modType: "ModType", modName: str = "") -> Dict[str, str]:
        resCalls = self.resCalls.get(resModObj, {})
        if (not resCalls):
            return

        resEdit = DictTools.getVal(self.resEdits, [resModObj, resGroupType])
        if (resEdit is None):
            return

        result = {}
        for resCall, query in DictTools.iterDict(resCalls, ["srcModObj", "sectionName", "partId", "orderInd"], leafOnly = True):
            newResCall = resEdit.getFixResourceName(resCall, modType, modName = modName)
            currentResCall, newResCall = resEdit.collectResourceName(resCall, resCall if (newResCall is None) else newResCall)

            result[currentResCall] = newResCall
            resRootQueries[newResCall] = query

        return result



    def _collectAllResources(self, graphGroups: List[IniGraphGroup], resGroupType: str, resModObj: Tuple[int, str, str], resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]], commonResTypes: Set[Tuple[int, str, str]], 
                             resCalls: DefaultDict[Tuple[int, str, str], DefaultDict[str, DefaultDict[int, Dict[int, Tuple[str, Union[bool, SympBooleanType]]]]]],
                             remappedGraphs: DefaultDict[Tuple[int, str, str], Dict[Tuple[str, int, int], Dict[str, Tuple[IniSectionGraph, Callable[[str], str]]]]], graphNeedsCopy: Dict[Tuple[int, str, str], bool], 
                             groupedResBuilder: IniGroupedResBuilder, modType: "ModType", ini: Optional["IniFile"] = None, modName: str = ""):
        resEdit = DictTools.getVal(self.resEdits, [resModObj, resGroupType])
        if (resEdit is None):
            return

        resRootQueries = {}
        currentResCallNewNames = self._getResCallNewNames(resModObj, resGroupType, resRootQueries, modType, modName = modName)
        resCallNewNames[resModObj] = currentResCallNewNames

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)

        needsCopy = graphNeedsCopy.get(resModObj, False)
        graph = resEdit.getResGraph(currentResCallNewNames, modType, ini, graphGroups, modName = modName, rename = False, copySections = needsCopy)
        if (not needsCopy):
            graphNeedsCopy[resModObj] = True

        for iterData in graph.iterByQuery():
            part = iterData.part
            sectionName = iterData.sectionName
            rootSectionName = iterData.rootSectionName

            fileVals = part.get(IniKeywords.Filename.value, default = [], withInds = True)
            if (not fileVals):
                continue

            newQuery = resRootQueries[rootSectionName]
            newQuery = sympy.And(newQuery, iterData.query)

            for ind, val in fileVals:
                if (val == IniKeywords.Null.value):
                    continue

                fileKey = BaseResEdit.getFileId(sectionName, part, ind, val)
                groupedResource = groupedResBuilder.build(isBuilt = False)
                groupedResource.resources[resModObj] = (val, rootSectionName, sectionName, modObj, fileKey)
                self.resources.append((newQuery, groupedResource))
                rootFrequencies[rootSectionName][fileKey] = 0



        

    

    def _collectResGroupCommonResources(self, resGroupType: str, resGroups: List[IniGroupedResource], validResCalls: Set[Tuple[Tuple[int, str, str], Tuple[int, str, str], str, int, int]], 
                                        resCallNewNames: Dict[Tuple[int, str, str], Dict[str, str]], commonResTypes: Set[Tuple[int, str, str]], modType: "ModType", ini: Optional["IniFile"] = None, modName: str = ""):
        groupedResBuilder = self.groupedResBuilders.get(resGroupType)
        if (groupedResBuilder is None):
            return
        
        firstCollected = False

        for resType in commonResTypes:
            resCalls = self.resCalls.get(resType, {})
            if (not resCalls):
                continue

            resEdit = DictTools.getVal(self.resEdits, [resType, resGroupType])
            if (resEdit is None):
                continue

            if (not firstCollected):
                firstCollected = True

            








    def _collectSectionQueries(self, modObj: Tuple[int, str, str], graphGroups: List[IniGraphGroup], modType: "ModType", collectedSections: Dict[str, str], 
                               collectedSectQueries: Dict[str, Union[bool, SympBooleanType]], ifContentPartsToEdit: DefaultDict[IfTemplate, DefaultDict[IfContentPart, List[Tuple[str, int, str]]]], modName: str = ""):
        if (not self.srcRegs or modObj not in self.srcRegs):
            return

        graphGroupsLen = len(graphGroups)
        iniInd, comp, obj = modObj

        if (iniInd >= graphGroupsLen):
            return
        
        resEdit = self.resEdits.get(modObj)
        if (resEdit is None):
            return
        
        predicates = self.resPredicates.get(modObj, {})
        srcRegs = self.srcRegs[modObj]
        keysToTrack = self.keysToTrack.get(modObj, {})

        srcTrackKeys = None
        trackKeys = None
        if (not isinstance(self.trackKeys, bool)):
            trackKeys = self.trackKeys.get(modObj, {})
        else:
            srcTrackKeys = self.trackKeys

        # collect all the source resources
        for srcModObj in srcRegs:
            srcReg = srcRegs[srcModObj]
            srcIniInd, srcComp, srcObj = srcModObj

            if (srcIniInd >= graphGroupsLen):
                continue

            graphGroup = graphGroups[srcIniInd]
            graph = graphGroup.graphs.get((srcComp, srcObj))

            if (graph is None):
                continue

            predicate = predicates.get(srcModObj)
            srcKeysToTrack = keysToTrack.get(srcModObj)

            if (trackKeys is not None):
                srcTrackKeys = trackKeys.get(srcModObj)

            for iterData in graph.iterByQuery(colour = srcTrackKeys, colourKeys = srcKeysToTrack):
                part = iterData.part
                query = iterData.query

                regVals = iterData.part.get(srcReg, default = [])
                regValsLen = len(regVals)

                for i in range(regValsLen):
                    ind, val = regVals[i]
                    if (predicate is not None and not predicate(srcReg, val, iterData)):
                        continue

                    newVal = resEdit.getFixResourceName(val, modType, modName = modName)
                    if (newVal is not None):
                        val, newVal = resEdit.collectResourceName(val, newVal)
                    else:
                        val, newVal = resEdit.collectResourceName(val, val)

                    if (newVal in collectedSectQueries):
                        collectedSectQueries[newVal].append(query)
                    else:
                        collectedSectQueries[newVal] = [query]

                    if (val not in collectedSections):
                        collectedSections[val] = newVal

                    ifContentPartsToEdit[iterData.section][part].append([modObj, ind, newVal, []])

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)

        for srcResource in collectedSectQueries:
            query = collectedSectQueries[srcResource]
            collectedSectQueries[srcResource] = query[0] if (len(query) == 1) else sympy.Or(*query)

    def _collectSatisfyingResourcesOld(self, modObj: Tuple[int, str, str], graphGroups: List[IniGraphGroup], modType: "ModType", collectedSections: Dict[str, str], collectedSectQueries: Dict[str, Union[bool, SympBooleanType]], rootFrequencies: DefaultDict[str, Dict[Tuple[str, int], int]], 
                                    ifContentPartsToEdit: DefaultDict[IfTemplate, DefaultDict[IfContentPart, List[Tuple[str, int, str]]]], ini: Optional["IniFile"] = None, modName: str = "") -> Optional[IniSectionGraph]:
        resEdit = self.resEdits.get(modObj)
        if (resEdit is None):
            return None
        
        self._collectSectionQueries(modObj, graphGroups, modType, collectedSections, collectedSectQueries, ifContentPartsToEdit, modName = modName)

        if (ini is None):
            return None
        
        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)
        sympyLogicInference = GlobalPackageManager.get(PackageModules.Sympy_Logic_Inference.value)

        graph = resEdit.getResGraph(collectedSections, modType, ini, graphGroups, modName = modName, rename = True)
        resType = resEdit.resType

        resourcesLen = len(self.resources)
        newResources = []

        for iterData in graph.iterByQuery():
            part = iterData.part
            sectionName = iterData.sectionName
            rootSectionName = iterData.rootSectionName

            fileVals = part.get(IniKeywords.Filename.value, default = [])
            if (not fileVals):
                continue

            newQuery = collectedSectQueries[rootSectionName]
            newQuery = sympy.And(newQuery, iterData.query)

            for ind, val in fileVals:
                if (val == IniKeywords.Null.value):
                    continue

                added = False
                fileKey = BaseResEdit.getFileId(sectionName, part, ind, val)

                # check which resources are satisfiable with the current resource file
                for i in range(resourcesLen):
                    resourceQuery, resource = self.resources[i]

                    newResourceQuery = sympy.And(newQuery, resourceQuery)
                    newResourceQuery = newResourceQuery.replace(sympy.Ne, lambda a, b: sympy.Or(sympy.Lt(a, b), sympy.Gt(a, b)))

                    if (not sympyLogicInference.satisfiable(newResourceQuery, use_lra_theory=True)):
                        continue

                    newResource = copy.deepcopy(resource)
                    newResource.resources[resType] = (val, rootSectionName, sectionName, modObj, fileKey)
                    newResources.append((newResourceQuery, newResource))

                    if (not added):
                        added = True

                if (not added):
                    groupedResource = self.groupedResBuilders.build(isBuilt = False)
                    groupedResource.resources[resType] = (val, rootSectionName, sectionName, modObj, fileKey)

                    if (not groupedResource.isMissing(self.collectedResTypes)):
                        self.resources.append((newQuery, groupedResource))
                else:
                    rootFrequencies[rootSectionName][fileKey] = 0

        self.resources = newResources
        self.resources = list(filter(lambda resourceData: not resourceData[1].isMissing(self.collectedResTypes), self.resources))
        self.collectedResTypes.add(resType)
        return graph
    
    def _buildResIfCalls(self, resCallers: List[Tuple[str, Union[bool, SympBooleanType]]], depth: int) -> List[IfTemplatePart]:
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
            result.append(IfContentPart({IniKeywords.Run.value: [(0, resSectionName)]}, depth + 1))
            result.append(IfPredPart(IfPredPartType.EndIf.value, IfPredPartType.EndIf))

            count += 1

        return result
    
    def _splitIfContentPart(self, part: IfContentPart, ifPartsToAdd: Dict[int, List[IfContentPart]]):
        parts = part.splitByInds(inds = list(ifPartsToAdd.keys()), includeSplitKVP = False, includeEmptyParts = True)
        result = ListTools.interleave(parts, list(ifPartsToAdd.values()))
        
        resultLen = len(result)
        for i in range(resultLen):
            if (not isinstance(result[i], list)):
                result[i] = [result[i]]

        result = list(itertools.chain(*result))
        result = list(filter(lambda part: (not isinstance(part, IfContentPart) or part.src), result))

        return result

    

    def _edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", collectedSections: Optional[Dict[Tuple[int, str, str], Dict[str, str]]] = None, 
              collectedSectQueries: Optional[Dict[Tuple[int, str, str], Dict[str, Union[bool, SympBooleanType]]]] = None, ini: Optional["IniFile"] = None, modName: str = "") -> List[IniGraphGroup]:

        for keys, values in DictTools.iterDict(self.srcRegs, ["resModObj", "srcModObj"]):
            resModObj = keys["resModObj"]
            srcModObj = keys["srcModObj"]
            srcReg = values["srcModObj"]
            graphGroups = self._collectFromGraphGroup(graphGroups, resModObj, srcModObj, srcReg)

        remappedGraphs = defaultdict(lambda: defaultdict(lambda: {})) if (self.remaps is not None) else None
        graphGroups = self._remapGraphs(graphGroups, remappedGraphs)

        resGroups = []
        validResCalls = set()
        resCallNewNames = {}
        resources = []
        resGroupsCollected = False
        validResGroupTypes = set()

        for resGroupType in self.resGroupTypes:
            isValidResGroupType, commonResTypes = self._isValidResGroupType(resGroupType)
            if (not isValidResGroupType):
                continue

            validResGroupTypes.add(resGroupType)
            resCallNewNames.clear()

            if (not self.resGroupTypesSameTopology):
                resGroups.clear()
                validResCalls.clear()
                resources.clear()

            if (not resGroupsCollected):
                self._collectResGroupCommonResources(resGroupType, resGroups, validResCalls, resCallNewNames, commonResTypes)

            if (self.resGroupTypesSameTopology and not resGroupsCollected):
                resGroupsCollected = True











        

        modObjInd = 0
        graphs = {}
        rootFrequencies = {}
        
        ifContentPartsToEdit = defaultdict(lambda: defaultdict(lambda: []))
        collectedSectChildren = {}
        resQueries = defaultdict(lambda: defaultdict(lambda: []))

        # collect all the resources
        for modObj in self.srcRegs:
            objCollectedSections = {}
            objCollectedSectQueries = {}
            graph = None

            objRootFrequencies = defaultdict(lambda: {})
            rootFrequencies[modObj] = objRootFrequencies

            if (modObjInd == 0):
                graph = self._collectAllResources(modObj, graphGroups, modType, objCollectedSections, objCollectedSectQueries, objRootFrequencies, ifContentPartsToEdit, ini = ini, modName = modName)
            else:
                graph = self._collectSatisfyingResources(modObj, graphGroups, modType, objCollectedSections, objCollectedSectQueries, objRootFrequencies, ifContentPartsToEdit, ini = ini, modName = modName)

            if (collectedSections is not None and objCollectedSections):
                collectedSections[modObj] = objCollectedSections

            if (collectedSectQueries is not None and objCollectedSectQueries):
                collectedSectQueries[modObj] = objCollectedSectQueries
            
            if (graph is not None and objCollectedSections):
                graphs[modObj] = graph
                collectedSectChildren[modObj] = list(objCollectedSections.values())

            modObjInd += 1

        if (ini is None):
            return graphGroups
        
        for modObj in graphs:
            graph = graphs[modObj]
            collectedSectChildren[modObj] = graph.getChildren(collectedSectChildren[modObj], getNeighbourChildren = False)

        fileFrequencies = defaultdict(lambda: {})
        filesAddedCount = defaultdict(lambda: defaultdict(lambda: 0))
        filesAdded = defaultdict(lambda: {})

        # count the number of files and roots
        resourcesLen = len(self.resources)

        for i in range(resourcesLen):
            query, resourceGroup = self.resources[i]
            resources = resourceGroup.resources

            for resourceName in resources:
                file, root, sectionName, modObj, fileKey = resources[resourceName]
                objFileFrequencies = fileFrequencies[modObj]

                if (fileKey not in objFileFrequencies):
                    objFileFrequencies[fileKey] = 1
                else:
                    objFileFrequencies[fileKey] += 1

                rootFrequencies[modObj][root][fileKey] += 1

        for modObj in rootFrequencies:
            objRootFrequencies = rootFrequencies[modObj]

            for root in objRootFrequencies:
                objRootFrequencies[root] = max(objRootFrequencies[root].values())

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        modObjId = 0
        graphGroupsLen = len(graphGroups)
        
        # build the resources
        for modObj in graphs:
            resEdit = self.resEdits.get(modObj)
            if (resEdit is None):
                continue 
            
            iniInd, comp, obj = modObj
            if (iniInd >= graphGroupsLen):
                continue
            
            graphGroup = graphGroups[iniInd]
            newGraphSections = {}
            newGraphTargetSectionNames = OrderedSet()

            graph = graphs[modObj]
            graphCount = 0

            objRootFrequencies = rootFrequencies[modObj]
            objFileFrequencies = fileFrequencies[modObj]
            objFilesAddedCount = filesAddedCount[modObj]
            objFilesAdded = filesAdded[modObj]

            resourceFilter = lambda file, fileKey: fileKey in objFileFrequencies

            prevRootLen = len(objRootFrequencies)
            rootLen = prevRootLen

            roots = list(objRootFrequencies.keys())
            files = list(objFileFrequencies.keys())

            # generate the resources
            while (objRootFrequencies and objFileFrequencies):
                rootLen = len(objRootFrequencies)
                newGraph = graph.deepcopy()

                if (prevRootLen > rootLen):
                    newGraph.build(targetSectionNames = roots)

                currentResGroupId = f"{IniKeywords.ResourceGroup.value}{self.id}_{modObjId}_{graphCount}"

                currentObjFilesAdded = {}
                resEdit.buildResModels(newGraph, ini, modType, modName = modName, resources = currentObjFilesAdded, resourceFilter = resourceFilter, graphId = currentResGroupId)
                newGraph.rename(lambda oldName: f"{oldName}_{currentResGroupId}")

                for fileData in currentObjFilesAdded:
                    currentFiles = currentObjFilesAdded[fileData]
                    currentFilesLen = len(currentFiles)

                    for i in range(currentFilesLen):
                        currentFiles[i] = (currentFiles[i], currentResGroupId)

                objFilesAdded = DictTools.update(objFilesAdded, currentObjFilesAdded, lambda key, srcFiles, currentFiles: srcFiles + currentFiles)

                graphCount += 1
                prevRootLen = rootLen

                rootIndsToRemove = set()
                rootsLen = len(roots)

                for i in range(rootsLen):
                    root = roots[i]
                    objRootFrequencies[root] -= 1
                    if (objRootFrequencies[root] >= 1):
                        continue

                    del objRootFrequencies[root]
                    rootIndsToRemove.add(i)

                if (rootIndsToRemove):
                    roots = CppListTools.removeByInds(roots, rootIndsToRemove)

                fileIndsToRemove = set()
                filesLen = len(files)

                for i in range(filesLen):
                    fileData = files[i]
                    if (fileData not in objFilesAdded):
                        continue

                    currentFilesAddedCount = len(objFilesAdded[fileData])
                    currentFilesAdded = currentFilesAddedCount - objFilesAddedCount[fileData]

                    objFilesAddedCount[fileData] = currentFilesAddedCount
                    objFileFrequencies[fileData] -= currentFilesAdded

                    if (objFileFrequencies[fileData] >= 1):
                        continue

                    del objFileFrequencies[fileData]
                    fileIndsToRemove.add(i)

                if (fileIndsToRemove):
                    files = CppListTools.removeByInds(files, fileIndsToRemove)

                DictTools.update(newGraphSections, newGraph.sections)
                newGraphTargetSectionNames.update(OrderedSet(newGraph.targetSectionNames))

            if (newGraphSections and newGraphTargetSectionNames):
                newGraph = IniSectionGraph(newGraphSections, newGraphTargetSectionNames)
                graphGroup.addGraph((comp, obj), newGraph)

            modObjId += 1

        # add the resources to the resource group
        resourcesLen = len(self.resources)
        removedResourceInds = set()

        for i in range(resourcesLen):
            query, resourceGroup = self.resources[i]
            resources = resourceGroup.resources

            isBuilt = True
            currentResQueries = []

            for resourceName in resources:
                file, root, sectionName, modObj, fileKey = resources[resourceName]
                
                if (modObj not in filesAdded):
                    isBuilt = False
                    break

                objFilesAdded = filesAdded[modObj]
                if (fileKey not in objFilesAdded):
                    isBuilt = False
                    break

                currentFilesAdded = objFilesAdded[fileKey]
                if (not currentFilesAdded):
                    isBuilt = False
                    break

                currentResource, resGroupId = currentFilesAdded.pop()
                resources[resourceName] = currentResource

                currentResQueries.append((modObj, sectionName, resGroupId))

            if (not isBuilt):
                removedResourceInds.add(i)
                continue

            resourceGroup.isBuilt = True
            ini.resources.append(resourceGroup)

            for currentResQuery in currentResQueries:
                modObj, sectionName, resGroupId = currentResQuery
                resQueries[modObj][sectionName].append((f"{sectionName}_{resGroupId}", query))

        self.resources = CppListTools.removeByInds(self.resources, removedResourceInds)

        # connect the resources back to the caller sections
        resIfCalls = defaultdict(lambda: {})

        for section in ifContentPartsToEdit:
            sectionParts = ifContentPartsToEdit[section]
            sectionNewParts = {}

            for part in sectionParts:
                partResources = sectionParts[part]
                partResourcesLen = len(partResources)
                partDepth = part.depth
                currentNewParts = {}

                for i in range(partResourcesLen):
                    modObj, ind, resName, resCalls = partResources[i]
                    if (modObj not in collectedSectChildren or modObj not in resQueries):
                        continue

                    objResIfCalls = resIfCalls[modObj]
                    if (resName in objResIfCalls):
                        resCalls = copy.deepcopy(objResIfCalls[resName])
                    else:
                        sectResQueries = resQueries[modObj].get(resName, [])
                        if (sectResQueries):
                            resCalls.append(sectResQueries)

                        childrenSections = collectedSectChildren[modObj].get(resName, [])
                        for child in childrenSections:
                            sectResQueries = resQueries[modObj].get(child, [])
                            if (sectResQueries):
                                resCalls.append(sectResQueries)

                        resCalls = list(itertools.chain.from_iterable(resCalls))
                        resCalls = self._buildResIfCalls(resCalls, partDepth)
                        objResIfCalls[resName] = resCalls

                    currentNewParts[ind] = resCalls

                if (currentNewParts):
                    sectionNewParts[part] = self._splitIfContentPart(part, currentNewParts)

            partInds = section.find(pred = lambda ifTemplate, ind, part: part in sectionParts)
            sectionParts = section.parts

            for partInd in partInds:
                part = partInds[partInd]
                partsToAdd = sectionNewParts.pop(part, None)

                if (partsToAdd is not None):
                    sectionParts[partInd] = None
                    sectionNewParts[partInd] = partsToAdd

            section.parts = CppListTools.addLstsByInds(section.parts, sectionNewParts)
            section.parts = list(filter(lambda part: part is not None, section.parts))
            section.rebuild()

        return graphGroups

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        self.clear()
        return self._edit(graphGroups, modType, collectedSections = self.collectedSections, collectedSectQueries = self.collectedSectQueries, modName = modName)
    
    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        self.clear()
        result = self._edit(graphGroups, modType, ini = ini, modName = modName)
        self.clear()
        return result
##### EndScript