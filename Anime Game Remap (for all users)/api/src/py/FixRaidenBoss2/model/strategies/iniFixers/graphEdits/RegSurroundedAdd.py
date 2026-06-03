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

##### ExtImport
import itertools
from collections import deque, defaultdict
from typing import TYPE_CHECKING, Set, Callable, Union, Tuple, Deque, Dict, List, DefaultDict
##### EndExtImports

##### CppLocalImports
from .....core import CppListTools
##### EndCppLocalImports

##### LocalImports
from .....constants.GenericTypes import SympBooleanType
from .....constants.IfPredPartType import IfPredPartType
from .....constants.GlobalPackageManager import GlobalPackageManager
from .....constants.Packages import PackageModules
from ....iftemplate.IfPredPart import IfPredPart
from ....iftemplate.IfContentPart import IfContentPart
from ....iftemplate.IfTemplatePart import IfTemplatePart
from ....SectionIterQueryData import SectionIterQueryData
from .....tools.parsing.ParseContext import ParseContext
from ....IniSectionGraph import IniSectionGraph
from .BaseIniGraphEdit import BaseIniGraphEdit

if (TYPE_CHECKING):
    from ...ModType import ModType
##### EndLocalImports


##### Script
class RegSurroundedAdd(BaseIniGraphEdit):
    def __init__(self, beforeRegs: Union[Set[str], Callable[[str], bool]], afterReg: str, addition: Tuple[str, str]):
        self.beforeRegs = beforeRegs
        self.afterReg = afterReg
        self.addition = addition

    def _addBeforeReg(self, queryStack: Deque[Dict[IfContentPart, Union[bool, SympBooleanType]]], part: IfContentPart, iterData: SectionIterQueryData, foundAfterRegCount: int):
        if (foundAfterRegCount == len(queryStack)):
            queryStack.append({})

        if (part not in queryStack[-1]):
            queryStack[-1][part] = iterData.query

    def _addAfterReg(self, foundAfterRegCount: int, afterRegCountPerPart: DefaultDict[IfContentPart, int], queryStack: Deque[Dict[IfContentPart, Union[bool, SympBooleanType]]], part: IfContentPart, iterData: SectionIterQueryData, currentAfterReg: Tuple[int, str], 
                     kvpInsertions: DefaultDict[str, DefaultDict[IfContentPart, Dict[int, Tuple[str, str]]]], partInsertions: DefaultDict[str, DefaultDict[IfContentPart, Dict[int, List[IfPredPart]]]], sympy) -> int:
        currentAfterRegInd = currentAfterReg[0]
        query = sympy.And(*list(queryStack[-1].values()))

        isKVP, insertion = self._getInsertionVal(query, part.depth, sympy)
        if (isKVP):
            kvpInsertions[iterData.sectionName][part][currentAfterRegInd] = insertion
        else:
            partInsertions[iterData.sectionName][part][currentAfterRegInd] = insertion

        foundAfterRegCount += 1
        afterRegCountPerPart[part] += 1
        return foundAfterRegCount

    def _getInsertionVal(self, query: Union[bool, SympBooleanType], depth: int, sympy) -> Tuple[bool, Union[Tuple[str, str], List[IfTemplatePart]]]:
        if (query == True):
            return (True, self.addition)
        
        query = sympy.simplify(query)
        parseCtx = ParseContext(str(query))
        ifPredPartSpace = "\t" * depth
        queryStr = IfPredPart.getIfPredStr(parseCtx)

        return (False,
            [
                IfPredPart(f"{ifPredPartSpace}{IfPredPartType.If.value} {queryStr}", IfPredPartType.If, parseCtx, query = query),
                IfContentPart({self.addition[0]: [(0, self.addition[1])]}, depth + 1),
                IfPredPart(IfPredPartType.EndIf.value, IfPredPartType.EndIf)
            ])

    def edit(self, graph: IniSectionGraph, modType: "ModType", modName: str = "") -> IniSectionGraph:
        queryStack = deque([])
        afterRegCountPerPart = defaultdict(lambda: 0)
        foundAfterRegCount = 0
        partInsertions = defaultdict(lambda: defaultdict(lambda: {}))
        kvpInsertions = defaultdict(lambda: defaultdict(lambda: {}))

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)

        # find the areas that require an insertion
        for iterData in graph.iterByQuery(states = 2):
            part = iterData.part

            if (iterData.state == 1):
                currentAfterRegVals = part.get(self.afterReg, [])
                currentBeforeRegKeys = set()
                currentBeforeRegVals = []

                if (isinstance(self.beforeRegs, set)):
                    currentBeforeRegKeys = self.beforeRegs & set(part.src.keys())
                else:
                    currentBeforeRegKeys = set(filter(lambda key: self.beforeRegs(key), list(part.src.keys())))

                currentBeforeRegVals = list(map(lambda key: part[key], currentBeforeRegKeys))
                currentBeforeRegVals = list(itertools.chain(*currentBeforeRegVals))

                print(f"RAWTER:  {bool(currentBeforeRegVals)} AND {bool(currentAfterRegVals)} {foundAfterRegCount + 1} AND {len(queryStack)}")

                if (currentBeforeRegVals and currentAfterRegVals):
                    currentBeforeRegVals = list(map(lambda val: (False, val), currentBeforeRegVals))
                    newCurrentAfterRegVals = list(map(lambda val: (True, val), currentAfterRegVals))

                    combinedVals = currentBeforeRegVals + newCurrentAfterRegVals
                    combinedVals.sort(key = lambda data: data[1][0])
                    currentAfterRegInd = 0

                    for combinedVal in combinedVals:
                        isAfterReg, valData = combinedVal

                        print(f"UNNO: {isAfterReg} AND {valData} AND {foundAfterRegCount + 1} AND {len(queryStack)}")

                        if (not isAfterReg):
                            self._addBeforeReg(queryStack, part, iterData, foundAfterRegCount)

                        elif (isAfterReg and foundAfterRegCount + 1 == len(queryStack)):
                            foundAfterRegCount = self._addAfterReg(foundAfterRegCount, afterRegCountPerPart, queryStack, part, 
                                                                   iterData, currentAfterRegVals[currentAfterRegInd], kvpInsertions, partInsertions, sympy)
                            currentAfterRegInd += 1
                        else:
                            currentAfterRegInd += 1

                elif (currentBeforeRegVals):
                    self._addBeforeReg(queryStack, part, iterData, foundAfterRegCount)

                elif (currentAfterRegVals and foundAfterRegCount + 1 == len(queryStack)):
                    foundAfterRegCount = self._addAfterReg(foundAfterRegCount, afterRegCountPerPart, queryStack, part, 
                                                                   iterData, currentAfterRegVals[0], kvpInsertions, partInsertions, sympy)
                    
            else:
                if (part in afterRegCountPerPart):
                    afterRegCount = afterRegCountPerPart[part]

                    for i in range(afterRegCount):
                        queryStack.pop()

                    del afterRegCountPerPart[part]
                    foundAfterRegCount -= afterRegCount

                if (queryStack and part in queryStack[-1]):
                    del queryStack[-1][part]

        # add the insertions
        for sectionName, templateInsertions in kvpInsertions.items():
            for part, currentPartInsertions in templateInsertions.items():
                part.addKVPsByInds(currentPartInsertions)

        for sectionName in partInsertions:
            section = graph.getSection(sectionName, raiseException = False)
            if (section is None):
                continue
            
            templateInsertions = partInsertions[sectionName]
            templateParts = section.find(pred = lambda template, ind, part: part in templateInsertions)
            sectionParts = section.parts

            for ind in templateParts:
                part = templateParts[ind]
                insertions = templateInsertions[part]

                newParts = part.splitByInds(set(insertions.keys()), sortIndices = True)
                newParts = CppListTools.addLstsByInds(newParts, insertions)
                templateParts[ind] = newParts
                sectionParts[ind] = None

            section.parts = CppListTools.addLstsByInds(section.parts, templateParts)
            section.parts = list(filter(lambda part: part is not None, section.parts))
            section.rebuild()

        return graph
##### EndScript