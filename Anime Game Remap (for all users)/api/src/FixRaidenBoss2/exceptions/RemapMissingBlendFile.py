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
from .FileException import FileException
##### EndLocalImports


##### Script
class RemapMissingBlendFile(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a RemapBlend.buf file is missing its corresponding Blend.buf file

    Parameters
    ----------
    remapBlend: :class:`str`
        The path to the RemapBlend.buf file
    """

    def __init__(self, remapBlend: str):
        super().__init__(f"Missing the corresponding Blend.buf file for the RemapBlend.buf", path = remapBlend)
##### EndScript