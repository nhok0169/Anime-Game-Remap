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
from enum import Enum
##### EndExtImports

##### LocalImports
from ..tools.tries.AhoCorasickBuilder import AhoCorasickBuilder
from ..tools.tries.AhoCorasickSingleton import AhoCorasickSingleton
##### EndLocalImports


##### Script
class GlobalClassifiers(Enum):
    """
    Global modules used by the sofware to help classify strings into different sets

    Attributes
    ----------
    ModTypes: :class:`AhoCorasickSingleton`
        The classifier used to identify the :class:`ModType` for some string

    ModOptFiles: :class:`AhoCorasickSingleton`
        The classifier used to identify the type of file within a mod

    DownloadModes: :class:`AhoCorasickSingleton`
        The classifier used to identify the :class:`DownloadMode` for some string

    IniModelParts: :class:`AhoCorasickSingleton`
        The classfier for the different parts of the model of a mod, according to most .ini files
    """

    ModTypes = AhoCorasickSingleton(AhoCorasickBuilder())
    ModOptFiles = AhoCorasickSingleton(AhoCorasickBuilder())
    DownloadModes = AhoCorasickSingleton(AhoCorasickBuilder())
    IniModelParts = AhoCorasickSingleton(AhoCorasickBuilder())
##### EndScript