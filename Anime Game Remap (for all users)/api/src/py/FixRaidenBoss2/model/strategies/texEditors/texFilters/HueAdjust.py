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
from .....core import CppHueAdjust
##### EndCppLocalImports


##### Script
class HueAdjust(CppHueAdjust):
    """
    This class inherits from :class:`CppHueAdjust`

    Adjusts the hue of a texture file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`CppBaseTexFilter.transform` for the filter, ``x``

    Parameters
    ----------
    hue: :class:`int`
        The hue to adjust the image. Value is from -180 to 180
    """

    pass
##### EndScript
