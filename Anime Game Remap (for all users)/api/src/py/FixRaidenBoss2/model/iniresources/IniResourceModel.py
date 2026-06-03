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


##### ExtImportss
from typing import Dict, Optional, List
##### EndExtImports

##### LocalImports
from ...tools.files.FileService import FileService
##### EndLocalImports


##### Script
class IniResourceModel():
    """
    Contains data for some particular resource in a .ini file

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    Attributes
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located
    """

    def __init__(self, iniFolderPath: str):
        self.iniFolderPath = iniFolderPath
##### EndScript