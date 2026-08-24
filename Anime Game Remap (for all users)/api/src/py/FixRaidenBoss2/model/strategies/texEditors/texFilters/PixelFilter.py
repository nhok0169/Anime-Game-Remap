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

##### ExtImports
from typing import TYPE_CHECKING, Callable, Union, List, Optional, Any
##### EndExtImports

##### CppLocalImports
from .....core import CppPixelFilter
##### EndCppLocalImports

##### LocalImports
from ..pixelTransforms.BasePixelTransform import BasePixelTransform
from ....textures.Colour import Colour

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class PixelFilter(CppPixelFilter):
    """
    This class inherits from :class:`CppPixelFilter`

    Manipulates each pixel within an image :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        A :class:`BasePixelTransform` placed in :attr:`transforms` runs at full C++ speed with no
        per-pixel Python overhead; a plain Python callable does not, since it needs to be called
        from Python once per pixel

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`CppBaseTexFilter.transform` for the filter, ``x``

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
    transforms: List[Union[:class:`BasePixelTransform`, Callable[[:class:`Colour`, :class:`int`, :class:`int`], Any]]]
        The transformation functions to edit a single pixel in the texture file
    """

    def __init__(self, transforms: Optional[List[Union[BasePixelTransform, Callable[[Colour, int, int], Any]]]] = None):
        super().__init__()
        self.transforms = [] if (transforms is None) else transforms
##### EndScript
