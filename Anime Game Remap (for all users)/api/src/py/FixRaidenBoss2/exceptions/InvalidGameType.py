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
class InvalidGameType(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the type of game specified to fix mods for is not found

    Parameters
    ----------
    game: :class:`str`
        The name for the type of game specified
    """
    def __init__(self, game: str):
        super().__init__(f"Unable to find the type of game by the search string, '{game}'")
##### EndScript
