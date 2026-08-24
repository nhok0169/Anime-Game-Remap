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
from ...core import CppColourRange
##### EndCppLocalImports


##### Script
class ColourRange(CppColourRange):
    """
    This class inherits from :class:`CppColourRange`

    Class to store data for a colour

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the hash id for the colour range based off :meth:`CppColourRange.getId`

    :raw-html:`<br />`

    Parameters
    ----------
    min: :class:`Colour`
        The minimum range for the RGBA values

    max: :class:`Colour`
        The maximum range for the RGBA values
    """

    pass
##### EndScript
