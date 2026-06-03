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
from typing import Dict, Tuple, List, TYPE_CHECKING, Union, Callable
##### EndExtImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from ....IniGraphGroup import IniGraphGroup
from ....IniNamingTools import IniNamingTools

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

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        return self.edit(graphGroups, modType, modName = modName)

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        graphGroupLen = len(graphGroups)

        for i in range(graphGroupLen):
            graphGroups[i] = [graphGroups[i]]

        graphGroups.append([])

        for i in range(graphGroupLen):
            fromIniGraphGroups = graphGroups[i]

            fromIniGraphGroup = fromIniGraphGroups[0]
            fromIniGraphs = fromIniGraphGroup.graphs
            modObjs = list(fromIniGraphs.keys())

            for component, obj in modObjs:
                iniModObj = (i, component, obj)
                modObj = (component, obj)
                fromGraph = fromIniGraphs[(component, obj)]

                remapppedObjs = self.remap.get(iniModObj)
                if (remapppedObjs is None):
                    continue

                fromIniGraphGroup.graphs.pop(modObj)

                for remappedObjData in remapppedObjs:
                    renameFunc = None
                    
                    if (len(remappedObjData) == 3):
                        iniInd, remapComponent, remapObj = remappedObjData
                    else:
                        iniInd, remapComponent, remapObj, renameFunc = remappedObjData

                    remappedObj = (remapComponent, remapObj)

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

                    toGraph = fromGraph.deepcopy()
                    if (renameFunc is not None):
                        toGraph.rename(renameFunc)
                    else:
                        toGraph.rename(lambda oldSectionName: IniNamingTools.getObjRemapFixName(oldSectionName, modName, modObj, remappedObj))

                    toIniGraphGroup.addGraph(remappedObj, toGraph)

                if (not fromIniGraphGroup.graphs):
                    fromIniGraphGroups.pop(0)

        graphGroups = list(IT.chain(*graphGroups))
        return graphGroups
##### EndScript