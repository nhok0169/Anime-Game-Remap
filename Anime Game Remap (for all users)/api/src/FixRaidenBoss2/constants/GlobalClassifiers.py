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

##### LocalImports
from ..tools.tries.AhoCorasickBuilder import AhoCorasickBuilder
from ..tools.tries.AhoCorasickSingleton import AhoCorasickSingleton
from ..tools.enums.DeferredEnum import DeferredEnum
##### EndLocalImports


##### Script
class GlobalClassifiers(DeferredEnum):
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

    __buildAhoCorasickSingleton__ = lambda: AhoCorasickSingleton(AhoCorasickBuilder())

    ModTypes = (__buildAhoCorasickSingleton__, )
    ModOptFiles = (__buildAhoCorasickSingleton__, )
    DownloadModes = (__buildAhoCorasickSingleton__, )
    IniModelParts = (__buildAhoCorasickSingleton__, )
##### EndScript