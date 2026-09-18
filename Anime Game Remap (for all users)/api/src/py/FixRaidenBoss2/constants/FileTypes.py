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

    Position = f"Position{FileExt.Buf.value}"
    """
    Position.buf files
    """

    Texture = f"*{FileExt.DDS.value}"
    """
    Texture .dds files
    """

    RemapBlend = f"Remap{Blend}"
    """
    RemapBlend.buf files    
    """

    RemapPosition = f"Remap{Position}"
    """
    RemapPostion.buf files
    """

    Log = f"RemapFixLog{FileExt.Txt.value}"
    """
    Log file
    """

    RemapTexture = f"RemapTex{FileExt.DDS.value}"
    """
    RemapTex.dds files
    """

    RemapDownload = f"RemapDL download"
    """
    RemapDL download files
    """

    Texcoord = f"Texcoord{FileExt.Buf.value}"
    """
    Texcoord.buf files
    """

    RemapTexcoord = f"Remap{Texcoord}"
    """
    RemapTexcoord.buf files created by this fix
    """

    RemapBuf = f"Remap*{FileExt.Buf.value}"
    """
    Any other .buf file created by this fix -- the catch-all for the .buf half of
    :meth:`RemapIniRemover.classifyResource`
    """

    RemapOther = "Remap file"
    """
    A file of no recognized kind created by this fix -- the final catch-all of
    :meth:`RemapIniRemover.classifyResource`
    """
##### EndScript