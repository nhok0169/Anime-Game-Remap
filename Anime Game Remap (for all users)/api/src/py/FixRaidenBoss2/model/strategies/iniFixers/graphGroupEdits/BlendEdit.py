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
from typing import Optional, Tuple, TYPE_CHECKING, Callable
##### EndExtImports

##### LocalImports
from ....IniNamingTools import IniNamingTools
from ....iniresources.IniResource import IniResource
from ....iniresources.RemapBlendResource import RemapBlendResource
from .....tools.HashTools import HashTools
from .....tools.TextTools import TextTools
from .ResEdit import ResReplace

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class RemapBlendReplace(ResReplace):
    """
    
    """

    def __init__(self, resModObj: Tuple[int, str, str], resType: str = "resourceRemapBlend", fixFunc: Optional[Callable[[RemapBlendResource], bool]] = None, 
                 resSubType: Optional[str] = None, fromComp: Optional[str] = None, toComp: Optional[str] = None):
        super().__init__(resType, resModObj)
        self.fixFunc = fixFunc
        self.fromComp = fromComp
        self.toComp = toComp
        self.resSubType = resSubType

    def getFixResourceName(self, resource: str, modType: "ModType", modName: str = "") -> Optional[str]:
        modName = TextTools.capitalize(modName)
        if (self.resSubType is not None):
            modName += TextTools.capitalize(self.resSubType)

        return IniNamingTools.getRemapBlendResourceName(resource, modName = modName)
    
    def getFixFile(self, file: str, modType: "ModType", modName: str = "", graphId: str = "") -> str:
        modName = TextTools.capitalize(modName)
        if (self.resSubType is not None):
            modName += TextTools.capitalize(self.resSubType)

        result = IniNamingTools.getFixedBlendFile(file, modName = modName)
        if (not graphId):
            return result

        return self.fileAddGraphId(result, graphId = HashTools.base64DeterministicShortUniqueHash(graphId))

    def buildResModel(self, resType: str, ini: "IniFile", srcPath: str, fixedPath: str, modType: "ModType", *args, modName: str = "", **kwargs) -> IniResource:
        vgRemap = modType.getVGRemap(modName, fromVersion = ini.version, toVersion = ini.toVersion, fromComp = self.fromComp, toComp = self.toComp)
        return RemapBlendResource(ini.folder, srcPath, fixedPath, vgRemap, type = self.resType, fixFunc = self.fixFunc)
##### EndScript