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
from .....core import CppTransparency
##### EndCppLocalImports


##### Script
class Transparency(CppTransparency):
    """
    This class inherits from :class:`CppTransparency`

    Adjust the trasparency (alpha channel) for an image

    Parameters
    ----------
    alphaChange: :class:`int`
        How much to adjust the alpha channel of each pixel. Range from -255 to 255

        .. note::
            The alpha channel for an image is inclusively bounded from 0 to 255
    """

    pass
##### EndScript
