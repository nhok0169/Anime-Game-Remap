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


##### Script
class Error(Exception):
    """
    The base exception used by this module

    Parameters
    ----------
    message: :class:`str`
        the error message to print out
    """

    def __init__(self, message: str):
        super().__init__(f"ERROR: {message}")
##### EndScript