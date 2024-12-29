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

    PaintTempIncRedFactor = 0.41
    """
    The parameter for approximately how fast the red channel increases for the temperature increase algorithm from Paint.net
    """

    PaintTempIncBlueFactor = 0.44
    """
    The parameter for approximately how fast the blue channel decreases for the temperature increase algorithm from Paint.net
    """

    PaintTempDecRedFactor = 0.5
    """
    The parameter for approximately how fast the red channel decreases for the temperature decrease algorithm from Paint.net
    """

    PaintTempDecBlueFactor = 2
    """
    The parameter for approximately how fast the blue channel increases for the temperature decrease algorithm from Paint.net
    """

    StandardGamma = 2.2
    """
    The reciprocal of the standard gamma value (1/2.2) used in computer displays, sRGB images, Adobe RGB images. See :class:`CorrectGamma` for more info.
    """

    SRGBGamma = 1 / StandardGamma
    """
    The standard gamma value (1/2.2) typically used in computer displays, sRGB images, Adobe RGB images. See :class:`CorrectGamma` for more info.
    """
##### EndScript