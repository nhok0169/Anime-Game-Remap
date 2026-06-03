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
from typing import List
##### EndExtImports

##### LocalImports
from .Error import Error
##### EndLocalImports


##### Script
class ConflictingOptions(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the script or :class:`RemapService` is ran with options that cannot be used together

    Parameters
    ----------
    options: List[:class:`str`]
        The options that cannot be used together
    """
    def __init__(self, options: List[str]):
        optionsStr = ", ".join(options)
        super().__init__(f"The following options cannot be used toghether: {optionsStr}")
##### EndScript