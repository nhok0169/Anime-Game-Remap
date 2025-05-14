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
class ShortCommandOpts(Enum):
    Src = "-s"
    DeleteBackup = '-d'
    FixOnly = '-f'
    Revert = '-u'
    All = '-a'
    Types = "-t"
    FixedTypes = "-rt"
    ForceType = "-ft"
    Version = "-v"
    Log = "-l"
    DefaultType = "-dt"
    HideOriginal = "-ho"
    Download = "-dl"
    Proxy = "-p"
##### EndScript