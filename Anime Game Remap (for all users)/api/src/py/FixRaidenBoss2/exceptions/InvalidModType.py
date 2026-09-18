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
class InvalidModType(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the type of mod specified to fix is not found

    Parameters
    ----------
    type: :class:`str`
        The name for the type of mod specified
    """
    def __init__(self, type: str):
        super().__init__(f"Unable to find the type of mod by the search string, '{type}'")
##### EndScript