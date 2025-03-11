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
from typing import TYPE_CHECKING, Set, Any, Dict, Optional
##### EndExtImports

##### LocalImports
from .BaseTexFilter import BaseTexFilter

if (TYPE_CHECKING):
    from ....files.TextureFile import TextureFile
##### EndLocalImports


##### Script
class TexMetadataFilter(BaseTexFilter):
    """
    This class inherits from :class:`BaseTexFilter`

    A pseudo-filter used to manipulate the metadata of a texture file (`PIL.Image.Image.info`_)

    .. warning::
        Currently, any metadata won't actually be saved into the texture file due to the image library (`Pillow`_) facing difficulty
        porting the `BCn Encoding Algorithm`_ from C/C# to Python. 

        https://github.com/python-pillow/Pillow/issues/4864

        :raw-html:`<br />`
        
        But the following metadata will affect how this software saves the texture file:

        - gamma

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(texFile)

            Calls :meth:`transform` for the filter, ``x``

    Parameters
    ----------
    edits: Optional[Dict[:class:`str`, Any]]
        The edits to perform on the metadata :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    removes: Optional[Set[:class:`str`]]
        keys to remove from the metadata :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    edits: Dict[:class:`str`, Any]
        The edits to perform on the metadata

    removes: Set[:class:`str`]
        keys to remove from the metadata
    """

    def __init__(self, edits: Optional[Dict[str, Any]] = None, removes: Optional[Set[str]] = None):
        self.removes = set() if (removes is None) else removes
        self.edits = {} if (edits is None) else edits

    def transform(self, texFile: "TextureFile"):
        """
        Changes metadata of the image

        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be editted
        """

        for remove in self.removes:
            texFile.img.info.pop(remove, None)

        for editKey in self.edits:
            texFile.img.info[editKey] = self.edits[editKey]
##### EndScript