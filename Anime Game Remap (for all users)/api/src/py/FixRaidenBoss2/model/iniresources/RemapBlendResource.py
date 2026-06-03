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
import os
from typing import Optional, Callable, List
##### EndExtImports

##### LocalImports
from .RemapIniResource import RemapIniFixResource
from ..stats.RemapStats import RemapStats
from ..files.BlendFile import BlendFile
from ..buffers.BufElementType import BufElementType
from ..VGRemap import VGRemap
##### EndLocalImports


##### Script
class RemapBlendResource(RemapIniFixResource):
    """
    This class inherits from :class:`RemapIniFixResource`
    
    Class for fixing some Blend.buf file used by the overall remap process at :class:`RemapService`

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The path to the folder of the .ini file

    srcPath: :class:`str`
        The file path to the resource

    vgRemap: :class:`VGRemap`
        The vertex group remap for the blend.buf file

    type: :class:`str`
        The name for the type of resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``"remapBlend"``

    fixFunc: Optional[Callable[[:class:`RemapIniDownload`, :class:`Mod`, :class:`RemapStats`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        The function takes in:

        #. The resource to fix
        #. The mod where the resource comes from
        #. The file stats to track :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    blendElements: Optional[List[:class:`BufElementType`]]
        The sequence of elements for constructing the blend.buf file :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, will use the elements used for a GIMI character :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, iniFolderPath: str, srcPath: str, fixedPath: str, vgRemap: VGRemap, type: str = "resourceRemapBlend", 
                 fixFunc: Optional[Callable[["RemapBlendResource"], bool]] = None, blendElements: Optional[List[BufElementType]] = None):

        super().__init__(type, iniFolderPath, srcPath, fixedPath, fixFunc)
        self.vgRemap = vgRemap
        self.blendElements = blendElements

    def srcEncounteredError(self, stats: RemapStats) -> bool:
        return self.srcPath in stats.blend.skipped
    
    def srcIsFixed(self, stats: RemapStats) -> bool:
        return self.srcPath in stats.blend.fixed
    
    def fixEncounteredError(self, stats: RemapStats):
        return self.fixedPath in stats.blend.skipped
    
    def fixIsFixed(self, stats: RemapStats):
        return self.fixedPath in stats.blend.fixed
    
    def fixExists(self, stats: RemapStats):
        return os.path.isfile(self.fixedPath)

    def createBlend(self) -> BlendFile:
        """
        Creates the blend file

        Returns
        -------
        :class:`BlendFile`
            The created blend file
        """

        return BlendFile(self.srcPath, elements = self.blendElements)

    def _fix(self, *args, **kwargs):
        """
        Performs a vertex group remap on the blend.buf file
        """

        blend = self.createBlend()
        blend.remap(self.vgRemap, fixedBlendFile = self.fixedPath)
##### EndScript