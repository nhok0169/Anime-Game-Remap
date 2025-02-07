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
from typing import Optional
##### EndExtImports

##### LocalImports
from ..ModType import ModType
##### EndLocalImports


##### Script
class BaseIniClassifier():
    """
    Base class to help classify the type of mod given the mod's .ini files
    """

    def classify(self, iniTxt: str) -> Optional[ModType]:
        """
        Determines the type of mod given the text from the mod's .ini file

        Parameters
        ----------
        iniTxt: :class:`str`
            The text of the .ini file to read from

        Returns 
        -------
        Optional[:class:`ModType`]
            The found type of mod
        """

        pass
##### EndScript