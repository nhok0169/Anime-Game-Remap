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
from typing import Any
##### EndExtImports

##### LocalImports
from ..Model import Model
##### EndLocalImports


##### Script
class File(Model):
    """
    Base class for a file
    """

    def read(self) -> Any:
        """
        Reads the data within a file        
        """
        pass
##### EndScript