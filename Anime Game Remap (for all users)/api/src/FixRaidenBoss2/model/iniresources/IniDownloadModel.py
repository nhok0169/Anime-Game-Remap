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
from ...tools.files.FileService import FileService
##### EndLocalImports


##### Script
class IniDownloadModel():
    """
    Contains data about a particular resource in the original .ini file

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    path: :class:`str`
        The file path to the downloaded file

    Attributes
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    path: :class:`str`
        The file path to the downloaded file

    fullPath: :class:`str`
        The absolute paths to the downloaded file
    """

    def __init__(self, iniFolderPath: str, path: str):
        self.iniFolderPath = iniFolderPath
        self.path = path
        self.fullPath = FileService.absPathOfRelPath(path, iniFolderPath)
##### EndScript