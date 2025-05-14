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
from typing import Optional
##### EndExtImports

##### LocalImports
from .GlobalClassifiers import GlobalClassifiers
##### EndLocalImports


##### Script
class DownloadMode(Enum):
    """
    The download mode of how the software handles file downloads
    """

    Disabled = "disabled"
    """
    Will not perform any file downloads for any mods
    """

    Always = "always"
    """
    Will always perform file downloads for every mod
    """

    Normal = "normal"
    """
    Will perform file downloads based off the following heuristics:

    #. Download textures or .ib files if there is a branch in the texture `sections`_ that does not reference the files
    #. Download model binary files if either texture/.ib downloads needed to be performed or there is branch in the vertex buffer `sections`_ that does not reference a resource to some vertex buffer metadata

    .. warning::
        The following heuristics may not download any files for certain cases that require file downloads 
        
        In such cases, you may need to switch using the :attr:`Always` download mode
    """

    @classmethod
    def setup(cls):
        if (GlobalClassifiers.DownloadModes.value.isSetup):
            return
        
        data = {}
        for downloadMode in cls:
            data[downloadMode.value] = downloadMode
        
        GlobalClassifiers.DownloadModes.value.setup(data)

    @classmethod
    def search(cls, mode: str) -> Optional["DownloadMode"]:
        """
        Searches a download mode based off the provided name

        Parameters
        ----------
        mode: :class:`str`
            The name of the download mode to search for

        Returns
        -------
        Optional[:class:`DownloadMode`]
            The found download mode based off the provided name
        """

        cls.setup()
        keyword, downloadMode = GlobalClassifiers.DownloadModes.value.dfa.getMaximal(mode.lower().strip(), errorOnNotFound = False)
        return downloadMode
##### EndScript