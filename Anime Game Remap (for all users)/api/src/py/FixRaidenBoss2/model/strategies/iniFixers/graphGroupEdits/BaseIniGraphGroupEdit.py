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
from typing import List, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from ..BaseIniPartEdit import BaseIniPartEdit
from ....IniGraphGroup import IniGraphGroup

if (TYPE_CHECKING):
    from ....files.IniFile import IniFile
    from ...ModType import ModType
##### EndLocalImports


##### Script
class BaseIniGraphGroupEdit(BaseIniPartEdit):
    """
    This class inherits from :class:`BaseIniPartEdit`

    Base class for a filter that edits a group of caller/callee graphs across many .ini files
    """

    def editFromIni(self, graphGroups: List[IniGraphGroup], ini: "IniFile", modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        """
        Edits a group of caller/callee graphs with state info from 'ini'

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        ini: :class:`IniFile`
            The associated original .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        List[:class:`IniGraphGroup`]
            The resultant group of graphs that got editted
        """

        return self.edit(graphGroups, modType, modName = modName)

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        """
        Edits a group of caller/callee graphs

        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        List[:class:`IniGraphGroup`]
            The resultant group of graphs that got editted
        """

        pass
##### EndScript