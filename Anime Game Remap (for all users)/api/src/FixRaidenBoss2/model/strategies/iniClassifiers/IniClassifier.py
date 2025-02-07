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
from typing import Optional, Callable
##### EndExtImports

##### LocalImports
from ..ModType import ModType
from .BaseIniClassifier import BaseIniClassifier
##### EndLocalImports


##### Script
class IniClassifier(BaseIniClassifier):
    """
    This class inherits from :class:`BaseIniClassifier`

    Class to help classify the type of mod given the mod's .ini files
    """

    def add(self, modType: ModType, keyword: str, statePredId: str, statePred: Callable[["IniClassifier"], bool]):
        """
        
        """

        pass
##### EndScript