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
from collections import OrderedDict
from typing import Any
##### EndExtImports


##### Script
# KeepAllDict: Dictionary used to keep all instances of a key
class KeepAllDict(OrderedDict):
    def __init__(self):
        super().__init__()
        self._orderInd = 0

    def __setitem__(self, key, value):
        keyExists = key in self
        valueIsList = isinstance(value, list)

        if (keyExists and valueIsList):
            self[key].append(f"{self._orderInd}_{value[0]}")
            self._orderInd += 1
            return
        elif (valueIsList):
            super().__setitem__(key, [f"{self._orderInd}_{value[0]}"])
            self._orderInd += 1
            return

        elif (isinstance(value, str) and keyExists and isinstance(self[key], list)):
            return

        super().__setitem__(key, value)
##### EndScript