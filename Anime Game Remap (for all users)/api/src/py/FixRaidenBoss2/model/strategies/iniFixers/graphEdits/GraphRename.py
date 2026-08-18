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
from typing import TYPE_CHECKING, Optional, Callable
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
##### EndCppLocalImports

##### LocalImports
from .BaseIniGraphEdit import BaseIniGraphEdit
from ....IniSectionGraph import IniSectionGraph
from ....SectionIterData import SectionIterData

if (TYPE_CHECKING):
    from ....files.IniFile import IniFile
    from ...ModType import ModType
##### EndLocalImports


##### Script
class GraphRename(BaseIniGraphEdit):
    """
    This class inherits from :class:`BaseIniGraphEdit`

    Renames the `sections`_ of some caller/callee graph of :class:`IniSectionGraph`

    Parameters
    ----------
    renameFunc: Callable[[:class:`str`], :class:`str`]
        Function used to rename a `section`_. The function takes in the name of the old `section`_ and returns
        the new name for the `section`_

    Attributes
    ----------
    renameFunc: Callable[[:class:`str`], :class:`str`]
        Function used to rename a `section`_. The function takes in the name of the old `section`_ and returns
        the new name for the `section`_
    """

    def __init__(self, renameFunc: Callable[[str], str]):
        self.renameFunc = renameFunc

    def edit(self, graph: IniSectionGraph, modType: "ModType", modName: str = "", partFilter: Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]] = None) -> IniSectionGraph:
        graph.rename(self.renameFunc)
        return graph
##### EndScript