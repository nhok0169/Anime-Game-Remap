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
from typing import TYPE_CHECKING
##### EndExtImports

##### LocalImports
if (TYPE_CHECKING):
    from ...IniFile import IniFile
##### EndLocalImports


##### Script
class BaseIniRemover():
    """
    Base class to remove fixes from a .ini file

    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to remove the fix from

    Attributes
    ----------
    iniFile: :class:`IniFile`
        The .ini file that will be parsed
    """

    def __init__(self, iniFile: "IniFile"):
        self.iniFile = iniFile

    @staticmethod
    def _readLines(func):
        """
        Decorator to read all the lines in the .ini file first before running a certain function

        All the file lines will be saved in :attr:`IniFile._fileLines`

        Examples
        --------
        .. code-block:: python
            :linenos:

            @_readLines
            def printLines(self):
                for line in self.iniFile.fileLines:
                    print(f"LINE: {line}")
        """

        def readLinesWrapper(self, *args, **kwargs):
            if (not self.iniFile._fileLinesRead):
                self.iniFile.readFileLines()
            return func(self, *args, **kwargs)
        return readLinesWrapper

    def remove(self, parse: bool = False) -> str:
        """
        Removes the fix from the .ini file

        Parameters
        ----------
        parse: :class:`bool`
            Whether to also parse for the .*RemapBlend.buf files that need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new content of the .ini file
        """
        pass
##### EndScript