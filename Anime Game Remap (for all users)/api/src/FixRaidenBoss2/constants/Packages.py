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
from enum import Enum
##### EndExtImports

##### LocalImports
from ..tools.PackageData import PackageData
##### EndLocalImports


##### Script
class PackageInstall(Enum):
    """
    Installation names for external packages to retrieve from `pypi`_
    """

    Pillow = "pillow"
    """
    Package for manipulating with images
    """


class PackageModules(Enum):
    """
    The data about modules from external packages used by the software

    Attributes
    ----------
    PIL_Image: :class:`PackageData`
        Module for PIL.Image

    PIL_ImageEnhance: :class:`PackageData`
        Module for PIL.ImageEnhance
    """

    PIL_Image = PackageData("PIL.Image", PackageInstall.Pillow.value)
    PIL_ImageEnhance = PackageData("PIL.ImageEnhance", PackageInstall.Pillow.value)
##### EndScript