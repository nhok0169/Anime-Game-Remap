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
import itertools as IT
from typing import Dict, Tuple, List, TYPE_CHECKING, Union, Callable, Optional
##### EndExtImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from ....IniGraphGroup import IniGraphGroup
from ....IniNamingTools import IniNamingTools
from ....IniSectionGraph import IniSectionGraph

if (TYPE_CHECKING):
    from ....files.IniFile import IniFile
    from ...ModType import ModType
##### EndLocalImports


##### Script
class GraphGroupRemap(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Remaps the graphs from a group of graphs

    Parameters
    ----------
    remap: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[Tuple[:class:`int`, :class:`str`, :class:`str`]]]
        The remap for the graphs :raw-html:`<br />` :raw-html:`<br />`

        * The keys of the dictionary are the mod objects to remap from.
        * The values of the dictionary are the mod objects to remap to.
        * The tuples include:

            #. The index of the .ini file for the graph
            #. The name of the component for the graph
            #. The name of the mod object for the graph

    Attributes
    ----------
    remap: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]
        The remap for the graphs :raw-html:`<br />` :raw-html:`<br />`

        * The keys of the dictionary are the mod objects to remap from.
        * The values of the dictionary are the mod objects to remap to.
        * The tuples include:

            #. The index of the .ini file for the graph
            #. The name of the component for the graph
            #. The name of the mod object for the graph
            #. An optional rename function if the tuple has 4 values. The rename function takes in the old name of the `section`_
    """

    def __init__(self, remap: Dict[Tuple[int, str, str], List[Union[Tuple[int, str, str], Tuple[int, str, str, Callable[[str], str]]]]]):
        self.remap = remap

    @classmethod
    def _getGraphGroup(cls, graphGroups: List[List[IniGraphGroup]], id: Tuple[int, str, str]) -> Optional[IniGraphGroup]:
        iniInd, comp, obj = id
        if (iniInd >= len(graphGroups)):
            return None
        
        graphGroup = graphGroups[iniInd]
        graphGroup = graphGroup[0]
        return graphGroup

    def remapGraphs(self, graphGroups: List[IniGraphGroup], createToGraph: Callable[[IniSectionGraph, Tuple[int, str, str], Tuple[int, str, str], Optional[Callable[[str], str]]], IniSectionGraph]) -> List[IniGraphGroup]:
        """
        Remaps the graphs from a group of graphs

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to remap

        createToGraph: Callable[[:class:`IniSectionGraph`, Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Callable[[:class:`str`], :class:`str`]]], :class:`IniSectionGraph`]
            The function used to create the new remapped graph :raw-html:`<br />` :raw-html:`<br />`

            The function takes in the following parameters:

            #. The graph to map from
            #. The id of the graph to map from. The tuple contains the index of the .ini file for the graph, the name of the component and the name of the mod object
            #. The id of the graph to map to. The tuple contains the index of the .ini file for the graph, the name of the component and the name of the mod object. Note that index of the .ini file may not correspond to the actual index of which .ini file holds the graph
            #. An optional rename function

        Returns
        -------
        List[:class:`IniSectionGraph`]
            group of graphs that got remapped
        """

        graphGroupLen = len(graphGroups)

        for i in range(graphGroupLen):
            graphGroups[i] = [graphGroups[i]]

        graphGroups.append([])

        # retreives the graphs to remove
        removedGraphs = {}
        for srcModObj in self.remap:
            graphGroup = self._getGraphGroup(graphGroups, srcModObj)
            if (graphGroup is None):
                continue
            
            iniInd, comp, obj = srcModObj
            graph = graphGroup.removeGraph((comp, obj))
            if (graph is not None):
                removedGraphs[srcModObj] = (graph, graphGroup)

        # remap the graphs
        for srcModObj in removedGraphs:
            fromGraph, _ = removedGraphs[srcModObj]
            remapppedObjs = self.remap[srcModObj]

            for remappedObjData in remapppedObjs:
                renameFunc = None
                
                if (len(remappedObjData) == 3):
                    iniInd, remapComponent, remapObj = remappedObjData
                else:
                    iniInd, remapComponent, remapObj, renameFunc = remappedObjData

                remappedObj = remappedObjData[1:3]

                if (iniInd > graphGroupLen):
                    iniInd = graphGroupLen

                toIniGraphGroups = graphGroups[iniInd]
                toIniGraphGroupInd = 0

                if (not toIniGraphGroups):
                    toIniGraphGroup = IniGraphGroup()
                    toIniGraphGroups.append(toIniGraphGroup)

                toIniGraphGroup = toIniGraphGroups[toIniGraphGroupInd]
                toIniGraphGroupsLen = len(toIniGraphGroups)

                while (remappedObj in toIniGraphGroup.graphs):
                    if (toIniGraphGroupInd >= toIniGraphGroupsLen - 1):
                        toIniGraphGroups.append(IniGraphGroup())

                    toIniGraphGroupInd += 1
                    toIniGraphGroup = toIniGraphGroups[toIniGraphGroupInd]

                toGraph = createToGraph(fromGraph, srcModObj, remappedObjData[:3], renameFunc)
                toIniGraphGroup.addGraph(remappedObj, toGraph)

        # remove any empty graph groups
        for srcModObj in removedGraphs:
            iniInd, comp, obj = srcModObj
            fromIniGraphGroups = graphGroups[iniInd]
            fromIniGraphGroup = fromIniGraphGroups[0]

            if (not fromIniGraphGroup.graphs):
                fromIniGraphGroups.pop(0)

        graphGroups = list(IT.chain(*graphGroups))
        return graphGroups
    
    @classmethod
    def copyGraph(cls, fromGraph: IniSectionGraph, modObj: Tuple[int, str, str], newModObj: Tuple[int, str, str], renameFunc: Optional[Callable[[str], str]] = None, modName: str = "") -> IniSectionGraph:
        result = fromGraph.deepcopy()

        if (renameFunc is not None):
            result.rename(renameFunc)
        else:
            result.rename(lambda oldSectionName: IniNamingTools.getObjRemapFixName(oldSectionName, modName, modObj[1:], newModObj[1:]))

        return result

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self.edit(graphGroups, modType, modName = modName)

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self.remapGraphs(graphGroups, lambda fromGraph, modObj, remappedObj, renameFunc: self.copyGraph(fromGraph, modObj, remappedObj, renameFunc = renameFunc, modName = modName))
##### EndScript