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
from typing import List, Dict, Tuple, Union, Type, Optional, TYPE_CHECKING, Set, Callable
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
##### EndCppLocalImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from ..graphEdits.BaseIniGraphEdit import BaseIniGraphEdit
from ..regEdits.BaseRegEdit import BaseRegEdit
from ....IniSectionGraph import IniSectionGraph
from ....IniGraphGroup import IniGraphGroup
from ....SectionIterData import SectionIterData

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class GraphGroupEdit(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Edits the individual :class:`IniSectionGraph` from a group of graphs

    Parameters
    ----------
    edits: List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]
        The specific edits to make on the individual graphs :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the individual edits for the corresponding graph

    trackKeys: Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]
        Whether to track the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the values of the flag

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    keysToTrack: Optional[List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]]
        Specific keys to track in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values are the keys to track for each graph. If the value is ``None``, then will keep track of all the keys encountered in some :class:`IfContentPart` for that graph

        :raw-html:`<br />`

        **Default**: ``None``

    keyFilters: Optional[List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]], Callable[[:class:`SectionIterData`], :class:`bool`]]]]]
        Functions for any :class:`BaseRegEdit` to only process on specific parts of a `section`_ :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the predicates for each .ini file
        * The keys are the name of the component and the name of the mod object
        * The values are functions that retrieve the ranges of valid order indices to process for some :class:`IfContentPart`

        :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    edits: List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]
        The specific edits to make on the individual graphs :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the individual edits for the corresponding graph

    trackKeys: Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]
        Whether to track the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the values of the flag

    keysToTrack: List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Whether to track the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more granular flag setting can be made.
        The structure of the granular version of the data is as follows: :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the values of the flag

    keyFilters: List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]], Callable[[:class:`SectionIterData`], :class:`bool`]]]]
        Functions for any :class:`BaseRegEdit` to only process on specific parts of a `section`_ :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the predicates for each .ini file
        * The keys are the name of the component and the name of the mod object
        * The values are functions that retrieve the ranges of valid order indices to process for some :class:`IfContentPart`
    """

    def __init__(self, edits: List[Dict[Tuple[str, str], List[Union[BaseIniGraphEdit, BaseRegEdit]]]], trackKeys: Union[bool, List[Dict[Tuple[str, str], bool]]] = False, 
                 keysToTrack: Optional[List[Dict[Tuple[str, str], Optional[Set[str]]]]] = None,
                 keyFilters: Optional[List[Dict[Tuple[str, str], Union[List[Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]]], Callable[[SectionIterData], bool]]]]] = None):
        self.edits = edits
        self.trackKeys = trackKeys
        self.keysToTrack = [] if (keysToTrack is None) else keysToTrack
        self.keyFilters = [] if (keyFilters is None) else keyFilters

    @classmethod
    def _getPartEditType(cls, filter: Union[BaseIniGraphEdit, BaseRegEdit]) -> Optional[Union[BaseIniGraphEdit, BaseRegEdit]]:
        result = None
        if (isinstance(filter, BaseIniGraphEdit)):
            result = BaseIniGraphEdit
        elif (isinstance(filter, BaseRegEdit)):
            result = BaseRegEdit

        return result
    
    @classmethod
    def _filterGroupEdit(cls, graph: IniSectionGraph, filterType: Union[Type[BaseIniGraphEdit], Type[BaseRegEdit]], 
                         filterGroup: Union[List[BaseIniGraphEdit], List[BaseRegEdit]], modType: "ModType", modName: str = "",
                         ini: Optional["IniFile"] = None, 
                         trackKeys: bool = False, keysToTrack: Optional[Set[str]] = None,
                         keyFilters: Optional[List[Optional[Callable[[SectionIterData], Ranges]]]] = None) -> IniSectionGraph:
        partEdit = None

        if (filterType == BaseIniGraphEdit):
            if (ini is not None):
                partEdit = lambda filter, part: filter.editFromIni(part, ini, modType, modName = modName)
            else:
                partEdit = lambda filter, part: filter.edit(part, modType, modName = modName)

            for graphFilter in filterGroup:
                graph = partEdit(graphFilter, graph)

        elif (filterType == BaseRegEdit):
            if (ini is not None):
                partEdit = lambda filter, part, sectionName: filter.editFromIni(part, sectionName, ini, modType, modName = modName)
            else:
                partEdit = lambda filter, part, sectionName: filter.edit(part, sectionName, modType, modName = modName)

            if (keyFilters is None):
                keyFilters = []

            filterGroupLen = len(filterGroup)
            keyFiltersLen = len(keyFilters)
            defaultFilter = lambda iterData, modType, ini: Ranges.createFull()

            for iterData in graph.iterByContentPart(colour = trackKeys, colourKeys = keysToTrack):
                parts = iterData.section.parts
                part = iterData.part
                sectionName = iterData.sectionName
                colouring = iterData.colouring
                partChanged = False
                
                for i in range(filterGroupLen):
                    regFilter = filterGroup[i]
                    keyFilter = defaultFilter if (i >= keyFiltersLen) else keyFilters[i]
                    keyRanges = keyFilter(iterData, modType, ini)

                    if (not keyRanges.isEmpty()):
                        part = partEdit(regFilter, part, sectionName)
                        if (colouring is not None):
                            colouring.updateColouring(part, targetKeys = keysToTrack, updatePreviousKVPs = False)

                        partChanged = True

                if (partChanged):
                    parts[i] = part

        return graph

    @classmethod
    def editSectionGraph(cls, graph: IniSectionGraph, filters: List[Union[BaseIniGraphEdit, BaseRegEdit]], modType: "ModType", modName: str = "", 
                         ini: Optional["IniFile"] = None, keyFilters: Optional[List[Optional[Callable[[SectionIterData], bool]]]] = None,
                         trackKeys: bool = False, keysToTrack: Optional[Set[str]] = None) -> IniSectionGraph:
        """
        Edits a caller/callee graph

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit

        filter: List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]
            The filters to edit the graph

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        ini: Optional[:class:`IniFile`]
            The associated .ini file, if available :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        trackkeys: :class:`bool`
            Whether to track the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` in the argument 'filter' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        keyFilters: Optional[List[Optional[Callable[[:class:`SectionIterData`], :class:`bool`]]]] 
            The predicates for any :class:`BaseRegEdit` to only process on specific parts of a `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`IniSectionGraph`
            The editted graph
        """

        filterGroup = []
        filterType = None

        if (filters):
            filterType = cls._getPartEditType(filters[0])

        for filter in filters:
            currentFilterType = cls._getPartEditType(filter)

            if (filterType == currentFilterType):
                filterGroup.append(filter)
                continue

            graph = cls._filterGroupEdit(graph, filterType, filterGroup, modType, modName = modName, ini = ini, trackKeys = trackKeys, keyFilters = keyFilters, keysToTrack = keysToTrack)

            filterType = currentFilterType
            filterGroup.clear()
            filterGroup.append(filter)

        if (filterGroup):
            graph = cls._filterGroupEdit(graph, filterType, filterGroup, modType, modName = modName, ini = ini, trackKeys = trackKeys, keyFilters = keyFilters, keysToTrack = keysToTrack)

        return graph
    
    def _edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "", ini: Optional["IniFile"] = None) -> List[IniGraphGroup]:
        minIniFileLen = min(len(graphGroups), len(self.edits))
        keyFiltersLen = len(self.keyFilters)
        keysToTrackLen = len(self.keysToTrack)

        isGlobalTrackKeys = isinstance(self.trackKeys, bool)
        trackKeysLen = 0 if (isGlobalTrackKeys) else len(self.trackKeys)

        for i in range(minIniFileLen):
            iniEdits = self.edits[i]
            iniGraphs = graphGroups[i].graphs
            iniKeyFilters = {} if (i >= keyFiltersLen) else self.keyFilters[i]
            iniKeysToTrack = {} if (i >= keysToTrackLen) else self.keysToTrack[i]

            iniTrackKeys = self.trackKeys
            if (not isGlobalTrackKeys and i >= trackKeysLen):
                iniTrackKeys = {}
            elif (not isGlobalTrackKeys):
                iniTrackKeys = self.trackKeys[i]

            for modObj in iniGraphs:
                if (modObj not in iniEdits):
                    continue

                iniGraph = iniGraphs[modObj]
                objIniEdits = iniEdits[modObj]
                objKeyFilters = iniKeyFilters.get(modObj)
                objKeysToTrack = iniKeysToTrack.get(modObj)
                objTrackKeys = iniTrackKeys if (isGlobalTrackKeys) else iniTrackKeys.get(modObj, False)

                if (callable(objKeyFilters)):
                    newObjKeyFilters = []
                    for edit in objIniEdits:
                        newObjKeyFilters.append(objKeyFilters)
                else:
                    newObjKeyFilters = objKeyFilters

                iniGraphs[modObj] = self.editSectionGraph(iniGraph, objIniEdits, modType, modName = modName, ini = ini, 
                                                          keyFilters = newObjKeyFilters, trackKeys = objTrackKeys,
                                                          keysToTrack = objKeysToTrack)

        return graphGroups

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self._edit(graphGroups, modType, modName = modName, ini = ini)

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self._edit(graphGroups, modType, modName = modName)
##### EndScript