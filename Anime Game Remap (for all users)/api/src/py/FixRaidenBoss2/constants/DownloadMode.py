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
from typing import Optional
##### EndExtImports

##### LocalImports
from .GlobalClassifiers import GlobalClassifiers
from ..tools.tries.AhoCorasickSingleton import AhoCorasickSingleton
from ..tools.enums.StrEnum import StrEnum
##### EndLocalImports


##### Script
class DownloadMode(StrEnum):
    """
    The download mode of how the software handles file downloads
    """

    Disabled = "disabled"
    """
    Will not perform any file downloads for any mods
    """
    
    Normal = "normal"
    """
    Only perform file downloads at places in a .ini file where a resource is missing
    """

    Always = "always"
    """
    Will always perform file downloads for every mod, if possible, using pessimistic assumptions
    """

    @classmethod
    def _buildAhocorasickDFA(cls) -> AhoCorasickSingleton:
        data = {}
        for downloadMode in cls:
            data[downloadMode.value] = downloadMode
        
        dfa = GlobalClassifiers.DownloadModes.value
        dfa.setup(data)
        return dfa

    @classmethod
    def search(cls, txt: str) -> Optional["DownloadMode"]:
        return super().search(txt.lower().strip())
##### EndScript