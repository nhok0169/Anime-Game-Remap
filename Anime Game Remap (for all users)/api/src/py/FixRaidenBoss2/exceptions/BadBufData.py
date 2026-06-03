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
from .Error import Error
##### EndLocalImports


##### Script
class BadBufData(Error):
    """
    This Class inherits from :class:`Error`

    Exception when certain bytes do not correspond to the format defined for a .buf file

    Parameters
    ----------
    fileType: :class:`str`
        The name for the type of .buf file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``Buffer``
    """

    def __init__(self, fileType: str = "Buffer"):
        super().__init__(f"Bytes do not corresponding to the defined format for a {fileType} file")
##### EndScript
