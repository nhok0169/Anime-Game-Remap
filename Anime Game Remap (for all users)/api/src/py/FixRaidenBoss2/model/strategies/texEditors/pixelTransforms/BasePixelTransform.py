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
from .....core import CppBasePixelTransform
##### EndCppLocalImports


##### Script
class BasePixelTransform(CppBasePixelTransform):
    """
    This class inherits from :class:`CppBasePixelTransform`

    Base class for transforming a pixel in a texture file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(pixel, xCoord, yCoord)

            Calls :meth:`CppBasePixelTransform.transform` for the :class:`BasePixelTransform`, ``x``
    """

    pass
##### EndScript
