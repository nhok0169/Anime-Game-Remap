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
import os
import copy
from typing import Dict, List, Optional, Union
##### EndExtImports

##### LocalImports
from ....constants.FileEncodings import FileEncodings
from ....constants.FileSuffixes import FileSuffixes
from ....constants.IniConsts import IniGraphModObjKeywords
from ..iniParsers.GIMIParser import GIMIParser
from .BaseIniFixer import BaseIniFixer
from ...IniGraphGroup import IniGraphGroup
from .graphGroupEdits.BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
##### EndLocalImports


##### Script
class GIMIFixer(BaseIniFixer):
    """
    This class inherits from :class:`BaseIniFixer`

    Fixes a .ini file used by a GIMI related importer

    Parameters
    ----------
    parser: :class:`GIMIParser`
        The associated parser to retrieve data for the fix

    modObjs: Optional[Dict[Tuple[:class:`str`, :class:`str`], List[Tuple[:class:`str`, :class:`str`]]]]


    Attributes
    ----------
    _parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix

    _iniFile: :class:`IniFile`
        The .ini file that will be fixed
    """

    def __init__(self, parser: GIMIParser,
                 graphGroupEdits: Optional[List[BaseIniGraphGroupEdit]] = None,
                 modsToFix: Optional[List[str]] = None,
                 prevFixer: Optional["GIMIFixer"] = None):
        super().__init__(parser)

        self.graphGroupEdits = graphGroupEdits if (graphGroupEdits is not None) else []
        self.modsToFix = modsToFix
        self.prevFixer = prevFixer

        self.graphGroups: List[IniGraphGroup] = []

    def clear(self):
        self.graphGroups = []

    def getModsToFix(self) -> List[str]:
        """
        Retrieves the mods to fix to

        Returns
        -------
        List[:class:`str`]
            The mods to fix to
        """

        if (self.modsToFix is not None):
            return self.modsToFix

        type = self._iniFile.availableType
        if (type is None):
            return []
        
        return type.getModsToFix()

    def getFix(self, onlyEditObjGraphs: bool = False) -> Optional[Dict[Union[str, int], IniGraphGroup]]:
        """
        Retrieves only the content of the fix

        Parameters
        ----------
        onlyEditObjGraphs: bool
            Whether to only run :attr:`graphGroupEdits`.

            If this value is ``True``, then will not return any output and the results will be saved in :attr:`graphGroups` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Optional[Dict[Union[:class:`str`, :class:`int`], :class:`IniGraphGroup`]]
            The content of the fix :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the file paths to the .ini files, if available. If the file paths are not available, then the keys are integer ids
            * The value are the new
        """

        graphGroups = None

        # retrieve the initial graph groups
        if (self.prevFixer is not None):
            self.prevFixer.getFix(onlyEditObjGraphs = True)
            graphGroups = self.prevFixer.graphGroups
            self.prevFixer.clear()
        else:
            self.graphGroups = [IniGraphGroup()]
            graphGroup = self.graphGroups[0]

            graphGroups = self._parser.commandGraphs
            for modObj in graphGroups:
                graphGroup.addGraph(modObj, graphGroups[modObj].deepcopy())

            graphGroups = self._parser.downloadResourceGraphs
            for modObj in graphGroups:
                objGraphGroups = graphGroups[modObj]

                for reg in objGraphGroups:
                    graph = objGraphGroups[reg]
                    download = self._parser.downloads[modObj][reg]
                    downloadModObj = (IniGraphModObjKeywords.Download.value, download.name)

                    if (downloadModObj not in graphGroup.graphs):
                        graphGroup.addGraph(downloadModObj, graph)

            graphGroups = self.graphGroups

        # edit the group of graphs
        modType = self._iniFile.type
        modsToFix = self.getModsToFix()

        for modToFix in modsToFix:
            for graphGroupEdit in self.graphGroupEdits:
                graphGroups = graphGroupEdit.editFromIni(graphGroups, self._parser._iniFile, modType = modType, modName = modToFix)

        self.graphGroups = graphGroups

        if (onlyEditObjGraphs):
            return

        # assign file names to the result
        result = {}
        iniFilePath = copy.deepcopy(self._iniFile.filePath)
        iniBaseName = iniFilePath.baseName if (iniFilePath is not None) else None
        numOfInis = len(graphGroups)

        for i in range(numOfInis):
            if (i > 0 and iniFilePath is not None):
                iniFilePath.baseName = f"{iniBaseName}{FileSuffixes.RemapFixCopy.value}{i}"

            currentFile = i if (iniFilePath is None) else iniFilePath.path
            result[currentFile] = graphGroups[i]

        return result
    
    def _fix(self, keepBackup: bool = True, fixOnly: bool = False, hideOrig: bool = False, withBoilerPlate: bool = True, withSrc: bool = True):
        uncommentedTxt = ""
        if (hideOrig):
            uncommentedTxt = self._fileTxt
            self._iniFile.hideOriginalSections()

        if (keepBackup and fixOnly and self._iniFile.filePath is not None and os.path.exists(self._iniFile.filePath.path)):
            self._iniFile.print("log", "Cleaning up and disabling the OLD STINKY ini")
            self._iniFile.disIni()

        result = self.getFix()

        for file in result:
            result[file] = result[file].toStr()

            if (withBoilerPlate):
                result[file] = self._iniFile.addFixBoilerPlate(fix = result[file])

            if (withSrc):
                result[file] = f"{self._iniFile.fileTxt}\n\n{result[file]}"

            if (isinstance(file, int)):
                continue

            with open(file, "w", encoding = FileEncodings.UTF8.value) as f:
                f.write(result[file])

        if (hideOrig):
            self._iniFile.fileTxt = uncommentedTxt

        self._iniFile._isFixed = True
        return result
##### EndScript
