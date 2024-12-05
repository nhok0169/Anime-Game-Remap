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

##### LocalImports
from .FileExt import FileExt
##### EndLocalImports


##### Script
class FileTypes(Enum):
    """
    Different types of files the software encounters
    """

    Default = "file"
    """
    Default file type
    """

    Ini = f"*{FileExt.Ini.value} file"
    """
    Initialization files
    """

    Blend = f"Blend{FileExt.Buf.value}"
    """
    Blend.buf files
    """

    Texture = f"*{FileExt.DDS.value}"
    """
    Texture .dds files
    """

    RemapBlend = f"Remap{Blend}"
    """
    RemapBlend.buf files    
    """

    Log = f"RemapFixLog{FileExt.Txt.value}"
    """
    Log file
    """

    RemapTexture = f"RemapTex{FileExt.DDS.value}"
    """
    RemapTex.dds files
    """
##### EndScript