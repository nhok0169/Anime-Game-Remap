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
##### EndLocalImports


##### Script
class PackageInstall(Enum):
    """
    Installation names for external packages to retrieve from `pypi`_
    """

    OrderedSet = "ordered-set"
    """
    Package for an ordered set
    """

    Pillow = "pillow"
    """
    Package for manipulating with images
    """

    PyAhoCorasick = "pyahocorasick"
    """
    Package for the `Aho-Corasick`_ algorithm, implemented at the C level
    """

    Requests = "requests"
    """
    Package for handling HTTP requests
    """


class PackageModules(Enum):
    """
    The data about modules from external packages used by the software

    Attributes
    ----------
    AhoCorasick: :class:`PackageData`
        Module for `pyahocorasick`_

    OrderedSet: :class:`PackageData`
        Module for `ordered_set`_

    PIL_Image: :class:`PackageData`
        Module for PIL.Image

    PIL_ImageChops: :class:`PackageData`
        Module for PIL.ImageChops

    PIL_ImageEnhance: :class:`PackageData`
        Module for PIL.ImageEnhance

    Requests: :class:`PackageData`
        Module for `requests`_
    """

    AhoCorasick = PackageData("ahocorasick", PackageInstall.PyAhoCorasick.value)
    OrderedSet = PackageData("ordered_set", PackageInstall.OrderedSet.value)
    PIL_Image = PackageData("PIL.Image", PackageInstall.Pillow.value)
    PIL_ImageChops = PackageData("PIL.ImageChops", PackageInstall.Pillow.value)
    PIL_ImageEnhance = PackageData("PIL.ImageEnhance", PackageInstall.Pillow.value)
    Requests = PackageData("requests", PackageInstall.Requests.value)
##### EndScript