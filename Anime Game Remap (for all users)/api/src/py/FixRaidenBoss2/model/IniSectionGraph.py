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
from collections import deque, defaultdict
import copy
from typing import Dict, Union, List, Optional, Set, Callable, Any, Generator, Type
##### EndExtImports

##### CppLocalImports
from ..core import IfContentPartColouring
from ..core import IfContentPart
from ..core import Hashes
from ..core import Indices
##### EndCppLocalImports

##### LocalImports
from ..constants.GlobalPackageManager import GlobalPackageManager
from ..constants.Packages import PackageModules
from ..constants.GenericTypes import SympBooleanType, OrderedSetType
from ..constants.IniConsts import IniKeywords
from ..constants.IfPredPartType import IfPredPartType
from .iftemplate.IfTemplate import IfTemplate
from .iftemplate.IfTemplateNode import IfTemplateNode
from .SectionIterData import SectionIterQueryData, SectionIterData
from .CallGraph import CallGraph
from ..tools.ListTools import ListTools
from ..tools.DictTools import DictTools
##### EndLocalImports


##### Script
class IniSectionGraph():
    """
    Class for constructing a directed subgraph for how the `sections`_ in the .ini file are ran

    :raw-html:`<br />`

    .. note::
        * The nodes are the `sections`_ of the .ini file
        * The directed edges are the command calls from the `sections`_ , where the source of the edge is the caller and the sink of the edge is the callees

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for sectionName, section in x

            Iterates through all the `sections`_ of the graph using `DFS`_

        .. describe:: x = y + z

            Creates a new graph ``x`` by combining graphs ``y`` and ``z``

        .. describe:: x += y

            Combines graph ``y`` into ``x`` using :meth:`combine`

    Parameters
    ----------
    sections: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ of the constructed subgraph :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the `sections`_ and the values are the `sections`_

    targetSectionNames: Union[Set[:class:`str`], List[:class:`str`]]
        Names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini files :raw-html:`<br />` 

    build: :class:`bool`
        Whether to the build the graph :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    copySections: :class:`bool`
        Whether to make a deep copy of the `sections`_ referenced by the constructed graph :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``
    
    Attributes
    ----------
    sections: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ of the constructed subgraph :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the `sections`_ and the values are the `sections`_

    neighbours: Dict[:class:`str`, List[:class:`str`]]
        The out-neighbours of the subgraph :raw-html:`<br />` 
        
        .. note::
            This is the `adjacency list`_ of the subgraph

    roots: List[:class:`str`]
        The root nodes of the subgraph
    """

    def __init__(self, sections: Dict[str, IfTemplate], targetSectionNames: Union[Set[str], List[str]], build: bool = True, copySections: bool = False):
        self.sections: Dict[str, IfTemplate] = {}
        self.neighbours: Dict[str, List[str]] = {}
        self.roots: List[str] = []
        
        self.sections = sections
        self._setTargetSectionNames(targetSectionNames)

        if (build):
            self.build(copySections = copySections)

    def __iter__(self) -> Generator:
        stack = deque()
        visitedSections = set()

        for root in self.roots:
            stack.appendleft(root)

        while (stack):
            sectionName = stack.pop()
            if (sectionName in visitedSections):
                continue

            section = self.getSection(sectionName)

            yield (sectionName, section)
            visitedSections.add(sectionName)

            neighbours = self.neighbours.get(sectionName, [])
            neighboursLen = len(neighbours)

            for i in range(neighboursLen - 1, -1, -1):
                stack.append(neighbours[i])

    def __iadd__(self, newGraph: "IniSectionGraph") -> "IniSectionGraph":
        if (not isinstance(newGraph, self.__class__)):
            return TypeError(f"The parameter to combine with this graph must be type: {self.__class__}")
        
        self.combine([newGraph])
        return self
    
    def __add__(self, newGraph: "IniSectionGraph") -> "IniSectionGraph":
        if (not isinstance(newGraph, self.__class__)):
            return TypeError(f"The parameter to combine with this graph must be type: {self.__class__}")
        
        targetSectionNames = self._targetSectionNames + newGraph.targetSectionNames
        sections = DictTools.combine(self.sections, newGraph.sections, combineDuplicate = lambda key, srcVal, newVal: srcVal)
        return self.__class__(sections, targetSectionNames = targetSectionNames)

    def combine(self, newGraphs: List["IniSectionGraph"]):
        """
        Combines this graph with another graph

        Parameters
        ----------
        newGraphs: List[:class:`IniSectionGraph`]
            The new graphs to combine with
        """

        ListTools.updateMany(self._targetSectionNames, list(map(lambda graph: graph.targetSectionNames, newGraphs)))
        DictTools.updateMany(self.sections, list(map(lambda graph: graph.sections, newGraphs)))
        self._build(self.sections, self._targetSectionNames)

    @classmethod
    def _initPruning(cls, isCyclePruning: bool, OrderedSet: Optional[Type[OrderedSetType]] = None) -> Union[Set[str], OrderedSetType[str]]:
        if (isCyclePruning and OrderedSet is not None):
            return OrderedSet()
        return set()

    @classmethod
    def iterSectsByContentPart(cls, sections: Dict[str, IfTemplate], roots: List[str], states: int = 1, colour: bool = False, colourKeys: Optional[Set[str]] = None) -> Generator:
        """
        An iterator that iterates through all :class:`IfContentPart` of the `sections`_ using `DFS`_ :raw-html:`<br />` :raw-html:`<br />`

        The iterator returns a :class:`SectionIterData` object

        eg.

        .. code-block:: python
            :linenos:

            for data in IniSectionGraph.iterSectsByContentPart(...):
                ...

        Parameters
        ----------
        sections: Dict[:class:`str`, :class:`IfTemplate`]
            All the `sections`_ in the graph

        roots: List[:class:`str`]
            The name of the root `sections`_ to start searching from

        states: :class:`int`
            The number of states a `section`_ will have (the number of times a `section`_ will be pushed into the stack) :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                For some state ``n``, the order the `sections`_ will be added will be in the opposite order the `sections`_ were added for state ``n - 1`` (like a stack)

                So the order for sibling nodes are processed are different depending on the parity of the state

            .. tip::
                Typically odd parity states will be used for doing some processing while even parity states will be used for cleanup

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``1``

        colour: :class:`bool`
            Whether to keep track of the current state of the `KVPs`_ for each :class:`IfContentPart`

            .. note::
                If this flag is enabled, then 'states' will be changed to an even number bigger or equal to the current number of states specified

            .. note::
                If this value is set to ``True``, then `cycle pruning`_ will be used. Otherwise, `multipath pruning`_ will be used.

            :raw-html:`<br />`

            **Default**: ``False``

        colourKeys: Optional[Set[:class:`str`]]
            The specific keys to keep track of :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart`

            .. note::
                This parameter wil only take affect if 'colour' is set to ``True``

            :raw-html:`<br />`

            **Default**: ``None``

        Yields
        ------
        :class:`SectionIterData`
            The object holding the iteration data

        Returns
        -------
        `Generator`_
            The corresponding iterator
        """

        processStates = states
        if (colour and states % 2 == 1):
            processStates += 1

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet if (colour) else None 
        
        for currentState in range(1, processStates + 1, 2):
            stack = deque()
            visitedParts = cls._initPruning(colour, OrderedSet = OrderedSet)

            colouring = IfContentPartColouring()
            colourChanges = defaultdict(lambda: [])

            for rootName in roots:
                section = sections.get(rootName, None)
                if (section is None):
                    continue

                rootNode = section.tree.root

                if (rootNode is not None and (rootName, rootNode) not in visitedParts):
                    stack.appendleft([rootName, section, rootNode, rootNode, currentState])
                    visitedParts.add((rootName, rootNode))

            if (colour):
                visitedParts.clear()

            while (stack):
                sectionName, currentSection, currentNode, currentPart, state = stack.pop()
                nodeId = (sectionName, currentNode)
                partId = (sectionName, currentPart)
                visitedParts.add(partId)

                isProcessState = state == currentState
                partIsIfContent = isinstance(currentPart, IfContentPart)

                if (state < processStates and isProcessState):
                    newColourChange = colouring.updateColouring(currentPart, targetKeys = colourKeys) if (colour and partIsIfContent) else None
                    stack.append([sectionName, currentSection, currentNode, currentPart, state + 1])

                    if (newColourChange is not None):
                        colourChanges[nodeId].append(newColourChange)

                if (partIsIfContent):
                    if (state <= states):
                        yield SectionIterData(sectionName, currentSection, currentPart, state, colouring = colouring)

                if (not isProcessState):
                    if (colour and not partIsIfContent):
                        nodeColourChanges = colourChanges[nodeId]
                        nodeColourChangesLen = len(nodeColourChanges)

                        for i in range(nodeColourChangesLen - 1, -1, -1):
                            currentColourChange = nodeColourChanges[i]
                            colouring.restore(currentColourChange)

                        del colourChanges[nodeId]

                    if (colour):
                        visitedParts.pop()

                    continue
                
                neighbours = []
                if (partIsIfContent):
                    neighbourData = currentPart.get(IniKeywords.Run.value, default = [], errorOnNotFound = False)

                    for neighbourName in neighbourData:
                        neighbourSection = sections.get(neighbourName, None)
                        if (neighbourSection is None):
                            continue
                        
                        if (neighbourSection is None):
                            continue
                        
                        neighbourRootNode = neighbourSection.tree.root
                        if (neighbourRootNode is not None and (neighbourName, neighbourRootNode) not in visitedParts):
                            neighbours.append([neighbourName, neighbourSection, neighbourRootNode, neighbourRootNode, currentState])

                else:
                    for part in currentPart.parts:
                        if ((sectionName, part) not in visitedParts):
                            neighbourNode = part if (isinstance(part, IfTemplateNode)) else currentPart
                            neighbours.append([sectionName, currentSection, neighbourNode, part, currentState])

                neighboursLen = len(neighbours)
                for i in range(neighboursLen - 1, -1, -1):
                    neighbour = neighbours[i]
                    stack.append(neighbour)

    def iterByContentPart(self, states: int = 1, colour: bool = False, colourKeys: Optional[Set[str]] = None) -> Generator:
        """
        An iterator that iterates through all :class:`IfContentPart` of the `sections`_ of this class using `DFS`_ :raw-html:`<br />` :raw-html:`<br />`

        The iterator returns a :class:`SectionIterData` object

        eg.

        .. code-block:: python
            :linenos:

            for data in x.iterByContentPart(states = 3):
                ...

        Parameters
        ----------
        states: :class:`int`
            The number of states a `section`_ will have (the number of times a `section`_ will be pushed into the stack) :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                See the note for the ``states`` argument at :meth:`iterSectsByContentPart`

            **Default**: ``1``

        colour: :class:`bool`
            Whether to keep track of the current state of the `KVPs`_ for each :class:`IfContentPart`

            .. note::
                If this flag is enabled, then 'states' will be changed to an even number bigger or equal to the current number of states specified

            .. note::
                If this value is set to ``True``, then `cycle pruning`_ will be used. Otherwise, `multipath pruning`_ will be used.

            :raw-html:`<br />`

            **Default**: ``False``

        colourKeys: Optional[Set[:class:`str`]]
            The specific keys to keep track of :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart`

            .. note::
                This parameter wil only take affect if 'colour' is set to ``True``

            :raw-html:`<br />`

            **Default**: ``None``

        Yields
        ------
        :class:`SectionIterData`
            The object holding the iteration data

        Returns
        -------
        `Generator`_
            The corresponding iterator
        """

        return self.iterSectsByContentPart(self.sections, self.roots, states = states, colour = colour, colourKeys = colourKeys)

    def refreshPartIds(self, minimal: bool = True):
        """
        Regenerates the ids of the parts

        Parameters
        ----------
        minimal: :class:`bool`
            only refreshes of the `sections`_ that are part of the graph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``
        """

        if (minimal):
            for sectionName, section in self:
                section.refreshPartIds()
            return

        for sectionName in self.sections:
            self.sections[sectionName].refreshPartIds()

    def deepcopy(self, minimal: bool = True, newPartIds: bool = True) -> "IniSectionGraph":
        """
        Performs a deep copy on the object

        Parameters
        ----------
        minimal: :class:`bool`
            only copy the `sections`_ that are part of the graph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        newPartIds: :class:`bool`
            Whether to refresh the ids for the parts :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`IniSectionGraph`
            The copied graph
        """

        if (not minimal):
            result = copy.deepcopy(self)

            if (newPartIds):
                result.refreshPartIds(minimal = False)

            return result

        neighbours = copy.deepcopy(self.neighbours)
        roots = copy.deepcopy(self.roots)
        targetSectionNames = copy.deepcopy(self._targetSectionNames)

        newSections = {}
        for sectionName, section in self:
            newSections[sectionName] = section.deepcopy(newPartIds = newPartIds)

        result = type(self)(newSections, targetSectionNames, build = False)
        result.neighbours = neighbours
        result.roots = roots
        return result
    
    def rename(self, renameFunc: Callable[[str], str]):
        """
        Renames the `sections` and reconstructs the graph

        Parameters
        ----------
        renameFunc: Callable[[:class:`str`], :class:`str`]
            Function to rename the `section`_. The function takes in the name of the old `section`_
        """

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        stack = deque()
        visitedSections = {}
        renamedSections = {}
        targetSectionNames = OrderedSet(self._targetSectionNames)
        hasRenamed = False

        for root in self.roots:
            stack.appendleft(root)

        # rename the name reference of the sections
        while (stack):
            sectionName = stack.pop()
            if (sectionName in visitedSections):
                continue

            newSectionName = renameFunc(sectionName)
            section = None

            if (newSectionName != sectionName):
                section = self.sections.pop(sectionName, None)
                section.name = newSectionName
                renamedSections[sectionName] = newSectionName

                if (section is not None and newSectionName not in self.sections):
                    self.sections[newSectionName] = section

                if (sectionName in self._targetSectionNames):
                    targetSectionNames.remove(sectionName)
                    targetSectionNames.add(newSectionName)

                if (not hasRenamed):
                    hasRenamed = True
            else:
                section = self.getSection(sectionName)

            visitedSections[sectionName] = section

            neighbours = self.neighbours.get(sectionName, [])
            neighboursLen = len(neighbours)

            for i in range(neighboursLen - 1, -1, -1):
                stack.append(neighbours[i])

        # rename the calls to the section
        for sectionName in visitedSections:
            section = visitedSections[sectionName]
            calledSubCommands = section.calledSubCommands

            for partInd in calledSubCommands:
                ifContentPart = section.parts[partInd]
                partSubCommands = ifContentPart.get(IniKeywords.Run.value, default = [], withInds = True)

                for ind, runVal in partSubCommands:
                    if (runVal in renamedSections):
                        ifContentPart.setValByInd(ind, renamedSections[runVal])

                partSubCommands = ifContentPart.get(IniKeywords.Run.value, default = [])
                calledSubCommands[partInd] = partSubCommands

        if (hasRenamed):
            self._setTargetSectionNames(targetSectionNames)
            self._build(self.sections, self._targetSectionNames)

    @property
    def targetSectionNames(self) -> List[str]:
        """
        Names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini file

        :getter: The names of the desired `sections`_ we want in the subgraph
        :setter: Constructs a new subgraph based on the new desired `sections`_ we want
        :type: List[:class:`str`]
        """

        return self._targetSectionNames
    
    def _setTargetSectionNames(self, newTargetSections: Union[Set[str], List[str]]):
        self._targetSectionNames = ListTools.getDistinct(newTargetSections, keepOrder = True)
    
    @targetSectionNames.setter
    def targetSections(self, newTargetSections: Union[Set[str], List[str]]):
        self._setTargetSectionNames(newTargetSections)
    
    def _build(self, sections: Dict[str, IfTemplate], targetSectionNames: List[str], copySections: bool = False):
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        self.roots.clear()
        self.neighbours.clear()

        visited = set()
        neighbours = defaultdict(lambda: OrderedSet())
        parents = {}
        resultSections = {}

        for sectionName in targetSectionNames:
            section = sections.get(sectionName)
            if (sectionName in visited or section is None):
                 continue
            
            visited.add(sectionName)
            stack = deque([(sectionName, section)])
            resultSections[sectionName] = section

            # perform the main DFS algorithm
            while (stack):
                currentSectionName, currentSection = stack.pop()
                calledSubCommands = currentSection.calledSubCommands
                currentToExplore = deque()

                for partInd in calledSubCommands:
                    subSections = calledSubCommands[partInd]

                    for subSectionName in subSections:
                        # we assume the .ini file has correct syntax and does not reference some
                        #   command that does not exist. It is not within this project's scope to help the
                        #   person fix their own mistakes in the .ini file. Assume that an incorrect referenced
                        #   command refers to some global command not in the file. So this command will be a sink in the
                        #   command call graph and a leaf in the DFS tree 
                        neighbourSection = sections.get(subSectionName)
                        if (neighbourSection is None):
                            continue

                        neighbours[currentSectionName].add(subSectionName)

                        if (subSectionName not in parents):
                            parents[subSectionName] = currentSectionName

                        if (subSectionName not in visited):
                            visited.add(subSectionName)
                            resultSections[subSectionName] = neighbourSection
                            currentToExplore.append((subSectionName, neighbourSection))
                
                stack.extend(currentToExplore)

        # clean the children
        for sectionName in neighbours:
            neighbours[sectionName] = list(neighbours[sectionName])

        self.neighbours = dict(neighbours)

        # get the roots
        roots = OrderedSet()
        nodeRoots = {}

        for sectionName in targetSectionNames:
            currentSectionName = sectionName
            pathNodes = {currentSectionName}
            addRoot = True

            while (currentSectionName in parents):
                currentSectionName = parents[currentSectionName]

                if (currentSectionName in nodeRoots):
                    currentSectionName = nodeRoots[currentSectionName]
                    break

                pathNodes.add(currentSectionName)

                if (currentSectionName in roots):
                    addRoot = False
                    break

                if (currentSectionName == sectionName):
                    break

            if (addRoot):
                roots.add(currentSectionName)
                for node in pathNodes:
                    nodeRoots[node] = currentSectionName

        self.roots = list(roots)

        if (copySections):
            resultSections = copy.deepcopy(resultSections)

        self.sections = resultSections

    def getRootSections(self) -> List[IfTemplate]:
        """
        Retrieves the `sections`_ corresponding to the roots of the graph

        Returns
        -------
        List[:class:`IfTemplate`]
            The corresponding `sections`_
        """

        result = []
        for root in self.roots:
            result.append(self.getSection(root))

        return result
    
    def isEmpty(self) -> bool:
        """
        Determines whether the graph is empty

        Returns
        -------
        :class:`bool`
            Whether the graph is empty
        """

        return not bool(self.roots)
    
    def getNeighbourNames(self, sectionName: str) -> List[str]:
        """
        Retrieves the names of the out-neighbour `sections`_

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the current `section`_

        Returns
        -------
        List[:class:`str`]
            The names of the out-neighbour `sections`_
        """

        return self.neighbours.get(sectionName, [])
    
    def getNeighbours(self, sectionName: str) -> Dict[str, IfTemplate]:
        """
        Retrieves the out-neighbours of some `section`_

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the current `section`_

        Returns
        -------
        Dict[:class:`str`, :class:`IfTemplate`]
            The out-neighbourhood :raw-html:`<br />` :raw-html:`<br />`

            The keys contain the names of the neighbour `sections`_ and the values are the corresponding `sections`_
        """

    def build(self, sections: Optional[Dict[str, IfTemplate]] = None, targetSectionNames: Optional[Union[Set[str], List[str]]] = None, copySections: bool = False):
        """
        Constructs the subgraph for the `sections`_ using `DFS`_

        Parameters
        ----------
        sections: Optional[Dict[:class:`str`, :class:`IfTemplate`]]
            All the `sections`_ of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the `sections`_ and the values are the `sections`_ :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :attr:`sections` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        targetSectionNames: Union[Set[:class:`str`], List[:class:`str`]]
            The new names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :attr:`targetSectionNames` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        copySections: :class:`bool`
            Whether to do a deep copy of the `sections`_ traversed by the graph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (sections is None):
            sections = self.sections
        else:
            self.sections = sections

        if (targetSectionNames is None):
            targetSectionNames = self.targetSectionNames
        else:
            self._setTargetSectionNames(targetSectionNames)

        self._build(sections, targetSectionNames, copySections = copySections)

    def getSection(self, sectionName: str, raiseException: bool = True) -> Optional[IfTemplate]:
        """
        Retrieves the :class:`IfTemplate` for a certain `section`_

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_

        raiseException: :class:`bool`
            Whether to raise an exception when the section's :class:`IfTemplate` is not found

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` for the `section`_ is not found and ``raiseException`` is set to ``True``

        Returns
        -------
        Optional[:class:`IfTemplate`]
            The corresponding :class:`IfTemplate` for the `section`_
        """
        result = self.sections.get(sectionName)
        if (result is None and raiseException):
            raise KeyError(f"The section by the name '{sectionName}' does not exist")
        
        return result
    
    def isKeyFullyCover(self, key: str) -> Dict[str, bool]:
        """
        Determines whether a key fully covers all the conditional branches of a `section`_

        Parameters
        ----------
        key: :class:`key`
            The target key to search

        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            The result for each `section`_ of whether the section has the key fully covering all its conditional branches :raw-html:`<br />` :raw-html:`<br />`

            .. tip::
                To filter only the result for `sections`_ that are the source nodes of the graph, you can call :meth:`targetsAreFullyCovered` instead
        """

        visited = set()
        sections = {}
        sectionsKeyFullCover = {}

        for sectionName in self.roots:
            ifTemplate = self.getSection(sectionName)
            sections[sectionName] = ifTemplate

        for sectionName in sections:
            section = sections[sectionName]
            section.isKeyFullyCover(key, self.sections, visited, sectionsKeyFullCover)

        return sectionsKeyFullCover
    
    def rootsAreFullyCovered(self, key: str) -> Dict[str, bool]:
        """
        Convenience function of :meth:`isKeyFullyCover` to determine whether the target `sections`_ from :meth:`targetSections` are
        fully covered by a key in all their conditional branches

        Parameters
        ----------
        key: :class:`key`
            The target key to search

        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            The result for the target `sections`_ of whether the section has the key fully covering all its conditional branches
        """

        sectionsKeyFullCover = self.isKeyFullyCover(key)
        
        result = {}
        for sectionName in self.roots:
            result[sectionName] = sectionsKeyFullCover[sectionName]

        return result
    
    def getKeyMissingParts(self, key: str) -> Dict[str, Set[IfContentPart]]:
        """
        Retrieves the parts in the `sections`_ that are not covered by 'key'

        Parameters
        ----------
        key: :class:`key`
            The target key to search

        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            The result for each `section`_ regarding which :class:`IfContentPart`\\s are not covered by 'key' :raw-html:`<br />` :raw-html:`<br />`
        """

        visited = set()
        sections = {}
        sectionsMissingParts = {}
        sectionAllBranchesMissing = {}

        for sectionName in self.roots:
            ifTemplate = self.getSection(sectionName)
            sections[sectionName] = ifTemplate

        for sectionName in sections:
            section = sections[sectionName]
            section.getKeyMissingParts(key, self.sections, visited, sectionsMissingParts, sectionAllBranchesMissing)

        return sectionsMissingParts
    
    def getChildren(self, targetSections: List[str], getNeighbourChildren: bool = True) -> Dict[str, OrderedSetType[str]]:
        """
        Retrieves the children `sections`_ of the `sections`_ specified at 'targetSections', by reading in a `DFS`_ manner,
        starting from the `sections`_ specified at 'targetSections'

        Parameters
        ----------
        targetSections: :class:`str`
            The name of the `sections`_ to retrieve the children

        getNeighbourChildren: :class:`bool`
            Whether to also retrieve the children for the neighbours of the `sections`_ specified at 'targetSections' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        Dict[:class:`str`, `OrderedSet`_[:class:`str`]]
            The names of the children `sections`_ for the `sections`_ specified at 'targetSections' and their neighbours
        """

        result  = {}
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        exploreState = 0
        addState = 1
        targetSections = OrderedSet(targetSections)

        for section in targetSections:
            visited = set()
            stack = deque([(exploreState, section)])

            while (stack):
                state, currentSection = stack.pop()
                neighbours = self.neighbours.get(currentSection, [])

                if (state == exploreState):
                    stack.append((addState, currentSection))

                    for neighbour in neighbours:
                        if (neighbour not in visited):
                            stack.append((exploreState, neighbour))
                            visited.add(neighbour)

                    continue

                currentResult = OrderedSet()
                for neighbour in neighbours:
                    currentResult.add(neighbour)

                    if (neighbour in result):
                        currentResult.update(result[neighbour])

                result[currentSection] = currentResult

        for section in targetSections:
            if (section not in result):
                result[section] = OrderedSet()

        if (getNeighbourChildren):
            return result

        resultKeys = list(result.keys())

        for section in resultKeys:
            if (section not in targetSections):
                del result[section]

        return result
    
    def getCommonMods(self, hashRepo: Hashes, indexRepo: Indices, version: Optional[float] = None) -> Set[str]:
        """
        Retrieves the common mods to fix to based off all the :class:`IfTemplate`\\s in the graph

        Parameters
        ----------
        hashRepo: :class:`Hashes`
            The data source for all the hashes

        indexRepo: :class:`Indices`
            The data source for all the indices

        version: Optional[:class:`float`]
            The version we want to fix to :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will assume we want to fix to the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Set[:class:`str`]
            The common mods to fix to
        """

        result = set()

        for sectionName in self.sections:
            ifTemplate = self.sections[sectionName]
            ifTemplateMods = ifTemplate.getMods(hashRepo, indexRepo, version = version)

            if (not result):
                result = ifTemplateMods
            elif (ifTemplateMods):
                result = result.intersection(ifTemplateMods)

        return result
    
    @staticmethod
    def computeSectionPredecessors(parts: List[Any]) -> Dict[int, List[int]]:
        """
        Computes, for every :class:`IfContentPart` in 'parts' (a `section`_'s flat, textually-ordered
        ``[IfContentPart | IfPredPart, ...]`` list), the ``id()`` of every :class:`IfContentPart` that must
        run immediately before it on some path through this `section`_ alone (ie. ignoring any ``run =`` call) :raw-html:`<br />` :raw-html:`<br />`

        A part with no such predecessors (an empty list) is one of this `section`_'s own entry points -- either
        the very first part of the `section`_, or the first part of a `branch`_ that starts right at the top
        (no content precedes the very first ``if``) :raw-html:`<br />` :raw-html:`<br />`

        Branching (``if``/``elif``/``else``) forks the "current" predecessor set into one independent copy per
        `branch`_; the part right after the matching ``endIf`` inherits from *every* branch's own ending part
        (plus whatever came right before the ``if``, if there's no ``else`` covering the "no branch taken" case)

        Parameters
        ----------
        parts: List[Union[:class:`IfContentPart`, :class:`IfPredPart`]]
            The flat parts of a `section`_ (ie. :attr:`IfTemplate.parts`)

        Returns
        -------
        Dict[:class:`int`, List[:class:`int`]]
            The ``id()`` of every :class:`IfContentPart` in 'parts', mapped to the ``id()`` of its predecessors
        """

        predecessors: Dict[int, List[int]] = {}
        current: List[int] = []
        stack: List[Dict[str, Any]] = []

        for part in parts:
            if (isinstance(part, IfContentPart)):
                predecessors[id(part)] = list(current)
                current = [id(part)]
                continue

            predType = part.type
            if (predType == IfPredPartType.If):
                stack.append({"preEntry": current, "branchExits": [], "hasElse": False})
                current = list(stack[-1]["preEntry"])
            elif (predType == IfPredPartType.Elif):
                stack[-1]["branchExits"].append(current)
                current = list(stack[-1]["preEntry"])
            elif (predType == IfPredPartType.Else):
                stack[-1]["branchExits"].append(current)
                stack[-1]["hasElse"] = True
                current = list(stack[-1]["preEntry"])
            elif (predType == IfPredPartType.EndIf):
                frame = stack.pop()
                exits = frame["branchExits"] + [current]
                if (not frame["hasElse"]):
                    exits.append(frame["preEntry"])

                merged = []
                for group in exits:
                    for pid in group:
                        if (pid not in merged):
                            merged.append(pid)

                current = merged

        return predecessors

    def buildPartPredecessorGraph(self) -> Dict[int, List[int]]:
        """
        Builds a graph-wide version of :meth:`computeSectionPredecessors`, additionally linking a ``run =``
        call's own part as a predecessor of whatever `section`_ it calls into (specifically, of that callee
        `section`_'s own entry points, as :meth:`computeSectionPredecessors` defines them) :raw-html:`<br />` :raw-html:`<br />`

        Unlike :meth:`buildCallGraph`, this only needs to know "who came before me" -- there's no notion of what
        happens after a callee returns, and no virtual "continuation" nodes. That makes it well-suited for
        *deduping* work across every part relationship this kind of analysis cares about (both within one
        `section`_ -- sequential parts, and parts on either side of an ``if``/``endIf`` -- and across `section`_\\s
        via a ``run =`` call) using one uniform mechanism, instead of leaning on :meth:`iterByContentPart`'s own
        `DFS`_ pre/post-order pairing (which only happens to line up with this for the ``run =`` case) :raw-html:`<br />` :raw-html:`<br />`

        A part that (transitively) calls into its own `section`_ (directly or through a cycle of ``run =`` calls)
        ends up listed as its own predecessor -- this is harmless for dedup purposes, since a part is only ever
        visited (and decided) once by a traversal like :meth:`iterByContentPart`

        Returns
        -------
        Dict[:class:`int`, List[:class:`int`]]
            The ``id()`` of every :class:`IfContentPart` reachable in this graph, mapped to the ``id()`` of its
            predecessors
        """

        predecessors: Dict[int, List[int]] = {}
        entryPoints: Dict[str, List[int]] = {}

        for sectionName, section in self.sections.items():
            sectionPredecessors = self.computeSectionPredecessors(section.parts)
            predecessors.update(sectionPredecessors)
            entryPoints[sectionName] = [pid for pid, preds in sectionPredecessors.items() if (not preds)]

        for section in self.sections.values():
            for part in section.parts:
                if (not isinstance(part, IfContentPart)):
                    continue

                for target in part.getVals(IniKeywords.Run.value):
                    for entryId in entryPoints.get(target, []):
                        entryPredecessors = predecessors.setdefault(entryId, [])
                        if (id(part) not in entryPredecessors):
                            entryPredecessors.append(id(part))

        return predecessors

    def buildCallGraph(self) -> CallGraph:
        """
        Builds a `call graph`_ over the :class:`IfContentPart`\\s of this :class:`IniSectionGraph`, modeling
        ``run =`` as a `call-with-return`_ (a call whose callee eventually hands control back to whatever comes
        after the call, rather than a plain `goto`_) :raw-html:`<br />` :raw-html:`<br />`

        Unlike :meth:`buildPartPredecessorGraph` (which only needs to know "who came before me", for dedup),
        analyses that need to reason about what happens *after* a call returns -- eg. the `dataflow analysis`_
        tools at :class:`GraphTools` -- need a "continue here once every call this part makes has returned" node,
        distinct from the calling part itself (whose own edges instead point *into* the call) :raw-html:`<br />` :raw-html:`<br />`

        Nodes are either the ``id()`` of a real :class:`IfContentPart`, or a virtual ``("exit", id(part))`` node
        (only created for a part that actually makes a ``run =`` call) representing that "continue here" point.
        A part *without* any ``run =`` call has no need for a separate exit node -- its own node already serves
        that purpose, since there's no call to distinguish "before" from "after" (see :meth:`CallGraph.exitNodeOf`)

        Returns
        -------
        :class:`CallGraph`
            The built `call graph`_
        """

        partsById: Dict[int, IfContentPart] = {}
        withinSuccessors: Dict[int, List[int]] = defaultdict(list)
        sectionOfPart: Dict[int, str] = {}
        callTargetsOfPart: Dict[int, List[str]] = {}
        callersOfSection: Dict[str, List[int]] = defaultdict(list)
        entryPointsOfSection: Dict[str, List[int]] = {}

        for sectionName, section in self.sections.items():
            sectionPredecessors = self.computeSectionPredecessors(section.parts)
            entryPointsOfSection[sectionName] = [pid for pid, preds in sectionPredecessors.items() if (not preds)]

            succ: Dict[int, List[int]] = defaultdict(list)
            for pid, preds in sectionPredecessors.items():
                for predId in preds:
                    succ[predId].append(pid)

            for part in section.parts:
                if (not isinstance(part, IfContentPart)):
                    continue

                partsById[id(part)] = part
                sectionOfPart[id(part)] = sectionName
                withinSuccessors[id(part)] = succ.get(id(part), [])

                targets = part.getVals(IniKeywords.Run.value)
                callTargetsOfPart[id(part)] = targets
                for target in targets:
                    callersOfSection[target].append(id(part))

        forwardEdges: Dict[Any, List[Any]] = defaultdict(list)

        def exitNodeOf(pid: int) -> Any:
            return ("exit", pid) if (callTargetsOfPart.get(pid)) else pid

        for pid in partsById:
            targets = callTargetsOfPart.get(pid, [])

            if (targets):
                for target in targets:
                    for entryId in entryPointsOfSection.get(target, []):
                        forwardEdges[pid].append(entryId)
                selfExit = ("exit", pid)
            else:
                selfExit = pid

            succs = withinSuccessors.get(pid, [])
            if (succs):
                for succ in succs:
                    forwardEdges[selfExit].append(succ)
            else:
                for callerId in callersOfSection.get(sectionOfPart[pid], []):
                    forwardEdges[selfExit].append(exitNodeOf(callerId))

        backwardEdges: Dict[Any, List[Any]] = defaultdict(list)
        for src, dsts in forwardEdges.items():
            for dst in dsts:
                backwardEdges[dst].append(src)

        rootNodeIds: Set[int] = set()
        for rootName in self.roots:
            rootNodeIds.update(entryPointsOfSection.get(rootName, []))

        return CallGraph(forwardEdges, backwardEdges, partsById, rootNodeIds)

    def normalize(self):
        """
        Normalizes the branching structure of all the `sections`_ in :attr:`sections` :raw-html:`<br />` :raw-html:`<br />`

        See :meth:`IfTemplate.normalize` for more details
        """

        for sectionName in self.sections:
            self.sections[sectionName].normalize()

    def toStr(self, autoindent: bool = True) -> str:
        """
        Converts all the `sections`_ to a string

        Parameters
        ----------
        autoIndent: :class:`bool`
            Whether to compute the proper tab indent for the `sections`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`str`
            The string representation 
        """

        result = []
        stack = deque()
        visited = set()
    
        for root in self.roots:
            stack.appendleft(root)

        while (stack):
            sectionName = stack.pop()
            section = self.getSection(sectionName)
            visited.add(sectionName)

            result.append(section.toStr(autoindent = autoindent))

            neighbours = self.neighbours.get(sectionName, [])
            neighboursLen = len(neighbours)

            for i in range(neighboursLen - 1, -1, -1):
                stack.append(neighbours[i])

        return "\n\n".join(result)
    
    def _getQuery(self, queryPath: List[Union[bool, SympBooleanType]], sympy, simplify: bool = False) -> Union[bool, SympBooleanType]:
        query = True
        queryPathLen = len(queryPath)

        if (queryPathLen == 1):
            query = queryPath[0]
        elif (queryPathLen > 1):
            query = sympy.And(*queryPath)

        if (simplify and queryPathLen >= 1):
            query = sympy.simplify(query)

        return query
    
    def iterByQuery(self, queryPath: Optional[Union[List[Union[bool, SympBooleanType]], Union[bool, SympBooleanType]]] = None, 
                    simplify: bool = False, states: int = 1, colour: bool = False, colourKeys: Optional[Set[str]] = None) -> Generator:
        """
        An iterator that iterates through all the :class:`IfContentPart`\\s of the graph and also retrieves the conditional logical predicate that each :class:`IfContentPart` resides in. :raw-html:`<br />` :raw-html:`<br />`

        The iterator returns a :class:`SectionIterQueryData` object

        eg.

        .. code-block:: python
            :linenos:

            for data in x.iterByQuery(...):
                ...

        Parameters
        ----------
        queryPath: Optional[List[Union[:class:`bool`, `sympy.Boolean`_]]]
            The starting conditional logic predicate at the root of the graph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        simplify: :class:`bool`
            Whether to simplify the logic of the conditional predicates :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        states: :class:`int`
            The number of states a `section`_ will have (the number of times a `section`_ will be pushed into the stack) :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                For some state ``n``, the order the `sections`_ will be added will be in the opposite order the `sections`_ were added for state ``n - 1`` (like a stack)

                So the order for sibling nodes are processed are different depending on the parity of the state

            .. tip::
                Typically odd parity states will be used for doing some processing while even parity states will be used for cleanup

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``1``

        colour: :class:`bool`
            Whether to keep track of the current state of the `KVPs`_ for each :class:`IfContentPart`

            .. note::
                If this flag is enabled, then 'states' will be changed to an even number bigger or equal to the current number of states specified

            .. note::
                If this value is set to ``True``, then `cycle pruning`_ will be used. Otherwise, `multipath pruning`_ will be used.

            :raw-html:`<br />`

            **Default**: ``False``

        colourKeys: Optional[Set[:class:`str`]]
            The specific keys to keep track of :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart`

            .. note::
                This parameter wil only take affect if 'colour' is set to ``True``

            :raw-html:`<br />`

            **Default**: ``None``

        Yields
        ------
        :class:`SectionIterQueryData`
            The object holding the iteration data

        Returns
        -------
        `Generator`_
            The corresponding iterator
        """

        userStates = states
        if (colourKeys and states % 2 == 1):
            states += 1

        for addState in range(1, states + 1, 2):
            if (queryPath is None):
                queryPath = []
            elif (not isinstance(queryPath, list)):
                queryPath = [queryPath]

            queryCount = deque([0])
            stack = deque()
            visitedSections = set()
            kvps = {}
            startDepth = 0

            colouring = IfContentPartColouring()
            colourChanges = defaultdict(lambda: [])

            exploreState = 0
            cleanState = 1

            sympy = GlobalPackageManager.get(PackageModules.Sympy.value)

            rootSectionNames = self.roots
            for sectionName in rootSectionNames:
                section = self.getSection(sectionName)
                rootNode = section.tree.root

                if (rootNode is not None):
                    stack.appendleft((exploreState, sectionName, section, rootNode, rootNode, startDepth, sectionName, section))

            while (stack):
                state, sectionName, section, currentNode, part, depth, rootSectionName, rootSection = stack.pop()
                nodeId = (sectionName, currentNode)
                visitedSections.add(sectionName)

                if (state == cleanState):
                    if (isinstance(part, IfContentPart)):
                        clearState = addState + 1
                        if (cleanState <= states and clearState <= userStates):
                            query = self._getQuery(queryPath, sympy, simplify = simplify)
                            yield SectionIterQueryData(part, query, sectionName, section, rootSectionName, rootSection, addState + 1, colouring = colouring)

                        continue

                    isLastChild = not stack
                    isEndIf = False

                    if (stack):
                        _, _, _, _, nextPart, nextDepth, _, _ = stack[-1]
                        isLastChild = nextDepth < depth
                        isEndIf = isinstance(nextPart, IfContentPart) or nextPart.ifPredPart is None or nextPart.ifPredPart.type == IfPredPartType.If if (not isLastChild) else True

                    if (isLastChild or isEndIf):
                        childrenQueryCount = queryCount[depth]
                        for i in range(childrenQueryCount):
                            queryPath.pop()

                        queryCount[depth] = 0

                    else:
                        queryPath[-1] = sympy.Not(queryPath[-1])

                    if (isLastChild):
                        queryCount.pop()

                    if (colour):
                        nodeColourChanges = colourChanges[nodeId]
                        nodeColourChangesLen = len(nodeColourChanges)

                        for i in range(nodeColourChangesLen - 1, -1, -1):
                            currentColourChange = nodeColourChanges[i]
                            colouring.restore(currentColourChange)

                        del colourChanges[nodeId]

                    continue
                
                if (isinstance(part, IfContentPart)):
                    query = self._getQuery(queryPath, sympy, simplify = simplify)
                    newColourChange = colouring.updateColouring(part, targetKeys = colourKeys) if (colour) else None

                    if (newColourChange is not None):
                        colourChanges[nodeId].append(newColourChange)

                    yield SectionIterQueryData(part, query, sectionName, section, rootSectionName, rootSection, addState, colouring)
                    stack.append((cleanState, sectionName, section, currentNode, part, depth, rootSectionName, rootSection))

                    childSectionNames = part.get(IniKeywords.Run.value, default = [])
                    childSectionNamesLen = len(childSectionNames)
                    
                    for i in range(childSectionNamesLen - 1, -1, -1):
                        childSectionName = childSectionNames[i]
                        if (childSectionName not in self.sections or childSectionName in visitedSections or childSectionName not in self.sections):
                            continue

                        childSection = self.sections[childSectionName]

                        childRoot = childSection.tree.root
                        if (childRoot is None):
                            continue

                        stack.append((exploreState, childSectionName, childSection, childRoot, childRoot, depth + 1, rootSectionName, rootSection))

                    continue

                ifPredPart = part.ifPredPart

                if (ifPredPart is not None and ifPredPart.query is not None):
                    stack.append((cleanState, sectionName, section, part, part, depth, rootSectionName, rootSection))
                    queryPath.append(ifPredPart.query)
                    queryCount[depth] += 1

                children = part.parts
                childrenLen = len(children)

                for i in range(childrenLen - 1, -1, -1):
                    child = children[i]
                    stack.append((exploreState, sectionName, section, part, child, depth + 1, rootSectionName, rootSection))

                queryCount.append(0)

    def processIfContentByQuery(self, processIfContent: Callable[[IfContentPart, Union[bool, SympBooleanType], str, IfTemplate], Any], 
                                queryPath: Optional[Union[List[Union[bool, SympBooleanType]], Union[bool, SympBooleanType]]] = None, 
                                simplify: bool = False, states: int = 1, colour: bool = False, colourKeys: Optional[Set[str]] = None):
        """
        Processes all :class:`IfContentPart`\\s of the graph that requires the conditional logic predicate that the :class:`IfContentPart` resides in.

        Parameters
        ----------
        processIfContent: Callable[[:class:`SectionIterQueryData`], Any]
            The function for processing the :class:`IfContentPart`

        queryPath: Optional[List[Union[:class:`bool`, `sympy.Boolean`_]]]
            The starting conditional logic predicate at the root of the graph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        simplify: :class:`bool`
            Whether to simplify the conditional logic predicate found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        states: :class:`int`
            The number of states a `section`_ will have (the number of times a `section`_ will be pushed into the stack) :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                For some state ``n``, the order the `sections`_ will be added will be in the opposite order the `sections`_ were added for state ``n - 1`` (like a stack)

                So the order for sibling nodes are processed are different depending on the parity of the state

            .. tip::
                Typically odd parity states will be used for doing some processing while even parity states will be used for cleanup

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``1``

        colour: :class:`bool`
            Whether to keep track of the current state of the `KVPs`_ for each :class:`IfContentPart`

            .. note::
                If this flag is enabled, then 'states' will be changed to an even number bigger or equal to the current number of states specified

            .. note::
                If this value is set to ``True``, then `cycle pruning`_ will be used. Otherwise, `multipath pruning`_ will be used.

            :raw-html:`<br />`

            **Default**: ``False``

        colourKeys: Optional[Set[:class:`str`]]
            The specific keys to keep track of :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart`

            .. note::
                This parameter wil only take affect if 'colour' is set to ``True``

            :raw-html:`<br />`

            **Default**: ``None``
        """

        for data in self.iterByQuery(queryPath = queryPath, simplify = simplify, states = states, colour = colour, colourKeys = colourKeys):
            processIfContent(data)
##### EndScript