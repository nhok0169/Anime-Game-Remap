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
from typing import List, Dict, Tuple, Union, Type, Optional, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from ..graphEdits.BaseIniGraphEdit import BaseIniGraphEdit
from ..regEdits.BaseRegEdit import BaseRegEdit
from ....IniSectionGraph import IniSectionGraph
from ....IniGraphGroup import IniGraphGroup
from ....iftemplate.IfContentPart import IfContentPart

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

    Attributes
    ----------
    edits: List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]
        The specific edits to make on the individual graphs :raw-html:`<br />` :raw-html:`<br />`

        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the individual edits for the corresponding graph
    """

    def __init__(self, edits: List[Dict[Tuple[str, str], List[Union[BaseIniGraphEdit, BaseRegEdit]]]]):
        self.edits = edits

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
                         ini: Optional["IniFile"] = None) -> IniSectionGraph:
        partEdit = None
        if (ini is not None):
            partEdit = lambda filter, part: filter.editFromIni(part, ini, modType, modName = modName)
        else:
            partEdit = lambda filter, part: filter.edit(part, modType, modName = modName) 

        if (filterType == BaseIniGraphEdit):
            for graphFilter in filterGroup:
                graph = partEdit(graphFilter, graph)

        elif (filterType == BaseRegEdit):
            for sectionName in graph.sections:
                section = graph.sections[sectionName]
                parts = section.parts
                partsLen = len(parts)

                for i in range(partsLen):
                    part = parts[i]
                    if (not isinstance(part, IfContentPart)):
                        continue

                    for regFilter in filterGroup:
                        part = partEdit(regFilter, part)

                    parts[i] = part

        return graph

    @classmethod
    def editSectionGraph(cls, graph: IniSectionGraph, filters: List[Union[BaseIniGraphEdit, BaseRegEdit]], modType: "ModType", modName: str = "", ini: Optional["IniFile"] = None) -> IniSectionGraph:
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

            graph = cls._filterGroupEdit(graph, filterType, filterGroup, modType, modName = modName, ini = ini)

            filterType = currentFilterType
            filterGroup.clear()
            filterGroup.append(filter)

        if (filterGroup):
            graph = cls._filterGroupEdit(graph, filterType, filterGroup, modType, modName = modName, ini = ini)

        return graph
    
    def _edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "", ini: Optional["IniFile"] = None) -> List[IniGraphGroup]:
        minIniFileLen = min(len(graphGroups), len(self.edits))

        for i in range(minIniFileLen):
            iniEdits = self.edits[i]
            iniGraphs = graphGroups[i].graphs

            for modObj in iniGraphs:
                if (modObj not in iniEdits):
                    continue

                iniGraph = iniGraphs[modObj]
                iniEdits = iniEdits[modObj]
                iniGraphs[modObj] = self.editSectionGraph(iniGraph, iniEdits, modType, modName = modName, ini = ini)

        return graphGroups

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self._edit(graphGroups, modType, modName = modName, ini = ini)

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self._edit(graphGroups, modType, modName = modName)
##### EndScript