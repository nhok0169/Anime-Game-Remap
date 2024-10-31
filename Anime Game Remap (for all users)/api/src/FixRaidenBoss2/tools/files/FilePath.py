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
        self._baseName = ""
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
        self._baseName = os.path.splitext(self._base)[0]

    @property
    def folder(self):
        """
        The parent folder for the path

        :getter: Retrieves the parent folder name
        :setter: Sets the new parent folder name
        :type: :class:`str`
        """
        return self._folder
    
    @folder.setter
    def folder(self, newFolder: str):
        self._folder = newFolder
        self._path = os.path.join(self._folder, self._base)
    
    @property
    def base(self):
        """
        The base for the file path (includes file extension)

        :getter: Retrieves the base
        :setter: Sets the new base for the file path
        :type: :class:`str`
        """
        return self._base
    
    @base.setter
    def base(self, newBase: str):
        self._base = newBase
        self._path = os.path.join(self._folder, self._base)
        self._baseName = os.path.splitext(self._base)[0]

    @property
    def baseName(self):
        """
        The basename for the file path without any file extensions

        :getter: Retrieves the basename
        :setter: Sets the new basename for the file path
        :type: :class:`str`
        """
        return self._baseName
    
    @baseName.setter
    def baseName(self, newBaseName: str):
        self._baseName = newBaseName
        oldBaseName, ext = os.path.splitext(self._base)
        self._base = f"{self._baseName}{ext}"
        self._path = os.path.join(self._folder, self._base)
##### EndScript
