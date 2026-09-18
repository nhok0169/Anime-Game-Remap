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
from .Error import Error
##### EndLocalImports


##### Script
class InvalidDownloadMode(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the download mode to activate is not found

    Parameters
    ----------
    mode: :class:`str`
        The name for the download mode specified
    """
    def __init__(self, mode: str):
        super().__init__(f"Unable to find the download mode by the string, '{mode}'")
##### EndScript