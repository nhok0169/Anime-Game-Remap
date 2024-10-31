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


##### ExtImports
import os
##### EndExtImports

##### LocalImports
from .FileException import FileException
##### EndLocalImports


##### Script
class BlendFileNotRecognized(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a Blend.buf file cannot be read

    Parameters
    ----------
    blendFile: :class:`str`
        The file path to the Blend.buf file
    """
    def __init__(self, blendFile: str):
        super().__init__(f"Blend file format not recognized for {os.path.basename(blendFile)}", path = os.path.dirname(blendFile))
##### EndScript