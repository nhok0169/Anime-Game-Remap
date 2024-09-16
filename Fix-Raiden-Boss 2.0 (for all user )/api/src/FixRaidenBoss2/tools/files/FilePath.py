##### Credits

# ===== Raiden Boss Fix =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
import os
##### EndExtImports


##### Script
class FilePath():
    """
    Class for storing info about a file path

    Parameters
    ----------
    path: :class:`str`
        The file path
    """

    def __init__(self, path: str):
        self._folder = ""
        self._base = ""
        self.path = path

    @property
    def path(self):
        """
        The file path

        :getter: Retrieves the path
        :setter: Sets a new path
        :type: :class:`str`
        """
        return self._path
    
    @path.setter
    def path(self, newPath: str):
        self._path = newPath
        self._folder = os.path.dirname(newPath)
        self._base = os.path.basename(newPath)

    @property
    def folder(self):
        """
        The parent folder for the path

        :getter: Retrieves the parent folder name
        :type: :class:`str`
        """
        return self._folder
    
    @property
    def base(self):
        """
        The basename for the file path

        :getter: Retrieves the basename
        :type: :class:`str`
        """
        return self._base
##### EndScript
