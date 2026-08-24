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
from .....core import CppCorrectGamma
##### EndCppLocalImports


##### Script
class CorrectGamma(CppCorrectGamma):
    """
    This class inherits from :class:`CppCorrectGamma`

    Performs a `Gamma Correction`_ on an individual pixel. See :class:`CppCorrectGamma` for more details

    Parameters
    ----------
    gamma: :class:`float`
        The luminance parameter for how bright humans perceive the image.

    .. note::
        See :attr:`CppCorrectGamma.gamma` for the corresponding attribute
    """

    pass
##### EndScript
