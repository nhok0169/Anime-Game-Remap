##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### LocalImports
from .Error import Error
##### EndLocalImports


##### Script
class BadBlendData(Error):
    """
    This Class inherits fsrom :class:`Error`

    Exception when certain bytes do not correspond to the format defined for a Blend.buf file
    """

    def __init__(self):
        super().__init__(f"Bytes do not corresponding to the defined format for a Blend.buf file")
##### EndScript
