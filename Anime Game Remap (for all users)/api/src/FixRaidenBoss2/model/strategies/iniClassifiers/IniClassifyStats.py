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
from typing import Optional, Optional, TYPE_CHECKING
##### EndExtImports

##### LocalImports
if (TYPE_CHECKING):
    from ..ModType import ModType
##### EndLocalImports


##### Script
class IniClassifyStats():
    """
    A class that stores the statistics about the classification result of a .ini file

    Parameters
    ----------
    modType: Optional[:class:`ModType`]
        The type of mod found

    isMod: :class:`bool`
        Whether the .ini file belongs to a mod

    isFixed: :class:`bool`
        Whether the .ini file is fixed

    Attributes
    ----------
    modType: Optional[:class:`ModType`]
        The type of mod found

    isMod: :class:`bool`
        Whether the .ini file belongs to a mod

    isFixed: :class:`bool`
        Whether the .ini file is fixed
    """

    def __init__(self, modType: Optional["ModType"] = None, isMod: bool = False, isFixed: bool = False):
        self.modType = modType
        self.isMod = isMod
        self.isFixed = isFixed
##### EndScript