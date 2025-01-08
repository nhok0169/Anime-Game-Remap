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
from enum import Enum
##### EndExtImports


##### Script
class FileEncodings(Enum):
    UTF8 = "utf-8"
    Latin1 = "latin1"


IniFileEncoding = FileEncodings.UTF8.value
ReadEncodings = [IniFileEncoding, FileEncodings.Latin1.value]
##### EndScript