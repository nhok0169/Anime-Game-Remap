##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from typing import TYPE_CHECKING, Callable, Union, List, Optional
##### EndExtImports

##### LocalImports
from ..pixelTransforms.BasePixelTransform import BasePixelTransform
from ....textures.Colour import Colour
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class PixelFilter(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    Manipulates each pixel within an image :raw-html:`<br />` :raw-html:`<br />`

    .. warning::
        This filter iterates through every pixel of the image using Python's for loops. It is recommended to try to use
        the different filters inherited from the :class:`BaseTexFilter` class since those filters have more capability to
        interact with `Pillow`_ API or the `Numpy`_ API, where their implementation are written at the C++ level,
        allowing images to be editted A LOT faster.

    Parameters
    ----------
    transforms: Optional[List[Union[:class:`BasePixelTransform`, Callable[[:class:`Colour`, :class:`int`, :class:`int`], Any]]]]
        The functions to edit a single pixel in the texture file :raw-html:`<br />` :raw-html:`<br />`

        The functions take the following parameters:

        #. The RGBA colour of the pixel
        #. The x-coordinate
        #. The y-coordinate

        :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    transforms: List[Union[:class:`BasePixelTransform`, Callable[[:class:`Colour`], :class:`Colour`]]]
        The transformation functions to edit a single pixel in the texture file
    """

    def __init__(self, transforms: Optional[List[Union[BasePixelTransform, Callable[[Colour, int, int], Colour]]]] = None):
        self.transforms = [] if (transforms is None) else transforms

    def transform(self, texFile: "TextureFile"):
        """
        Changes each individual pixel in the image

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be editteds
        """

        if (self.transforms):
            pixels = texFile.read()
            pixelColour = Colour()
            
            for y in range(texFile.img.size[1]):
                for x in range(texFile.img.size[0]):
                    pixel = pixels[x, y]
                    pixelColour.fromTuple(pixel)

                    for transformation in self.transforms:
                        transformation(pixelColour, x, y)

                    pixels[x, y] = pixelColour.getTuple()
##### EndScript