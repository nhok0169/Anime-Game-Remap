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
from typing import TYPE_CHECKING, Optional
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
from .....core import IfContentPart
##### EndCppLocalImports

##### LocalImports
from ..BaseIniGraphPartEdit import BaseIniGraphPartEdit

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class RegRemap(BaseIniGraphPartEdit):
    def __init__(self, keyRemap: Dict[Any, Union[List[Union[Any, :class:`RemappedKeyData`]], :class:`KeyRemapData`]]):
        pass

    def edit(self, part: IfContentPart, sectionName: str, modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        pass
##### EndScript