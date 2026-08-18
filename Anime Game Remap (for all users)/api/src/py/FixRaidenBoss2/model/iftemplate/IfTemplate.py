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
from typing import List, Union, Dict, Any, Optional, Set, Callable, Tuple, Type
##### EndExtImports

##### CppLocalImports
from ...core import IfTemplatePart
from ...core import IfContentPart
from ...core import ParseContext
##### EndCppLocalImports

##### LocalImports
from ...constants.Packages import PackageModules
from ...constants.IniConsts import IniKeywords
from ...constants.IfPredPartType import IfPredPartType
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...constants.GenericTypes import SymbolType
from ..assets.Hashes import Hashes
from ..assets.Indices import Indices
from .IfPredPart import IfPredPart
from .IfTemplateTree import IfTemplateTree, IfTemplateNonEmptyNodeTree, IfTemplateNormTree
from .IfTemplateNode import IfTemplateNode
##### EndLocalImports


##### Script
# IfTemplate: Data class for the if..else template of the .ini file
class IfTemplate():
    """
    Data for storing information about a `section`_ in a .ini file

    :raw-html:`<br />`

    .. note::
        Assuming every `if/else` clause must be on its own line, we have that an :class:`IfTemplate` have a form looking similar to this:

        .. code-block:: ini
            :linenos:
            :emphasize-lines: 1,2,5,7,12,16,17

            ...(does stuff)...
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                endif
            endif
            ...(does stuff)...
            ...(does stuff)...

        We split the above structure into parts (:class:`IfTemplatePart`) where each part is either:

        #. **An If Predicate Part (:class:`IfPredPart`)**: a single line containing the keywords "if", "else" or "endif" :raw-html:`<br />` **OR** :raw-html:`<br />`
        #. **A Content Part (:class:`IfContentPart`)**: a group of lines that *"does stuff"*

        **Note that:** an :class:`ifTemplate` does not need to contain any parts containing the keywords "if", "else" or "endif". This case covers the scenario
        when the user does not use if..else statements for a particular `section`_
        
        Based on the above assumptions, we can assume that every ``[section]`` in a .ini file contains this :class:`IfTemplate`

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for element in x

            Iterates over all the parts of the :class:`IfTemplate`, ``x``

        .. describe:: x[num]

            Retrieves the part from the :class:`IfTemplate`, ``x``, at index ``num``

        .. describe:: x[num] = newPart

            Sets the part at index ``num`` of the :class:`IfTemplate`, ``x``, to have the value of ``newPart``

    :raw-html:`<br />`

    Parameters
    ----------
    parts: List[:class:`IfTemplatePart`]
        The individual parts of how we divided an :class:`IfTemplate` described above

    name: :class:`str`
        The name of the `section`_ for this :class:`IfTemplate`

        **Default**: ``""``

    treeCls: Type[:class:`IfTemplateTree`]
        The class to construct the parse tree for the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :class:`IfTemplateNonEmptyNodeTree`

    prefix: :class:`str`
        Any prefix that precedes the content :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``""``

    suffix: :class:`str`
        Any suffix that follows the content :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``""``

    Attributes
    ----------
    parts: List[:class:`IfTemplatePart`]
        The individual parts of how we divided an :class:`IfTemplate` described above

    partsById: Dict[:class:`int`, :class:`IfTemplatePart`]
        The indivdual parts, grouped by their ids :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids of the parts and the values are the parts

    treeCls: Type[:class:`IfTemplateTree`]
        The class to construct the parse tree for the :class:`IfTemplate`

    tree: :class:`IfTemplateTree`
        The parse tree for the :class:`IfTemplate` . Details on the structure of the tree can be found at :class:`IfTemplateTree`

    prefix: :class:`str`
        Any prefix that precedes the content

    suffix: :class:`str`
        Any suffix that follows the content

    calledSubCommands: Dict[:class:`int`, List[Tuple[:class:`str`]]]
        Any other sections that this :class:`IfTemplate` references :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` in the :class:`IfTemplate` that the section is called
        * The values are the corresponding referenced `sections`_ for each part
    """

    def __init__(self, parts: List[IfTemplatePart], name: str = "", treeCls: Type[IfTemplateTree] = IfTemplateNonEmptyNodeTree, prefix: str = "", suffix: str = ""):
        self.name = name
        self.parts = parts
        self.partsById = {}

        self.calledSubCommands: Dict[int, List[str]] = {}

        self.treeCls = treeCls
        self.tree = None
        self.prefix = prefix
        self.suffix = suffix
        
        self.rebuild()

    def _hasNeededAtts(self, ifTemplate, partIndex: int, part: IfTemplatePart) -> bool:
        return isinstance(part, IfContentPart) and (IniKeywords.Run.value in part)
    
    def _setupIfTemplateAtts(self, ifTemplate, partIndex: int, part: IfContentPart):
        ifTemplate.calledSubCommands[partIndex] = part[IniKeywords.Run.value]

    def setupPartsById(self) -> Dict[int, IfTemplatePart]:
        result = {}
        for part in self.parts:
            result[part.id] = part

        return result

    def rebuild(self):
        """
        Updates the parse tree and the reference to other sections that call this objects
        """

        self.partsById = self.setupPartsById()
        self.tree = self.treeCls.construct(self.parts)
        self.find(pred = self._hasNeededAtts, postProcessor = self._setupIfTemplateAtts)

    def refreshPartIds(self):
        """
        Regenerates the ids for the parts
        """

        for part in self.parts:
            part.refreshId()

        self.partsById = self.setupPartsById()

    def deepcopy(self, newPartIds: bool = True):
        """
        Performs a deep copy on the object

        Parameters
        ----------
        newPartIds: :class:`bool`
            whether to refresh to the ids for each part :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`IfTemplate`
            The copied object
        """

        result = copy.deepcopy(self)
        if (not newPartIds):
            return result

        result.refreshPartIds()
        return result

    @classmethod
    def build(cls, rawParts: List[Tuple[int, Union[str, Dict[str, List[Tuple[int, str]]]]]], name: str = "", ctx: Optional[ParseContext] = None, vars: Optional[Dict[str, SymbolType]] = None):
        """
        Builds the :class:`IfTemplate`

        Parameters
        ----------
        rawParts: List[Tuple[:class:`int`, Union[:class:`str`, Dict[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]]]]
            The list of raw parts found :raw-html:`<br />` :raw-html:`<br />`

            Each tuple includes:

            #. The starting line number for the part
            #. The corresponding part

            :raw-html:`<br />` :raw-html:`<br />`

            If the part is a string, then the part should correspond to a conditional predicate that will be built using :class:`IfPredPart`
            If the part is a dictionary, then the part should correspond to :attr:`IfContentPart.src` that will be built using :class:`IfContentPart`

        ctx: Optional[:class:`ParseContext`]
            The context for parsing the conditional predicate :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        vars: Optional[Dict[:class:`str`, `sympy.Symbol`_]]
            The variables to save for the corresponding .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (ctx is None):
            ctx = ParseContext()

        if (vars is None):
            vars = {}

        parts = []
        rawPartsLen = len(rawParts)
        depth = 0

        for i in range(rawPartsLen):
            lineNo, rawPart = rawParts[i]
            part = None

            if (isinstance(rawPart, str)):
                predType = IfPredPartType.getType(rawPart)
                if (predType is None):
                    continue
                elif (predType == IfPredPartType.If):
                    depth += 1
                elif (predType == IfPredPartType.EndIf):
                    depth -= 1

                ctx.startLineNo = lineNo
                part = IfPredPart(rawPart, predType, ctx = ctx, vars = vars)

            elif (isinstance(rawPart, dict)):
                part = IfContentPart(rawPart, depth)

            if (part is not None):
                parts.append(part)

        return cls(parts, name = name)

    def __iter__(self):
        return self.parts.__iter__()
    
    def __getitem__(self, key: int) -> Union[str, Dict[str, Any]]:
        return self.parts[key]
    
    def __setitem__(self, key: int, value: Union[str, Dict[str, Any]]):
        self.parts[key] = value

    def normalize(self):
        """
        Normalizes the branching structure within this :class:`ifTemplate` to follow the structure described at :class:`IfTemplateNormTree`
        """

        self.tree = IfTemplateNormTree.construct(self.parts)

    def add(self, part: IfTemplatePart, updateTree: bool = False):
        """
        Adds a part to the :class:`ifTemplate`

        Parameters
        ----------
        part: :class:`IfTemplatePart`
            The part to add to the :class:`IfTemplate`

        updateTree: :class:`bool`
            Whether to update the parse tree for the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """
        self.parts.append(part)
        self.partsById[part.id] = part

        if (updateTree):
            self.tree = IfTemplateTree.construct(self.parts)

    # find(pred, postProcessor): Searches each part in the if template based on 'pred'
    def find(self, pred: Optional[Callable[["IfTemplate", int, IfTemplatePart], bool]] = None, postProcessor: Optional[Callable[["IfTemplate", int, IfTemplatePart], Any]] = None) -> Dict[int, Any]:
        """
        Searches the :class:`IfTemplate` for parts that meet a certain condition

        Parameters
        ----------
        pred: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], :class:`bool`]]
            The predicate used to filter the parts :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then this function will return all the parts :raw-html:`<br />` :raw-html:`<br />`

            The order of arguments passed into the predicate will be:

            #. The :class:`IfTemplate` that this method is calling from
            #. The index for the part in the :class:`IfTemplate`
            #. The current part of the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        postProcessor: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], Any]]
            A function that performs any post-processing on the found part that meets the required condition :raw-html:`<br />` :raw-html:`<br />`

            The order of arguments passed into the post-processor will be:

            #. The :class:`IfTemplate` that this method is calling from
            #. The index for the part in the :class:`IfTemplate`
            #. The current part of the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will return the found :class:`IfTemplatePart` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``

        Returns
        -------
        Dict[:class:`int`, Any]
            The filtered parts that meet the search condition :raw-html:`<br />` :raw-html:`<br />`

            The keys are the index locations of the parts and the values are the found parts
        """

        result = {}
        if (pred is None):
            pred = lambda ifTemplate, partInd, part: True

        if (postProcessor is None):
            postProcessor = lambda ifTemplate, partInd, part: part

        partsLen = len(self.parts)
        for i in range(partsLen):
            part = self.parts[i]
            if (pred(self, i, part)):
                result[i] = (postProcessor(self, i, part))

        return result
    
    def getMods(self, hashRepo: Hashes, indexRepo: Indices, version: Optional[float] = None) -> Set[str]:
        """
        Retrieves the corresponding mods the :class:`IfTemplate` will fix to

        Parameters
        ----------
        hashRepo: :class:`Hashes`
            The data source for the hashes

        indexRepo: :class:`Indices`
            The data source for the indices

        version: Optional[:class:`float`]
            What version we want to fix :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, will assume we want to fix to the latest version :raw-html:`<br />` :raw-html:`<br />`
            
             **Default**: ``None``

        Returns
        -------
        Set[:class:`str`]
            Names of all the types of mods the :class:`IfTemplate` will fix to
        """

        result = set()

        for hash in self.hashes:
            replacments = hashRepo.replace(hash, version = version, errorOnNotFound = False, default = {})
            result = result.union(set(replacments.keys()))

        for index in self.indices:
            replacments = indexRepo.replace(index, version = version, errorOnNotFound = False, default = {})
            result = result.union(set(replacments.keys()))

        return result

    def _isKeyFullyCover(self, node: IfTemplateNode, key: str, sections: Dict[str, "IfTemplate"], visited: Set[str], sectionsKeyFullCover: Dict[str, bool]):
        result = node.hasKey(key)
        if (result):
            return result

        childrenResult = True
        branchChildren = node.children

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        runValues = node.getKeyValues(IniKeywords.Run.value)
        subCommandsToCheck = OrderedSet([])
        subCommandsChecked = set()

        for partValues in runValues:
            for valueData in partValues:
                subCommand = valueData[1]
                if (subCommand not in visited):
                    subCommandsToCheck.add(subCommand)
                elif (subCommand in sectionsKeyFullCover):
                    subCommandsChecked.add(subCommand)

        if (not branchChildren and not subCommandsToCheck and not subCommandsChecked):
            return result

        for subCommand in subCommandsChecked:
            childrenResult &= sectionsKeyFullCover[subCommand]
            if (not childrenResult):
                return result

        for childId in branchChildren:
            child = branchChildren[childId]
            childrenResult &= self._isKeyFullyCover(child, key, sections, visited, sectionsKeyFullCover)
            if (not childrenResult):
                return result
            
        for subCommand in subCommandsToCheck:

            # we assume the .ini file has correct syntax and does not reference some
            #   command that does not exist. It is not within this project's scope to help the
            #   person fix their own mistakes in the .ini file. Assume that an incorrect referenced
            #   command refers to some global command not in the file. So this command will be a sink in the
            #   command call graph and a leaf in the DFS tree 
            if (subCommand not in sections):
                continue

            ifTemplate = sections[subCommand]
            childrenResult &= ifTemplate.isKeyFullyCover(key, sections, visited, sectionsKeyFullCover)
            if (not childrenResult):
                return result

        result |= childrenResult
        return result
    
    def _getKeyMissingParts(self, node: IfTemplateNode, key: str, sections: Dict[str, "IfTemplate"], visited: Set[str], 
                            sectionsMissingParts: Dict[str, Set[IfContentPart]], sectionAllBranchesMissing: Dict[str, bool]) -> Tuple[Set[IfContentPart], bool]:
        nodeMissingPart, hasContentPart = node.getKeyMissingPart(key)
        if (hasContentPart and nodeMissingPart is None):
            return (set(), False)
        
        result = set() if (nodeMissingPart is None) else {nodeMissingPart}
        childrenResult = set()
        branchChildren = node.children

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        runValues = node.getKeyValues(IniKeywords.Run.value)
        subCommandsToCheck = OrderedSet([])
        subCommandsChecked = set()

        for partValues in runValues:
            for valueData in partValues:
                subCommand = valueData[1]
                if (subCommand not in visited):
                    subCommandsToCheck.add(subCommand)
                elif (subCommand in sectionsMissingParts):
                    subCommandsChecked.add(subCommand)

        if (not branchChildren and not subCommandsToCheck and not subCommandsChecked):
            return (result, True)
        
        branchChildrenLen = len(branchChildren)
        subCommandsToCheckLen = len(subCommandsToCheck)
        subCommandsCheckedLen = len(subCommandsChecked)

        missingKeyBranchChildren = 0
        missingKeySubCommandsToCheck = 0
        missingKeySubCommandsChecked = 0
        currentChildMissingKeys = set()
        currentAllBranchesMissing = False
        
        for subCommand in subCommandsChecked:
            currentChildMissingKeys = sectionsMissingParts[subCommand]
            if (subCommand not in sectionAllBranchesMissing):
                continue
            
            currentAllBranchesMissing = sectionAllBranchesMissing[subCommand]
            if (currentChildMissingKeys):
                childrenResult.update(currentChildMissingKeys)

                if (currentAllBranchesMissing):
                    missingKeyBranchChildren += 1

        for childId in branchChildren:
            child = branchChildren[childId]
            currentChildMissingKeys, currentAllBranchesMissing = self._getKeyMissingParts(child, key, sections, visited, sectionsMissingParts, sectionAllBranchesMissing)

            if (currentChildMissingKeys):
                childrenResult.update(currentChildMissingKeys)

                if (currentAllBranchesMissing):
                    missingKeyBranchChildren += 1


        for subCommand in subCommandsToCheck:

            # we assume the .ini file has correct syntax and does not reference some
            #   command that does not exist. It is not within this project's scope to help the
            #   person fix their own mistakes in the .ini file. Assume that an incorrect referenced
            #   command refers to some global command not in the file. So this command will be a sink in the
            #   command call graph and a leaf in the DFS tree 
            if (subCommand not in sections):
                continue

            ifTemplate = sections[subCommand]
            currentChildMissingKeys = ifTemplate.getKeyMissingParts(key, sections, visited, sectionsMissingParts, sectionAllBranchesMissing)

            if (currentChildMissingKeys):
                childrenResult.update(currentChildMissingKeys)

                if (currentAllBranchesMissing):
                    missingKeyBranchChildren += 1

        missingKeyChildrenTotal = missingKeyBranchChildren + missingKeySubCommandsToCheck + missingKeySubCommandsChecked
        childrenTotal = branchChildrenLen + subCommandsToCheckLen + subCommandsCheckedLen

        if (result and missingKeyChildrenTotal == childrenTotal):
            return (result, True)

        return (childrenResult, False)
    
    def isKeyFullyCover(self, key: str, sections: Dict[str, "IfTemplate"], visited: Set[str], sectionsKeyFullCover: Dict[str, bool]) -> bool:
        """
        Checks whether a key appears in all branches of the :class:`IfTemplate`

        Parameters
        ----------
        key: :class:`str`
            The key to search

        sections: Dict[:class:`str`, :class:`IfTemplate`]
            The available sections in the graph (:class:`IniSectionGraph`) where this :class:`IfTemplate` belongs to :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names for each section and the values are the corresponding :class:`IfTemplate` for each section

        visited: Set[:class:`str`]
            The names of the sections that have been visited by this method

        sectionsKeyFullCover: Dict[:class:`str`, :class:`bool`]
            The result of whether a particular section has the target key after searching of its branches (names of sections that this method has finished visiting)

        Returns
        -------
        :class:`bool`
            Whether the key appears in all conditional branches
        """

        visited.add(self.name)

        node = self.tree.root
        result = self._isKeyFullyCover(node, key, sections, visited, sectionsKeyFullCover)
        sectionsKeyFullCover[self.name] = result
        return result

    def getKeyMissingParts(self, key: str, sections: Dict[str, "IfTemplate"], visited: Set[str], sectionsMissingParts: Dict[str, Set[IfContentPart]],
                           sectionAllBranchesMissing: Dict[str, bool]) -> Set[IfContentPart]:
        """
        Finds all the :class:`IfContentPart`s that are referenced by this :class:`IfTemplate` that do not have the search 'key'

        Parameters
        ----------
        key: :class:`str`
            The key to search

        sections: Dict[:class:`str`, :class:`IfTemplate`]
            The available `sections`_ in the graph (:class:`IniSectionGraph`) where this :class:`IfTemplate` belongs to :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names for each `section`_ and the values are the corresponding :class:`IfTemplate` for each section

        visited: Set[:class:`str`]
            The names of the `sections`_ that have been visited by this method

        sectionsMissingParts: Dict[:class:`str`, :class:`bool`]
            The result of the :class:`IfContentPart` with missing keys for a particular `section`_ after searching all of its branches (names of sections that this method has finished visiting)

        sectionallBranchesMissing: Dict[:class:`str`, :class:`bool`]
            Whether all the branches within some `section`_ are missing the key to search :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names for each `section`_ and the values are whether the `section`_ has the key missing in all its branches
        """

        visited.add(self.name)

        node = self.tree.root
        result, allBranchesMissing = self._getKeyMissingParts(node, key, sections, visited, sectionsMissingParts, sectionAllBranchesMissing)
        sectionsMissingParts[self.name] = result
        sectionAllBranchesMissing[self.name] = allBranchesMissing
        return result
    
    def addTopContentPart(self) -> IfContentPart:
        """
        Add a new :class:`IfContentPart` at the root of this :class:`IfTemplate`, if needed

        Returns
        -------
        :class:`IfContentPart`
            The top part at the root of this :class:`IfTemplates`
        """

        ifContentPart = None

        if (not self.parts or not isinstance(self.parts[0], IfContentPart)):
            ifContentPart = IfContentPart({}, 0)
            self.parts.insert(0, ifContentPart)
            self.partsById[ifContentPart.id] = ifContentPart
        else:
            ifContentPart = self.parts[0]

        if (self.tree.root is None):
            self.tree.root = IfTemplateNode()

        if (not self.tree.root.parts or not isinstance(self.tree.root.parts[0], IfContentPart)):
            self.tree.root.parts.insert(0, ifContentPart)

        return ifContentPart

    def addKVPsToFront(self, kvps: List[Tuple[str, str]]):
        """
        Adds some `KVPs`_ to the top of this :class:`IfTemplate`

        Parameters
        ----------
        kvps: List[Tuple[:class:`str`, :class:`str`]]
            The `KVPs`_ to add
        """

        ifContentPart = self.addTopContentPart()
        ifContentPart.addKVPsToFront(kvps)

    def addKVPToFront(self, key: str, val: str):
        """
        Add a `KVP`_ to the top of this :class:`IfTemplate`

        Parameters
        ----------
        key: :class:`str`
            The key to add

        val: :class:`str`
            The value to add
        """

        self.addKVPsToFront([(key, val)])

    def addBottomContentPart(self) -> IfContentPart:
        """
        Add a new :class:`IfContentPart` at the very end of this :class:`IfTemplate`, if needed

        Returns
        -------
        :class:`IfContentPart`
            The bottom part at the end of this :class:`IfTemplates`
        """

        ifContentPart = None

        if (not self.parts or not isinstance(self.parts[-1], IfContentPart)):
            ifContentPart = IfContentPart({}, 0)
            self.parts.append(ifContentPart)
            self.partsById[ifContentPart.id] = ifContentPart
        else:
            ifContentPart = self.parts[-1]

        if (self.tree.root is None):
            self.tree.root = IfTemplateNode()

        if (not self.tree.root.parts or not isinstance(self.tree.root.parts[-1], IfContentPart)):
            self.tree.root.parts.append(ifContentPart)

        return ifContentPart

    def addKVPsToBack(self, kvps: List[Tuple[str, str]]):
        """
        Adds some `KVPs`_ to the bottom of this :class:`IfTemplate`

        Parameters
        ----------
        kvps: List[Tuple[:class:`str`, :class:`str`]]
            The `KVPs`_ to add
        """

        ifContentPart = self.addBottomContentPart()
        ifContentPart.addKVPs(kvps)

    def addKVPToBack(self, key: str, val: str):
        """
        Add a `KVP`_ to the bottom of this :class:`IfTemplate`

        Parameters
        ----------
        key: :class:`str`
            The key to add

        val: :class:`str`
            The value to add
        """

        self.addKVPsToBack([(key, val)])

    def toStr(self, linePrefix = "", autoindent: bool = True) -> str:
        """
        Converts this class to a string

        Parameters
        ----------
        linePrefix: :class:`str`
            The string that will prefix every line :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        autoIndent: :class:`bool`
            Whether to compute the proper tab indent for the `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`str`
            The string representation 
        """

        result = []
        tabCount = 0

        if (self.prefix):
            result.append(self.prefix)

        result.append(f"[{self.name}]")
        for part in self.parts:
            isPredPart = isinstance(part, IfPredPart)

            if (autoindent and isPredPart and part.type != IfPredPartType.If):
                tabCount -= 1

            tabs = "\t" * tabCount
            result.append(part.toStr(linePrefix = f"{linePrefix}{tabs}"))

            if (autoindent and isPredPart and part.type != IfPredPartType.EndIf):
                tabCount += 1

        if (self.suffix):
            result.append(self.suffix)

        return "\n".join(result)
##### EndScript