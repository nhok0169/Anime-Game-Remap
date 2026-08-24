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
from ...core import CppColour
##### EndCppLocalImports


##### Script
class Colour(CppColour):
    """
    This class inherits from :class:`CppColour`

    Class to store data for a colour

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the hash id for the colour based off :meth:`CppColour.getId`

    :raw-html:`<br />`

    Parameters
    ----------
    red: :class:`int`
        The red channel for the colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    green: :class:`int`
        The green channel for the colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    blue: :class:`int`
        The blue channel for the colour :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``

    alpha: :class:`int`
        The transparency (alpha) channel for the colour with a range from 0-255. 0 = transparent, 255 = opaque :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``255``
    """

    pass
##### EndScript
