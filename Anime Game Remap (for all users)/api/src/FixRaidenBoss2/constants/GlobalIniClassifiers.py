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
from enum import Enum
##### EndExtImports

##### LocalImports
from ..model.strategies.iniClassifiers.IniClassifier import IniClassifier
from ..model.strategies.iniClassifiers.IniClassifierBuilder import IniClassifierBuilder
##### EndLocalImports


##### Script
class GlobalIniClassifiers(Enum):
    """
    Global modules used by the sofware to help identify what mod belongs to a .ini file

    Attributes
    ----------
    Classifier: :class:`IniClassifier`
        The classifier used to identify whether the .ini file belongs to some mod
    """

    Classifier = IniClassifier(builder = IniClassifierBuilder())
##### EndScript