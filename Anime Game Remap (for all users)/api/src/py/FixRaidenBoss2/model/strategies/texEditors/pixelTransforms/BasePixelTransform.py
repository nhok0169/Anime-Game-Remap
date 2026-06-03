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

##### LocalImports
from ....textures.Colour import Colour
##### EndLocalImports


##### Script
class BasePixelTransform():
    """
    Base class for transforming a pixel in a texture file

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(pixel, xCoord, yCoord)

            Calls :meth:`BasePixelTransform.transform` for the :class:`BasePixelTransform`, ``x``
    """

    def __call__(self, pixel: Colour, x: int, y: int):
        self.transform(pixel, x, y)

    def transform(self, pixel: Colour, x: int, y: int):
        """
        Applies a Transformation to 'pixel'

        Parameters
        ----------
        pixel: :class:`Colour`
            The pixel to be editted

        x: :class:`int`
            x-coordinate of the pixel

        y: :class:`int`
            y-coordinate of the pixel
        """

        pass
##### EndScript