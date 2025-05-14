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
from typing import Dict, List, TYPE_CHECKING, Union
##### EndExtImports

##### LocalImports
from ....constants.FileSuffixes import FileSuffixes
from ..iniParsers.BaseIniParser import BaseIniParser
from .BaseIniFixer import BaseIniFixer

if (TYPE_CHECKING):
    from .IniFixBuilder import IniFixBuilder
##### EndLocalImports


##### Script
class MultiModFixer(BaseIniFixer):
    """
    This class inherits from :class:`BaseIniFixer`

    Fixes a .ini file where each mod to fix requires a different :class:`BaseIniFixer` strategy

    Parameters
    ----------
    parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix

    fixBuilders: Dict[:class:`str`, :class:`IniFixBuilder`]
        The different builders to dynamcally construct the :class:`BaseIniFixer` used for each mod to fix :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the mods to fix and the values are the different :class:`IniFixBuilder` used to construct the :class:`BaseIniFixer` to fix the mod

    Attributes
    ----------
    _fixBuilders: Dict[:class:`str`, :class:`IniFixBuilder`]
        The different builders to dynamcally construct the :class:`BaseIniFixer` used for each mod to fix :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the mods to fix and the values are the different :class:`IniFixBuilder` used to construct the :class:`BaseIniFixer` to fix the mod

    _fixers: Dict[:class:`str`, :class:`BaseIniFixer`]
        The different fixers to fix each type of mod :raw-html:`<br />` :raw-html:`<br />`

        The keys are the names of the mods to fix and the values are the fixers to fix the mod
    """

    def __init__(self, parser: BaseIniParser, fixBuilders: Dict[str, "IniFixBuilder"]):
        super().__init__(parser)
        self._fixBuilders = fixBuilders
        self._fixers: Dict[str, BaseIniFixer] = {}
        self.buildFixers()

    def buildFixers(self):
        """
        Rebuilds the :class:`BaseIniFixer` used for each mod
        """

        self._fixers = {}
        for modToFix in self._fixBuilders:
            self._fixers[modToFix] = self._fixBuilders[modToFix].build(self._parser)

    # _mergeFix(resultFix, currentFix): Merges the current fix results into the resultant fix
    def _mergeFix(self, resultFix: List[str], currentFix: List[str]):
        resultFixLen = len(resultFix)
        currentFixLen = len(currentFix)
        minFixLen = min(resultFixLen, currentFixLen)

        for i in range(minFixLen):
            resultFix[i] += currentFix[i]

        if (currentFixLen > resultFixLen):
            resultFix.append(currentFix[i])

    def _fix(self, keepBackup: bool = True, fixOnly: bool = False, update: bool = False, hideOrig: bool = False, withBoilerPlate: bool = True, fixId: int = 0) -> Union[str, List[str]]:
        modsToFix = self._parser._modsToFix.intersection(set(self._fixers.keys()))
        sortedModsToFix = list(modsToFix)
        sortedModsToFix.sort()

        result = [""]

        # retrieve the results for each fixer
        for modToFix in sortedModsToFix:
            self._parser._modsToFix = {modToFix}
            fixer = self._fixers[modToFix]
            self._iniFile._iniFixer = fixer
            currentResult = fixer._fix(keepBackup = keepBackup, fixOnly = fixOnly, update = update, hideOrig = hideOrig, withBoilerPlate = False, withSrc = False, fixId = fixId)
            self._iniFile._isFixed = False

            if (isinstance(currentResult, str)):
                result[0] += currentResult
            else:
                self._mergeFix(result, currentResult)

        self._parser._modsToFix = modsToFix

        resultLen = len(result)
        iniFilePath = self._iniFile.filePath
        iniBaseName = iniFilePath.baseName

        if (hideOrig):
            self._iniFile.hideOriginalSections()

        # add the boilerplate
        for i in range(resultLen):
            if (withBoilerPlate):
                result[i] = f"\n\n{self._iniFile.addFixBoilerPlate(fix = result[i])}"

            if (iniFilePath is not None and i > 0):
                iniFilePath.baseName = f"{iniBaseName}{FileSuffixes.RemapFixCopy.value}{i}"

            result[i] = self._iniFile.injectAddition(result[i], beforeOriginal = False, keepBackup = keepBackup, fixOnly = fixOnly, update = update)
            self._iniFile._isFixed = False

        self._iniFile._isFixed = True
        iniFilePath.baseName = iniBaseName
        if (resultLen == 1):
            result = result[0]
        
        return result
##### EndScript

