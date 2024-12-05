##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from enum import Enum
##### EndExtImports


##### Script
class ColourConsts(Enum):
    """
    Constants about colours
    """

    MinColourValue = 0
    """
    Minimum bound for a colour channel
    """

    MaxColourValue = 255
    """
    Maximum bound for a colour channel
    """

    MinColourDegree = 0
    """
    Minimum degrees for some HSV/HSL images    
    """

    MaxColourDegree = 360
    """
    Maximum degrees for some HSV/HSL images    
    """
##### EndScript