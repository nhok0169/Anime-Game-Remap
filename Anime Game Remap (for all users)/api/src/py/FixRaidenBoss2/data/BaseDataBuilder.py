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
from typing import Dict, Optional, List, Any, Hashable
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T
from ..tools.FlyweightBuilder import FlyweightBuilder
##### EndLocalImports


##### Script
class BaseDataBuilder(FlyweightBuilder):
    DefaultId = "default"

    def __init__(self):
        super().__init__(self._buildData)

    def build(self, args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None, id: Optional[Hashable] = DefaultId, cache: bool = True) -> T:
        return super().build(args = args, kwargs = kwargs, id = id, cache = cache)

    def _buildData(self) -> T:
        pass
##### EndScript