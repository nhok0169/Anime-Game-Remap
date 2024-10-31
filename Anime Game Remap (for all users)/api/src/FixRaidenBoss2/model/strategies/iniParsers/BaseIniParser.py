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
from typing import TYPE_CHECKING, Set
##### EndExtImports

##### LocalImports
if (TYPE_CHECKING):
    from ...IniFile import IniFile
##### EndLocalImports


##### Script
class BaseIniParser():
    """
    Base class to parse a .ini file

    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to parse

    Attributes
    ----------
    _modsToFix: Set[:class:`str`]
        The name of the mods that will be fixed to

    _iniFile: :class:`IniFile`
        The .ini file that will be parsed
    """

    def __init__(self, iniFile: "IniFile"):
        self._modsToFix: Set[str] = set()
        self._iniFile = iniFile

    def clear(self):
        """
        Clears any saved data
        """
        self._modsToFix.clear()

    def parse(self):
        """
        Parses the .ini file
        """
        pass
##### EndScript