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
from typing import TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .BaseIniClassifierBuilder import BaseIniClassifierBuilder
from ....constants.ModTypes import ModTypes

if (TYPE_CHECKING):
    from .IniClassifier import IniClassifier
##### EndLocalImports


##### Script
class IniClassifierBuilder(BaseIniClassifierBuilder):
    """
    This class inherits from :class:`BaseIniClassifierBuilder` :raw-html:`<br />` :raw-html:`<br />`

    Class to help build/customize a :class:`IniClassifier` used for this software

    Attributes
    ----------
    _startStateId: :class:`str`
        The id for the root state
    """

    def __init__(self):
        self._startStateId = "root"

    def build(self, classifier: "IniClassifier"):
        classifier._stateDFA.addState(self._startStateId)
##### EndScript