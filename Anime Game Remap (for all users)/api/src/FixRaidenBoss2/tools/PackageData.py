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
from typing import Optional
##### EndExtImports


##### Script
class PackageData():
    """
    Data class to hold data relating to retrieving/installing a package at runtime

    Parameters
    ----------
    module: :class:`str`
        The name of the module to import

    install: Optional[:class:`str`]
        The name of the installation for the package when using `pip`_ to download from `pypi`_ :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then assume that the name of the installation is the same as the name of the package :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, module: str, installName: Optional[str] = None):
        self.module = module
        self.installName = module if (installName is None) else installName
##### EndScript