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
from typing import TYPE_CHECKING, Any
##### EndExtImports

##### LocalImports
if (TYPE_CHECKING):
    from ...files.IniFile import IniFile
    from ..ModType import ModType
##### EndLocalImports

##### Script
class BaseIniPartEdit():
    """
    Base class for a filter that edits some part of the .ini file
    """

    pass

    def clear(self):
        """
        Clears any saved state information
        """

        pass

    def editFromIni(self, ini: "IniFile", *args, modType: "ModType", modName: str = "", **kwargs) -> Any:
        """
        Edits some part in a .ini file with state info from 'ini'

        Parameters
        ----------
        ini: :class:`IniFile`
            The associated .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        Any
            The corresponding editted part
        """

        return self.edit(*args, **kwargs)

    def edit(self, *args, modType: "ModType", modName: str = "", **kwargs) -> Any:
        """
        Edits some part in a .ini file

        Parameters
        ----------
        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns 
        -------
        Any
            The corresponding editted part
        """

        pass
##### EndScript