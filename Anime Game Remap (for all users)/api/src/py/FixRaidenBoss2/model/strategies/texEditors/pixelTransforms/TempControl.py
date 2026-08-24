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
from .....core import CppTempControl
##### EndCppLocalImports


##### Script
class TempControl(CppTempControl):
    """
    This class inherits from :class:`CppTempControl`

    Controls the temperature of a texture file using a modified version of the `Simple Image Temperature/Tint Adjust Algorithm`_ such that
    the colour channels increase/decrease linearly with respect to their corresponding pixel value and the user selected temperature

    Parameters
    ----------
    temp: :class:`float`
        The temperature to set the image. Range from -1 to 1 :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``
    """

    pass
##### EndScript
