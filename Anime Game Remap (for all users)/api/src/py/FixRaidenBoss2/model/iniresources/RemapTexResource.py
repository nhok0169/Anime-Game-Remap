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
from typing import Optional, Callable, Any
##### EndExtImports

##### LocalImports
from .RemapIniResource import RemapIniResource
from ..stats.RemapStats import RemapStats
from ..files.TextureFile import TextureFile
from ..strategies.texEditors.TexCreator import TexCreator
##### EndLocalImports


##### Script
class RemapTexAddResource(RemapIniResource):
    def __init__(self, iniFolderPath: str, srcPath: str, texCreator: TexCreator, type: str = "resourceRemapTexAdd", fixFunc: Optional[Callable[["RemapTexAddResource"], Any]] = None):
        super().__init__(type, iniFolderPath, srcPath, fixFunc = fixFunc)
        self.texCreator = texCreator

    def srcEncounteredError(self, stats: RemapStats) -> bool:
        return self.srcPath in stats.texAdd.skipped
    
    def srcIsFixed(self, stats: RemapStats) -> bool:
        return self.srcPath in stats.texAdd.fixed
    
    def fixEncounteredError(self, stats: RemapStats):
        return self.srcPath in stats.texAdd.skipped
    
    def fixIsFixed(self, stats: RemapStats):
        return self.srcPath in stats.texAdd.fixed
    
    def fixExists(self, stats: RemapStats):
        return os.path.isfile(self.srcPath)
    
    def _fix(self, *args, **kwargs):
        """
        Performs a vertex group remap on the blend.buf file
        """

        texture = TextureFile(self.srcPath)
        self.texCreator.fix(texture, self.srcPath)
##### EndScript