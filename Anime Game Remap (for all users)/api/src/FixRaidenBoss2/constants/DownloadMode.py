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

    Always = "always"
    """
    Will always perform file downloads for every mod, if possible
    """

    AlwaysTex = "alwaystex"
    """
    Only download textures or .ib files
    """

    AlwaysBuf = "alwaysbuf"
    """
    Only download .buf files, if possible
    """

    Tex = "tex"
    """
    Only download textures or .ib files if there is a specified branch in the texture `sections`_ that does not reference the files
    """

    Buf = "buf"
    """
    Only download .buf files if there is a specified branch in the .vb `sections`_ that does not reference the files
    """

    HardTexDriven = "hardtexdriven"
    """
    Will perform file downloads based off the following heuristics:

    #. Download textures or .ib files if there is a specified branch in the texture `sections`_ that does not reference the files
    #. If any texture/.ib downloads needed to be performed, then download .buf files at specified branches with missing resources
    """

    HardTexDrivenAll = "texdrivenall"
    """
    Will perform file downloads based off the following heuristics:

    #. Download textures or .ib files if there is a specified branch in the texture `sections`_ that does not reference the files
    #. If any texture/.ib downloads needed to be performed, then download model .buf files at specified/unspecified branch cases with missing resources
    """

    SoftTexDriven = "softtexdriven"
    """
    Will perform file downloads based off the following heuristics:

    #. Download textures or .ib files if there is a specified branch in the texture `sections`_ that does not reference the files
    #. Download .buf files if either texture/.ib downloads needed to be performed or there are specified branch cases with missing resources
    """

    SoftTexDrivenAll = "softtexdrivenall"
    """
    Will perform file downloads based off the following heuristics:

    #. Download textures or .ib files if there is a specified branch in the texture `sections`_ that does not reference the files
    #. Download .buf files if either texture/.ib downloads needed to be performed or there are specified/unspecified branch cases with missing resources
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