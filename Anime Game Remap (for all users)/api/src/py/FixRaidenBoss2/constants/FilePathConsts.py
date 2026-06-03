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
import os
from typing import Optional
##### EndExtImports


##### Script
class FilePathConsts():
    DefaultPath = os.getcwd()
    CurrentDir = "."

    @classmethod
    def getPath(cls, path: Optional[str]) -> str:
        if (path is None):
            return cls.DefaultPath
        return path
##### EndScript