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
from typing import TYPE_CHECKING
##### EndExtImports

##### LocalImports
from ..BaseIniGraphPartEdit import BaseIniGraphPartEdit
from ....IniSectionGraph import IniSectionGraph

if (TYPE_CHECKING):
    from ....files.IniFile import IniFile
    from ...ModType import ModType
##### EndLocalImports


##### Script
class BaseIniGraphEdit(BaseIniGraphPartEdit):
    """
    This class inherits from :class:`BaseIniGraphPartEdit`

    Base class for a filter that edits some caller/callee graph of :class:`IniSectionGraph`
    """

    def editFromIni(self, graph: IniSectionGraph, ini: "IniFile", modType: "ModType", modName: str = "") -> IniSectionGraph:
        """
        Edits the caller/callee graph of :class:`IniSectionGraph` with state info from 'ini'

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit

        ini: :class:`IniFile`
            The associated .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        :class:`IniSectionGraph`
            The resultant graph that got editted
        """

        return self.edit(graph, modType, modName = modName)

    def edit(self, graph: IniSectionGraph, modType: "ModType", modName: str = "") -> IniSectionGraph:
        """
        Edits the caller/callee graph of :class:`IniSectionGraph`

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        :class:`IniSectionGraph`
            The resultant graph that got editted
        """

        pass
##### EndScript