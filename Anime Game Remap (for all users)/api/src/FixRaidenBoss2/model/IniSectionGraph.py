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
from typing import Dict, Union, List, Optional, Set, Callable, Tuple
##### EndExtImports

##### LocalImports
from .iftemplate.IfTemplate import IfTemplate
from ..tools.ListTools import ListTools
from .assets.Hashes import Hashes
from .assets.Indices import Indices
##### EndLocalImports


##### Script
class IniSectionGraph():
    """
    Class for constructing a directed subgraph for how the `sections`_ in the .ini file are ran

    :raw-html:`<br />`

    .. note::
        * The nodes are the `sections`_ of the .ini file
        * The directed edges are the command calls from the `sections`_ , where the source of the edge is the caller and the sink of the edge is the callee

    Parameters
    ----------
    sections: Set[:class:`str`]
        Names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini file

    allSections: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ for the .ini file

        :raw-html:`<br />`

        .. note::
            You can think of this as the `adjacency list`_ for the directed graph of all `sections`_ in the .ini file

    remapNameFunc: Optional[Callable[[:class:`str`, :class:`str`], :class:`str`]]
        Function to get the corresponding remap names for the section names :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will not get the remap names for the sections :raw-html:`<br />` :raw-html:`<br />`

        The parameters for the function are:

            #. Name of the `section`_
            #. Name fo the type of mod to fix
        
        **Default**: ``False``

    modsToFix: Optional[Set[:class:`str`]]
        The names of the mods that will be fixed by the .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    remapNameFunc: Optional[Callable[[:class:`str`, :class:`str`], :class:`str`]]
        Function to get the corresponding remap names for the section names :raw-html:`<br />` :raw-html:`<br />`

        The parameters for the function are:

            #. Name of the `section`_
            #. Name fo the type of mod to fix
    """

    def __init__(self, targetSections: Union[Set[str], List[str]], allSections: Dict[str, IfTemplate], 
                 remapNameFunc: Optional[Callable[[str, str], str]] = None, modsToFix: Optional[Set[str]] = None):
        self._modsToFix = modsToFix
        if (modsToFix is None):
            self._modsToFix = {}
        
        self._setTargetSections(targetSections)
        self._sections: Dict[str, IfTemplate] = {}
        self._allSections = allSections
        self._remapNames: Dict[str, Dict[str, str]] = {}
        self._runSequence: List[Tuple[str, IfTemplate]] = []
        self.remapNameFunc = remapNameFunc

        self.build()

    @property
    def targetSections(self):
        """
        Names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini file

        :getter: The names of the desired `sections`_ we want in the subgraph
        :setter: Constructs a new subgraph based on the new desired `sections`_ we want
        :type: List[:class:`str`]
        """

        return self._targetSections
    
    def _setTargetSections(self, newTargetSections: Union[Set[str], List[str]]):
        self._targetSections = ListTools.getDistinct(newTargetSections, keepOrder = True)
    
    @targetSections.setter
    def targetSections(self, newTargetSections: Union[Set[str], List[str]]):
        self._setTargetSections(newTargetSections)
        self.build()

    @property
    def sections(self):
        """
        The `sections`_ that are part of the contructed subgraph based on the desired sections specified at :attr:`IniSectionGraph.targetSections`

        :raw-html:`<br />`

        .. note::
            You can think of this as the `adjacency list`_ for the subgraph

        :getter: All the `sections`_ for the subgraph
        :type: Dict[:class:`str`, :class:`IfTemplate`]
        """

        return self._sections
    
    @property
    def allSections(self):
        """
        All the `sections`_ of the .ini file

        :raw-html:`<br />`

        .. note::
            You can think of this as the `adjacency list`_ for the directed graph of all `sections`_ in the .ini file

        :getter: All the `sections`_ for the .ini file
        :setter: Constructs a new subgraph based on the new `sections`_ for the .ini file
        :type: Dict[:class:`str`, :class:`IfTemplate`]
        """

        return self._allSections
    
    @allSections.setter
    def allSections(self, newAllSections: Dict[str, IfTemplate]):
        self._allSections = newAllSections
        self.build()

    @property
    def remapNames(self):
        """
        The corresponding names for the `sections`_ that the fix will make :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the name of the original `section`_
        * The inner key is the name for the type of mod to fix
        * The inner value is the corresponding name for the `section`_ and mod type

        :getter: All the corresponding names for the `sections`_
        :type: Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]
        """

        return self._remapNames
    
    @property
    def runSequence(self):
        """
        The order the `sections`_ will be ran

        :getter: Retrieves the order the `sections`_ will be ran
        :type: List[Tuple[:class:`str`, :class:`IfTemplate`]]
        """

        return self._runSequence
    
    @property
    def modsToFix(self):
        """
        The names of the mods that will be fixed by the .ini file

        :getter: Retrieves the names of the mods to fix
        :type: Set[:class:`str`]
        """

        return self._modsToFix

    def build(self, newTargetSections: Optional[Union[Set[str], List[str]]] = None, newAllSections: Optional[Dict[str, IfTemplate]] = None,
              newModsToFix: Optional[Set[str]] = None):
        """
        Performs the initialization for rebuilding the subgraph

        Parameters
        ----------
        newTargetSections: Optional[Set[:class:`str`], List[:class:`str`]]
            The new desired `sections`_ we want in our subgraph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newAllSections: Optional[Dict[:class:`str`, :class:`IfTemplate`]]
            The new `sections`_ for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newModsToFix: Optional[Set[:class:`str`]]
            The new desired names of the mods that we want to fix for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (newTargetSections is not None):
            self._setTargetSections(newTargetSections)

        if (newAllSections is not None):
            self._allSections = newAllSections

        self.construct()
        if (self.remapNameFunc is not None):
            self.getRemapBlendNames(newModsToFix = newModsToFix)
        else:
            self._remapNames = {}


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
        try:
            ifTemplate = self._allSections[sectionName]
        except Exception as e:
            if (raiseException):
                raise KeyError(f"The section by the name '{sectionName}' does not exist") from e
            else:
                return None
        else:
            return ifTemplate

    def _dfsExplore(self, section: IfTemplate, visited: Dict[str, IfTemplate], runSequence: List[Tuple[str, IfTemplate]]):
        """
        The typical recursive implementation of `DFS`_ for exploring a particular `section`_ (node)

        Parameters
        ----------
        section: :class:`IfTemplate`
            The `section`_ that is currently being explored
        
        visited: Dict[:class:`str`, :class:`ifTemplate`]
            The `sections`_ that have already been visited

        runSequence: List[Tuple[:class:`str`, :class:`IfTemplate`]]
            The order the `sections`_ will be ran
        """

        calledSubCommands = section.calledSubCommands
        for partInd in calledSubCommands:
            subSections = calledSubCommands[partInd]

            for subSectionData in subSections:
                subSection = subSectionData[1]
                if (subSection not in visited):

                    # we assume the .ini file has correct syntax and does not reference some
                    #   command that does not exist. It is not within this project's scope to help the
                    #   person fix their own mistakes in the .ini file. Assume that an incorrect referenced
                    #   command refers to some global command not in the file. So this command will be a sink in the
                    #   command call graph and a leaf in the DFS tree 
                    neighbourSection = self.getSection(subSection, raiseException = False)
                    if (neighbourSection is None):
                        continue

                    visited[subSection] = neighbourSection
                    
                    runSequence.append((subSection, neighbourSection))
                    self._dfsExplore(neighbourSection, visited, runSequence)

    def construct(self) -> Dict[str, IfTemplate]:
        """
        Constructs the subgraph for the `sections`_ using `DFS`_

        Returns
        -------
        Dict[:class:`str`, :class:`IfTemplate`]
            The `sections` that are part of the subgraph
        """

        visited = {}
        runSequence = []
        sections = {}

        for sectionName in self._targetSections:
            ifTemplate = self.getSection(sectionName)
            sections[sectionName] = ifTemplate

        # perform the main DFS algorithm
        for sectionName in sections:
            section = sections[sectionName]

            if (sectionName not in visited):
                visited[sectionName] = section
                runSequence.append((sectionName, section))
                self._dfsExplore(section, visited, runSequence)

        self._sections = visited
        self._runSequence = runSequence
        return self._sections

    def getRemapBlendNames(self, newModsToFix: Optional[Set[str]] = None) -> Dict[str, Dict[str, str]]:
        """
        Retrieves the corresponding remap names of the sections made by this fix

        Parameters
        ----------
        newModsToFix: Optional[Set[:class:`str`]]
            The new desired names of the mods that we want to fix for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[:class:`str`, :class:`str`]
            The new names for the `sections`_ with the 'FixRemap' keyword added
        """

        result = {}
        if (newModsToFix is not None):
            self._modsToFix = newModsToFix

        if (not self._modsToFix):
            self._remapNames = result
            return result

        for sectionName in self._sections:
            for modName in self._modsToFix:
                try:
                    result[sectionName]
                except KeyError:
                    result[sectionName] = {}

                result[sectionName][modName] = self.remapNameFunc(sectionName, modName)

        self._remapNames = result
        return result
    
    def getCommonMods(self, hashRepo: Hashes, indexRepo: Indices, version: Optional[float] = None) -> Set[str]:
        """
        Retrieves the common mods to fix to based off all the :class:`IfTemplate`s in the graph

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

        for sectionName in self._sections:
            ifTemplate = self._sections[sectionName]
            ifTemplateMods = ifTemplate.getMods(hashRepo, indexRepo, version = version)

            if (not result):
                result = ifTemplateMods
            elif (ifTemplateMods):
                result = result.intersection(ifTemplateMods)

        return result
##### EndScript