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
from typing import Union, Dict
##### EndExtImports

##### LocalImports
from ..iniParsers.BaseIniParser import BaseIniParser
##### EndLocalImports


##### Script
class BaseIniFixer():
    """
    Base class to fix a .ini file

    Parameters
    ----------
    parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix

    Attributes
    ----------
    _parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix

    _iniFile: :class:`IniFile`
        The .ini file that will be fixed
    """

    def __init__(self, parser: BaseIniParser):
        self._parser = parser
        self._iniFile = parser._iniFile

    def clear(self):
        """
        Resets any saved states within the fixer
        """

        pass

    def _fix(self, keepBackup: bool = True, fixOnly: bool = False, hideOrig: bool = False, withBoilerPlate: bool = True, withSrc: bool = True) -> Union[str, Dict[Union[str, int], str]]:
        pass

    def fix(self, keepBackup: bool = True, fixOnly: bool = False, hideOrig: bool = False) -> Union[str, Dict[Union[str, int], str]]:
        """
        Fixes the .ini file

        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether to keep backups for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to only fix the .ini file without undoing any fixes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        hideOrig: :class:`bool`
            Whether to hide the mod for the original character :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Union[:class:`str`, Dict[Union[:class:`str`, :class:`int`], :class:`str`]]
            The new content of the .ini file(s)  :raw-html:`<br />` :raw-html:`<br />`
            
            If this value is a dictionary, then:
             
            * The keys are the file paths to the .ini files, if available. If the file paths are not available, then the keys are integer ids
            * The values are the new content of both the original and newly created .ini files related to fixing the particular .ini file
        """

        result = self._fix(keepBackup = keepBackup, fixOnly = fixOnly, hideOrig = hideOrig, withBoilerPlate = True, withSrc = True)
        self._iniFile._isFixed = True
        return result
##### EndScript
