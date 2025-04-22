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
from ..tools.PackageData import PackageData
from ..tools.PackageManager import PackageManager
##### EndLocalImports


##### Script
class GlobalPackageManager(Enum):
    """
    Global pacakge manager for handling external libraries

    Attributes
    ----------
    Packager: :class:`PackageManager`
        The pacakge manager used by the softwares
    """

    Packager = PackageManager()

    @classmethod
    def get(cls, packageData: PackageData):
        """
        Convenience function to call :meth:`PackageManager.get` from :attr:`Packager`

        Parameters
        ----------
        packageData: :class:`PackageData`
            The data needed for install the external package

        Returns
        -------
        `Module`_
            The module to the external package
        """

        return cls.Packager.value.get(packageData)
##### EndScript