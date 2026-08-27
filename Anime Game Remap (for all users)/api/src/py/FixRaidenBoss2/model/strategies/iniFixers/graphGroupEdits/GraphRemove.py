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
from typing import List, TYPE_CHECKING, Tuple
##### EndExtImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from .....core import IniGraphGroup

if (TYPE_CHECKING):
    from ...ModType import ModType
##### EndLocalImports


##### Script
class GraphRemove(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Removes some graphs from a group of graphs

    Parameters
    ----------
    graphIds: List[Tuple[:class:`int`, :class:`str`, :class:`str`]]
        The ids of the graphs to remove. Each tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    Attributes
    ----------
    graphIds: List[Tuple[:class:`int`, :class:`str`, :class:`str`]]
        The ids of the graphs to remove
    """

    def __init__(self, graphIds: List[Tuple[int, str, str]]):
        self.graphIds = graphIds

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        graphGroupsLen = len(graphGroups)

        for graphId in self.graphIds:
            iniInd, comp, obj = graphId
            if (iniInd >= graphGroupsLen):
                continue

            graphGroups[iniInd].removeGraph((comp, obj))

        return graphGroups
##### EndScript