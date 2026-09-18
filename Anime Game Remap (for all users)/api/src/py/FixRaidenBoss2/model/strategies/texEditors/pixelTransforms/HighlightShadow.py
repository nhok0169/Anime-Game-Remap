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
from .....core import CppHighlightShadow
##### EndCppLocalImports


##### Script
class HighlightShadow(CppHighlightShadow):
    """
    This class inherits from :class:`CppHighlightShadow`

    A filter that approximates the adjustment of the shadow/hightlight of an image

    .. note::
        Reference: `Highlight Shadow Approximation Reference`_

    Parameters
    ----------
    highlight: :class:`float`
        The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``

    shadow: :class:`float`
        The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``0``
    """

    pass
##### EndScript
