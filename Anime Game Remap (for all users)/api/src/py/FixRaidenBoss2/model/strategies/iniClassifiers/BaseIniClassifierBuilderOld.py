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
if (TYPE_CHECKING):
    from .IniClassifierOld import IniClassifierOld
##### EndLocalImports


##### Script
class BaseIniClassifierBuilderOld():
    """
    Base class to help build/customize a :class:`IniClassifierOld`
    """

    def build(self, classifier: "IniClassifierOld") -> "IniClassifierOld":
        """
        Builds/customize a :class:`IniClassifierOld`

        Parameters
        ----------
        classifier: :class:`IniClassifierOld`
            The classifier to build

        Returns
        -------
        :class:`IniClassifierOld`
            The classifier that has been built
        """

        pass
##### EndScript