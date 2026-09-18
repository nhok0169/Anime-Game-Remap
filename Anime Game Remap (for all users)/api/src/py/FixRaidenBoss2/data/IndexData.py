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
from ..core import Indices as _CppIndices
##### EndLocalImports


##### Script

# The literal index data now lives in C++ (py/src/data/IndexData.cpp), backing the C++-bound
# 'Indices' class directly -- there is no Python copy of it any more. This reconstructs the
# original nested-dict shape ({version: {name: {component: {type: index}}}}) from a live
# Indices() instance, computed once at import time, so every existing consumer of this module
# (this project's own ModData.Indices, and any external caller doing `FixRaidenBoss2.IndexData`)
# keeps working unchanged -- see Indices' constructor and ModDictAssets.toNestedDict()
IndexData = _CppIndices().repo.toNestedDict()
##### EndScript
