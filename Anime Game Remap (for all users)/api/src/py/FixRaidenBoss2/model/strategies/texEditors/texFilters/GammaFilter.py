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

##### CppLocalImports
from .....core import CppGammaFilter
##### EndCppLocalImports


##### Script
class GammaFilter(CppGammaFilter):
    """
    This class inherits from :class:`CppGammaFilter`

    Performs a `Gamma Correction`_ on the texture file. See :class:`CppGammaFilter` for more details

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`CppBaseTexFilter.transform` for the filter, ``x``

    Parameters
    ----------
    gamma: :class:`float`
        The luminance parameter for how bright humans perceive the image.

    .. note::
        See :attr:`CppGammaFilter.gamma` for the corresponding attribute
    """

    pass
##### EndScript
