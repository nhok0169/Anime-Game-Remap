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
import shutil
from typing import Optional, List, Set, Union, Dict, Callable, Any
##### EndExtImports

##### LocalImports
from ..constants.FileExt import FileExt
from ..constants.FileTypes import FileTypes
from ..constants.FilePrefixes import FilePrefixes
from ..constants.FileSuffixes import FileSuffixes
from ..constants.DownloadMode import DownloadMode
from ..exceptions.RemapMissingBlendFile import RemapMissingBlendFile
from .strategies.ModType import ModType
from .Model import Model
from .files.BlendFile import BlendFile
from .files.PositionFile import PositionFile
from .files.TextureFile import TextureFile
from ..tools.files.FileService import FileService
from ..tools.ListTools import ListTools
from .files.IniFile import IniFile
from .stats.FileStats import FileStats
from .stats.CachedFileStats import CachedFileStats
from .stats.RemapStats import RemapStats
from .iniresources.IniFixResourceModel import IniFixResourceModel
from .iniresources.IniSrcResourceModel import IniSrcResourceModel
from ..constants.GlobalClassifiers import GlobalClassifiers
from .iniresources.IniTexModel import IniTexModel
from .iniresources.IniDownloadModel import IniDownloadModel
from .strategies.texEditors.BaseTexEditor import BaseTexEditor
from ..view.Logger import Logger
##### EndLocalImports


##### Script
class Mod(Model):
    """
    This Class inherits from :class:`Model`

    Used for handling a mod

    .. note::
        We define **a mod** based off the following criteria:

        * A folder that contains at least 1 .ini file
        * At least 1 of the .ini files in the folder contains:

            * a section with the regex ``[TextureOverride.*Blend]`` if :attr:`RemapService.readAllInis` is set to ``True`` or the script is ran with the ``--all`` flag :raw-html:`<br />`  :raw-html:`<br />` **OR** :raw-html:`<br />` :raw-html:`<br />`
            * a section that meets the criteria of one of the mod types defined :attr:`Mod._types` by running the mod types' :meth:`ModType.isType` function

        :raw-html:`<br />`
        See :class:`ModTypes` for some predefined types of mods
        
    Parameters
    ----------
    path: Optional[:class:`str`]
        The file location to the mod folder. :raw-html:`<br />` :raw-html:`<br />`
        
        If this value is set to ``None``, then will use the current directory of where this module is loaded.
        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    files: Optional[List[:class:`str`]]
        The direct children files to the mod folder (does not include files located in a folder within the mod folder). :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is set to ``None``, then the class will search the files for you when the class initializes :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    logger: Optional[:class:`Logger`]
        The logger used to pretty print messages :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    types: Optional[Set[:class:`ModType`]]
        The types of mods this mod should be. :raw-html:`<br />` :raw-html:`<br />` 
        If this argument is empty or is ``None``, then all the .ini files in this mod will be parsed :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    remappedTypes: Optional[Set[:class:`ModType`]]
        The types of mods to the mods specified at :attr:`Mod._types` will be fixed to.

        .. note::
            For more details, see :attr:`RemapService.remappedTypes`

        **Default**: ``None``

    defaultType: Optional[:class:`ModType`]
        The type of mod to use if a mod has an unidentified type :raw-html:`<br />` :raw-html:`<br />`
        If this argument is ``None``, then will skip the mod with an identified type :raw-html:`<br />` :raw-html:`<br />` 

        **Default**: ``None``

    forcedType: Optional[:class:`ModType`]
        The type of mod to forcibly assume for some .ini file :raw-html:`<br />` :raw-html:`<br />` 

        **Default**: ``None``

    version: Optional[:class:`float`]
        The game version we want the fixed mod :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will fix the mod to using the latest hashes/indices.

    downloadMode: :class:`DownloadMode`
        The download mode to handle file downloads :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            For more information about the available download modes to specify, see :ref:`Download Modes`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`DownloadMode.Normal`

    Attributes
    ----------
    path: Optional[:class:`str`]
        The file location to the mod folder

    version: Optional[:class:`float`]
        The game version we want the fixed mod

    downloadMode: :class:`DownloadMode`
        The download mode to handle file downloads :raw-html:`<br />`

        .. note::
            For more information about the available download modes to specify, see :ref:`Download Modes`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: :attr:`DownloadMode.SoftTexDriven`

    _files: List[:class:`str`]
        The direct children files to the mod folder (does not include files located in a folder within the mod folder).

    _types: Set[:class:`ModType`]
        The types of mods this mod should be

    _remappedType: Set[:class:`str`]
        The types of mods to the mods specified at :attr:`Mod.types` will be fixed to.

        .. note::
            For more details, see :attr:`RemapService.remappedTypes`

    _defaultType: Optional[:class:`ModType`]
        The type of mod to use if a mod has an unidentified type

    _forcedType: Optional[:class:`ModType`]
        The type of mod to forcibly assume for some .ini file

    logger: Optional[:class:`Logger`]
        The logger used to pretty print messages

    inis: Dict[:class:`str`, :class:`IniFile`]
        The .ini files found for the mod :raw-html:`<br />` :raw-html:`<br />`

        The keys are the file paths to the .ini file

    remapBlend: List[:class:`str`]
        The RemapBlend.buf files found for the mod

    backupInis: List[:class:`str`]
        The RemapBKUP.txt files found for the mod

    remapCopies: Dict[:class:`str`, :class:`IniFile`]
        The *remapFix*.ini files found for the mod

        The keys are the file paths to the .ini file

    remapTextures: List[:class:`str`]
        The *remapFix*.dds files found for the mod
    """
    def __init__(self, path: Optional[str] = None, files: Optional[List[str]] = None, logger: Optional[Logger] = None, types: Optional[Set[ModType]] = None, 
                 forcedType: Optional[ModType] = None, defaultType: Optional[ModType] = None, version: Optional[float] = None, remappedTypes: Optional[Set[str]] = None,
                 downloadMode: DownloadMode = DownloadMode.HardTexDriven):
        super().__init__(logger = logger)
        self.path = FileService.getPath(path)
        self.version = version
        self.downloadMode = downloadMode
        self._files = files

        if (types is None):
            types = set()
        if (remappedTypes is None):
            remappedTypes = set()

        self._types = types
        self._remappedTypes = remappedTypes
        self._defaultType = defaultType
        self._forcedType = forcedType

        self._optFileClassifier = GlobalClassifiers.ModOptFiles.value

        self.inis = []
        self.remapBlend = []
        self.backupInis = []
        self.remapCopies = []
        self.groupedRemapCopies = {}
        self._setupFiles()

    @property
    def files(self):
        """
        The direct children files to the mod folder (does not include files located in a folder within the mod folder).

        :getter: Returns the files to the mod
        :setter: Sets up the files for the mod
        :type: Optional[List[:class:`str`]]
        """

        return self._files

    @files.setter
    def files(self, newFiles: Optional[List[str]] = None):
        self._files = newFiles
        self._setupFiles()

    def createIniFile(self, iniPath: str) -> IniFile:
        """
        Creates a new .ini file given the file path

        Parameters
        ----------
        iniPath: :class:`str`
            The file path to the .ini file

        Returns
        -------
        :class:`IniFile`
            The new object representing the .ini file
        """

        return IniFile(iniPath, logger = self.logger, modTypes = self._types, defaultModType = self._defaultType, 
                       forcedModType = self._forcedType, version = self.version, modsToFix = self._remappedTypes, downloadMode = self.downloadMode)
    
    def getOrigIniPath(self, remapCopyPath: str) -> str:
        """
        Retrieves the file path to the original .ini file for some RemapFix.ini file

        Parameters
        ----------
        remapCopyPath: :class:`str`
            The file path to the RemapFix.ini file

        Returns
        -------
        :class:`str`
            The file path to the corresponding .ini file
        """

        folder = os.path.dirname(remapCopyPath)
        basename = os.path.basename(remapCopyPath)

        remapCopyBaseParts = basename.rsplit(FileSuffixes.RemapFixCopy.value, 1)
        remapCopyBasePartsLen = len(remapCopyBaseParts)

        if (remapCopyBasePartsLen == 1):
            return os.path.join(folder, remapCopyBaseParts[0])
        
        ext = remapCopyBaseParts[-1]
        extPos = ext.find(".")
        if (extPos > 0):
            remapCopyBaseParts[-1] = ext[extPos:]

        basename = "".join(remapCopyBaseParts)
        return os.path.join(folder, basename)

    def _setupFiles(self):
        """
        Searches the direct children files to the mod folder if :attr:`Mod.files` is set to ``None``        
        """

        if (self._files is None):
            self._files = FileService.getFiles(path = self.path)

        self.inis, self.backupInis, self.remapCopies = self.getOptionalFiles()

        iniPaths = self.inis
        self.inis = {}
        for iniPath in iniPaths:
            iniFile = self.createIniFile(iniPath)
            self.inis[iniPath] = iniFile

        iniPaths = self.remapCopies
        self.remapCopies = {}
        for iniPath in iniPaths:
            iniFile = self.createIniFile(iniPath)
            self.remapCopies[iniPath] = iniFile

            origIniFile = self.getOrigIniPath(iniPath)

            remapCopies = self.groupedRemapCopies.get(origIniFile)
            if (remapCopies is None):
                remapCopies = []
                self.groupedRemapCopies[origIniFile] = remapCopies

            remapCopies.append(iniPath)

    @classmethod
    def isIni(cls, file: str) -> bool:
        """
        Determines whether the file is a .ini file which is the file used to control how a mod behaves

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a .ini file
        """

        return file.endswith(FileExt.Ini.value)
    
    @classmethod
    def isSrcIni(cls, file: str) -> bool:
        """
        Determines whether the file is a .ini file that is not created by this fix

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a .ini file not created by this fix
        """

        fileBaseName = os.path.basename(file)
        return (cls.isIni(file) and fileBaseName.find(FileSuffixes.RemapFixCopy.value) == -1)
    
    @classmethod
    def isRemapBlend(cls, file: str) -> bool:
        """
        Determines whether the file is a RemapBlend.buf file which is the fixed Blend.buf file created by this fix

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a RemapBlend.buf file
        """

        baseName = os.path.basename(file)
        if (not baseName.endswith(FileExt.Buf.value)):
            return False

        baseName = baseName.rsplit(".", 1)[0]
        baseNameParts = baseName.rsplit("RemapBlend", 1)

        return (len(baseNameParts) > 1)
    
    @classmethod
    def isBlend(cls, file: str) -> bool:
        """
        Determines whether the file is a Blend.buf file which is the original blend file provided in the mod

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a Blend.buf file
        """

        return bool(file.endswith(FileTypes.Blend.value) and not cls.isRemapBlend(file))
   
    @classmethod
    def isBackupIni(cls, file: str) -> bool:
        """
        Determines whether the file is a RemapBKUP.txt file that is used to make
        backup copies of .ini files

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a RemapBKUP.txt file
        """

        fileBaseName = os.path.basename(file)
        return (fileBaseName.startswith(FilePrefixes.BackupFilePrefix.value) or fileBaseName.startswith(FilePrefixes.OldBackupFilePrefixV3.value) or fileBaseName.startswith(FilePrefixes.OldBackupFilePrefixV4_3.value)) and file.endswith(FileExt.Txt.value)
    
    @classmethod
    def isRemapCopyIni(cls, file: str) -> bool:
        """
        Determines whether the file is *RemapFix*.ini file which are .ini files generated by this fix to remap specific type of mods :raw-html:`<br />` :raw-html:`<br />`

        *eg. mods such as Keqing or Jean that are fixed by :class:`GIMIObjMergeFixer` *

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a *RemapFix*.ini file
        """

        fileBaseName = os.path.basename(file)
        return (cls.isIni(file) and fileBaseName.rfind(FileSuffixes.RemapFixCopy.value) > -1)
    
    @classmethod
    def isRemapTexture(cls, file: str) -> bool:
        """
        Determines whether the file is a *RemapTex*.dds file which are texture .dds files generated by this fix to edit a particular texture file for some specific type of mods :raw-html:`<br />` :raw-html:`<br />`

        *eg. mods such as Kirara or Nilou that are fixed by :class:`GIMIRegEditFixer` *

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a *RemapTex*.dds file
        """

        return bool(file.endswith(FileTypes.RemapTexture.value)) 

    def getOptionalFiles(self) -> List[List[str]]:
        """
        Retrieves a list of each type of files that are not mandatory for the mod

        Returns
        -------
        [ List[:class:`str`], List[:class:`str`], List[:class:`str`]]
            The resultant files found for the following file categories (listed in the same order as the return type):

            #. .ini files not created by this fix
            #. RemapBKUP.txt files
            #. RemapFix.ini files

            .. note::
                See :meth:`Mod.isIni`, :meth:`Mod.isBackupIni`, :meth:`Mod.isRemapCopyIni` for the specifics of each type of file
        """

        resultIni = []
        resultBackup = []
        resultCopy = []

        if (not self._optFileClassifier.isSetup):
            self._optFileClassifier.setup({FileExt.Ini.value: "isIni", 
                                           FileExt.Txt.value: "isTxt", 
                                           FilePrefixes.BackupFilePrefix.value: "isBackup", 
                                           FilePrefixes.OldBackupFilePrefixV3.value: "isBackup",
                                           FilePrefixes.OldBackupFilePrefixV4_3.value: "isBackup",
                                           FileSuffixes.RemapFixCopy.value: "isCopy"})
            
        backupFilePrefixes = {FilePrefixes.BackupFilePrefix.value, FilePrefixes.OldBackupFilePrefixV3.value, FilePrefixes.OldBackupFilePrefixV4_3.value}
            
        for file in self._files:
            basename = os.path.basename(file)
            basenameLen = len(basename)

            searchResult = self._optFileClassifier.dfa.findAll(basename)
            if (not searchResult):
                continue

            searchResultLen = len(searchResult)
            extKey = None

            if (FileExt.Ini.value in searchResult):
                extKey = FileExt.Ini.value
            elif (FileExt.Txt.value in searchResult):
                extKey = FileExt.Txt.value

            if (extKey is None or searchResult[extKey][-1][1] != basenameLen):
                continue

            if (searchResultLen == 1 and extKey == FileExt.Ini.value):
                resultIni.append(file)

            if (searchResultLen == 1):
                continue

            foundBackupFilePrefixes = backupFilePrefixes.intersection(set(searchResult.keys()))

            if (foundBackupFilePrefixes):
                resultBackup.append(file)
            elif (FileSuffixes.RemapFixCopy.value in searchResult):
                resultCopy.append(file)

        return [resultIni, resultBackup, resultCopy]
    
    # _removeFileType(fileTypeAtt, logFunc): Removes all the files for a particular file type for the mod
    def _removeFileType(self, fileTypeAtt: str, logFunc: Callable[[str], str]):
        files = getattr(self, fileTypeAtt)

        for file in files:
            logTxt = logFunc(file)
            self.print("log", logTxt)
            try:
                os.remove(file)
            except FileNotFoundError:
                pass
    
    def removeBackupInis(self):
        """
        Removes all RemapBKUP.txt contained in the mod
        """

        self._removeFileType("backupInis", lambda file: f"Removing the backup ini, {os.path.basename(file)}")

    def _removeIniResources(self, ini: IniFile, result: Set[str], resourceName: str, resourceStats: FileStats, getPathsToRemove: Callable[[IniFile], List[str]]) -> bool:
        """
        Removes a particular type of resource from a .ini file

        Parameters
        ----------
        ini: :class:`IniFile`
            The particular .ini file to be processed

        result: Set[:class:`str`]
            The resultant paths to the resources that got removed

        resourceName: :class:`str`
            The name of the type of resource

        resourceStats: :class:`FileStats`
            The associated statistical data for the resource type

        getPathsToRemove: Callable[[:class:`IniFile`], List[:class:`str`]]
            The function to file paths to remove for a particular type of resource

        Returns
        -------
        :class:`bool`
            Whether there was a file that was attempted to be removed
        """

        paths = getPathsToRemove(ini)
        hasRemovedResource = False

        for path in paths:
            if (path not in resourceStats.fixed and path not in resourceStats.visitedAtRemoval):
                try:
                    os.remove(path)
                except FileNotFoundError as e:
                    self.print("log", f"No Previous {resourceName} found at {path}")
                else:
                    self.print("log", f"Removing previous {resourceName} at {path}")
                    result.add(path)
                
                resourceStats.addVisitedAtRemoval(path)

                if (not hasRemovedResource):
                    hasRemovedResource = True

        return hasRemovedResource

    def _getIniFixResourceFixPaths(self, iniFixResources: List[IniFixResourceModel]) -> List[str]:
        result = set()
        for model in iniFixResources:
            for fixedPath, fixedFullPath, origPath, origFullPath in model:
                result.add(fixedFullPath)

        return list(result)
    
    def _getIniSrcResourcePaths(self, iniSrcResources: List[IniSrcResourceModel]) -> List[str]:
        result = set()
        for model in iniSrcResources:
            for path, fullPath in model:
                result.add(fullPath)

        return list(result)
    
    def _removeIniFix(self, ini: IniFile, remapStats: RemapStats, removedRemapBlends: Set[str], removedRemapPositions: Set[str], 
                      removedTextures: Set[str], removedDownloads: Set[str], undoedInis: Set[str],
                      keepBackups: bool = True, fixOnly: bool = False, readAllInis: bool = False, writeBackInis: bool = True) -> bool:
        iniFilesUndoed = False
        iniFullPath = None
        iniHasErrors = False
        iniStats = remapStats.ini

        if (ini.file is not None):
            iniFullPath = FileService.absPathOfRelPath(ini.file, self.path)

        # remove the fix from the .ini files
        if (iniFullPath is None or (iniFullPath not in iniStats.fixed and iniFullPath not in iniStats.skipped and (ini.isModIni or readAllInis))):
            try:
                ini.removeFix(keepBackups = keepBackups, fixOnly = fixOnly, parse = True, writeBack = writeBackInis)
            except Exception as e:
                iniStats.addSkipped(iniFullPath, e, modFolder = self.path)
                iniHasErrors = True
                self.print("handleException", e)

            if (not iniHasErrors and iniFullPath is not None):
                undoedInis.add(iniFullPath)

            if (not iniFilesUndoed):
                iniFilesUndoed = True

        # remove only the remap blends that have not been recently created
        self._removeIniResources(ini, removedRemapBlends, FileTypes.RemapBlend.value, remapStats.blend, lambda iniFile: self._getIniFixResourceFixPaths(list(iniFile.remapBlendModels.values())))

        # remove only the remap positions that have not been recently created
        self._removeIniResources(ini, removedRemapPositions, FileTypes.Position.value, remapStats.position, lambda iniFile: self._getIniFixResourceFixPaths(list(iniFile.remapPositionModels.values())))

        # remove only the remap texture files that have not been recently created
        self._removeIniResources(ini, removedTextures, FileTypes.RemapTexture.value, remapStats.texAdd, lambda iniFile: self._getIniFixResourceFixPaths(iniFile.getTexAddModels()))

        # remove only the download files that have not been recently created
        downloadsRemoved = self._removeIniResources(ini, removedDownloads, FileTypes.RemapDownload.value, remapStats.download, lambda iniFile: self._getIniSrcResourcePaths(list(iniFile.fileDownloadModels.values())))
        if (downloadsRemoved):
            self.print("space")

        return iniFilesUndoed

    def removeFix(self, remapStats: RemapStats, keepBackups: bool = True, fixOnly: bool = False, readAllInis: bool = False, writeBackInis: bool = True) -> List[Set[str]]:
        """
        Removes any previous changes done by this module's fix

        Parameters
        ----------
        remapStats: :class:`RemapStats`
            The stats for the remap process

        keepBackups: :class:`bool`
            Whether to create or keep RemapBKUP.txt files in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to not undo any changes created in the .ini files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        readAllInis: :class:`bool`
            Whether to remove the .ini fix from all the .ini files encountered :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        writeBackInis: :class:`bool`
            Whether to write back the changes to the .ini files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        [Set[:class:`str`], Set[:class:`str`], Set[:class:`str`], Set[:class:`str`], Set[:class:`str`]]
            The removed files that have their fix removed, where the types of files for the return value is based on the list below:

            #. .ini files with their fix removed
            #. RemapBlend.buf files that got deleted
            #. RemapPosition.buf files that got deleted
            #. RemapTex.dds files that got deleted
            #. Download files that got deleted
        """

        removedRemapBlends = set()
        removedRemapPositions = set()
        removedTextures = set()
        removedDownloads = set()
        undoedInis = set()
        undoedRemapCopies = set()

        for iniPath in self.inis:
            ini = self.inis[iniPath]

            iniFileUndoed = self._removeIniFix(ini, remapStats, removedRemapBlends, removedRemapPositions, removedTextures, removedDownloads, undoedInis,
                                               keepBackups = keepBackups, fixOnly = fixOnly, readAllInis = readAllInis, writeBackInis = writeBackInis)
            
            if (not iniFileUndoed or iniPath not in self.groupedRemapCopies):
                continue
            
            # remove the remap copies associated to the .ini file
            remapCopiesRemoved = False
            for remapCopyPath in self.groupedRemapCopies[iniPath]:
                remapCopy = self.remapCopies[remapCopyPath]
                remapCopy.classify()

                self._removeIniFix(remapCopy, remapStats, removedRemapBlends, removedRemapPositions, removedTextures, removedDownloads, undoedRemapCopies,
                                   keepBackups = False, fixOnly = fixOnly, readAllInis = readAllInis, writeBackInis = writeBackInis)

                if (remapCopy.file is None):
                    continue

                try:
                    os.remove(remapCopy.file)
                except FileNotFoundError:
                    pass
                else:
                    self.print("log", f"Removing the .ini remap copy, {os.path.basename(remapCopy.file)}")
                    
                    if (not remapCopiesRemoved):
                        remapCopiesRemoved = True

            if (remapCopiesRemoved):
                self.print("space")

        return [undoedInis, removedRemapBlends, removedRemapPositions, removedTextures, removedDownloads]

    @classmethod
    def blendCorrection(cls, blendFile: Union[str, bytes], modType: ModType, modToFix: str, 
                        fixedBlendFile: Optional[str] = None, version: Optional[float] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Blend.buf file

        See :meth:`BlendFile.remap` for more info

        Parameters
        ----------
        blendFile: Union[:class:`str`, :class:`bytes`]
            The file path to the Blend.buf file to fix

        modType: :class:`ModType`
            The type of mod to fix from

        modToFix: :class:`str`
            The name of the mod to fix to

        fixedBlendFile: Optional[:class:`str`]
            The file path for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        version: Optional[float]
            The game version to fix to :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will fix to the latest game version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`BufFileNotRecognized`
            If the original Blend.buf file provided by the parameter ``blendFile`` cannot be read

        :class:`BadBufData`
            If the bytes passed into this function do not correspond to the format defined for a Blend.buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedBlendFile`` is ``None``, then will return an array of bytes for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed RemapBlend.buf file if the provided Blend.buf file got corrected
        """

        blend = BlendFile(blendFile)
        vgRemap = modType.getVGRemap(modToFix, version = version)
        return blend.remap(vgRemap = vgRemap, fixedBlendFile = fixedBlendFile)
    
    @classmethod
    def positionCorrection(cls, positionFile: Union[str, bytes], modType: ModType, modToFix: str,
                           fixedPositionFile: Optional[str] = None, version: Optional[float] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Position.buf file

        Parameters
        ----------
        positionFile: Union[:class:`str`, :class:`bytes`]
            The file path to the Position.buf file to fix

        modType: :class:`ModType`
            The type of mod to fix from

        modToFix: :class:`str`
            The name of the mod to fix to

        fixedPositionFile: Optional[:class:`str`]
            The file path for the fixed Position.buf file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        version: Optional[float]
            The game version to fix to :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will fix to the latest game version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`BufFileNotRecognized`
            If the original Position.buf file provided by the parameter ``positionFile`` cannot be read

        :class:`BadBufData`
            If the bytes passed into this function do not correspond to the format defined for a Position.buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedPositionFile`` is ``None``, then will return an array of bytes for the fixed Position.buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed RemapPosition.buf file if the provided Position.buf file got corrected
        """

        
        position = PositionFile(positionFile)
        positionEditor = modType.getPositionEditor(modToFix, version = version)
        return positionEditor.fix(position, fixedBufFile = fixedPositionFile)
    
    @classmethod
    def _texCorrection(cls, fixedTexFile: str, modToFix: str, model: IniTexModel, partInd: int, pathInd: int, texFile: Optional[str] = None) -> str:
        texEditor = model.texEdits[partInd][modToFix][pathInd]
        if (texFile is None):
            texFile = fixedTexFile

        result = cls.texCorrection(fixedTexFile, texEditor, texFile = texFile)
        if (result is None):
            raise FileNotFoundError(f"Cannot find texture file at {texFile}")
        
        return result
    
    @classmethod
    def texCorrection(cls, fixedTexFile: str, texEditor: BaseTexEditor, texFile: Optional[str] = None) -> Optional[str]:
        """
        Fixes a .dds file

        Parameters
        ----------
        fixedTexFile: :class:`str`
            The name of the file path to the fixed RemapTex.dds file

        texEditor: :class:`BaseTexEditor`
            The texture editor to change the texture file

        texFile Optional[:class:`str`]
            The file path to the original texture .dds file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use 'fixedTexFile' as the original file path to the texture .dds file 
            (usually this case for creating a brand new .dds file by also passing in object of type :class:`TexCreator` into the 'texEditor' argument) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Optional[:class:`str`]
            The file path to the fixed texture, if the original texture exists
        """
        if (texFile is None):
            texFile = fixedTexFile

        tex = TextureFile(texFile)
        texEditor.fix(tex, fixedTexFile)

        if (tex.img is None):
            return None
        return fixedTexFile

    def _downloadFile(self, downloadPath: str, model: IniDownloadModel, partInd: int, pathInd: int, downloadStats: CachedFileStats, proxy: Optional[str] = None) -> str:
        download = model.downloads[partInd][pathInd]
        downloadFolder = os.path.dirname(downloadPath)

        rawDownloadFullPath, downloaded, downloadExisted =  download.get(downloadFolder, proxy = proxy)

        if (downloadPath != rawDownloadFullPath):
            shutil.move(rawDownloadFullPath, downloadPath)

        if (downloaded):
            downloadStats.addFixed(downloadPath)
            self.print("log", f"Download successful at {downloadPath}")
        else:
            downloadStats.addHit(downloadPath)
            self.print("log", f"Copied previous download to {downloadPath}")

    def correctResource(self, resourceStats: FileStats, getResourceModels: Callable[[IniFile], List[IniFixResourceModel]], 
                        correctFile: Callable[[str, str, ModType, str, int, int, int, IniFixResourceModel, FileStats], str], 
                        iniPaths: Optional[List[str]] = None, fileTypeName: str = "", 
                        needsSrcFile: bool = True, fixOnly: bool = False,
                        newTranslations: Optional[Dict[str, Callable[[List[str]], Any]]] = None) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the files for a particular type of resource referenced by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        resourceStats: :class:`FileStats`
            The stats to keep track of whether the particular resource has been fixed or skipped

        getResourceModels: Callable[[:class:`IniFile`], List[:class:`IniFixResourceModel`]]
            Function to retrieve all of the needed :class:`IniFixResourceModel` from some .ini file

        correctFile: Callable[[:class:`str`, :class:`str`, :class:`ModType`, :class:`str`, :class:`int`, :class:`int`, :class:`int`, :class:`IniFixResourceModel`, :class:`FileStats`], :class:`str`]
            Function to fix up the resource file :raw-html:`<br />` :raw-html:`<br />`

            The parameters for the function are as follows:

            #. The full file path to the original resource
            #. The fixed file path to the resource
            #. The type of mod being fixed within the .ini file
            #. The name of the mod to fix to
            #. The index of the part within the :class:`IfTemplate`
            #. The index of the path within the particular part of the :class:`IfTemplate`
            #. The version of the game to fix to
            #. The current :class:`IniFixResourceModel` being processed
            #. The stats for the particular resource

            :raw-html:`<br />` :raw-html:`<br />`

            The function returns a :class:`str` with the fixed file path to the resource

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have their resources corrected. If this value is ``None``, then will correct all the .ini file in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fileTypeName: :class:`str`
            The name of the file resource

        fixOnly: :class:`bool`
            Whether to not correct some resource file if its corresponding fixed resource file already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        newTranslations: Optional[Dict[:class:`str`, Callable[[...], Any]]]
            Event handlers to print output based on some event. :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the events and the values are the handlers.

            The argument supports the following event handlers:

            .. list-table::
                :widths: 20 40 40
                :header-rows: 1

                * - Event Name
                  - Parameters
                  - Description
                * - **missingOrig**
                  - | origFullPath: :class:`str`
                    |   The full path to the source file to fix
                  - When the source file to fix is not found
                * - **origAlreadyError**
                  - | origFullPath: :class:`str`
                    |   The full path to the source file to fix
                  - When the source file to fix had already encountered an error
                * - **fixedAlreadyFixed**
                  - | fixedFullPath: :class:`str`
                    |   The full path to the fixed file
                  - When the file to fix has already been fixed
                * - **fixedAlreadyExists**
                  - | fixedFullPath: :class:`str`
                    |   The full path to the fixed file
                  - When the file to fix has already encountered an error
                * - **noCorrectionNeeded**
                  - | origFullPath: :class:`str`
                    |   The full path to the source file to fix               
                  - When no correction is needed to be done
                * - **correctionDone**
                  - | fixedFullPath: :class:`str`
                    |   The full path to the fixed file
                  - When the correction has been done to the fixed file
                * - **onIniFirstCorrection**
                  - | fixedFullPath: :class:`str`
                    |   The full path to the fixed file
                  - When handling the first file for a particular .ini file
                * - **handleError**
                  - | error: :class:`Exception`
                    |   The error that occured when trying to fix some resource
                  - When an error occurs during the correction of a file
                * - **iniSpace**
                  - | iniPath: :class:`str`
                    |   The path to the .ini file
                  - When printing out a seperator between .ini files

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the fixed resource files that were fixed
            #. The exceptions encountered when trying to fix some fixed resource files :raw-html:`<br />` :raw-html:`<br />`

               The keys are absolute filepath to the fixed resource file and the values are the exception encountered
        """

        if (newTranslations is None):
            newTranslations = {}

        translations =  {"missingOrig": lambda fixedFullPath: self.print("log", f"Missing Original {fileTypeName} for the {fileTypeName} file at {fixedFullPath}"),
                         "origAlreadyError": lambda origFullPath: self.print("log", f"{fileTypeName} has already previously encountered an error at {origFullPath}"),
                         "fixedAlreadyFixed": lambda fixedFullPath: self.print("log", f"{fileTypeName} has already been corrected at {fixedFullPath}"),
                         "fixedAlreadyError": lambda fixedFullPath: self.print("log", f"{fileTypeName} has already previously encountered an error at {fixedFullPath}"),
                         "fixedAlreadyExists": lambda fixedFullPath: self.print("log", f"{fileTypeName} was previously fixed at {fixedFullPath}"),
                         "noCorrectionNeeded": lambda origFullPath: self.print("log", f"{fileTypeName} does not need to be corrected at {origFullPath}"),
                         "correctionDone": lambda fixedFullPath: self.print("log", f'{fileTypeName} correction done at {fixedFullPath}'),
                         "onIniFirstCorrection": lambda iniPath: self.print("log", f"Fixing the {fileTypeName} files for {os.path.basename(iniPath)}..."),
                         "handleError": lambda error: self.print("handleException", error),
                         "iniSpace": lambda iniPath: self.print("space")}
        
        translations.update(newTranslations)

        currentBlendsSkipped = {}
        currentBlendsFixed = set()
        fileTypeName = "file" if (fileTypeName == "") else f"{fileTypeName} file"
        correctionDone = False

        if (iniPaths is None):
            iniPaths = list(self.inis.keys())
        else:
            iniPaths = ListTools.getDistinct(iniPaths, keepOrder = True)

        iniPathsLen = len(iniPaths)
        for iniInd in range(iniPathsLen):
            iniPath = iniPaths[iniInd]
            ini = None
            try:
                ini = self.inis[iniPath]
            except KeyError:
                continue

            if (ini is None):
                continue
            
            modType = ini.availableType
            if (modType is None):
                continue

            resourceModels = getResourceModels(ini)
            iniLogged = False

            for model in resourceModels:
                for partIndex, partFullPaths in model.fullPaths.items():
                    for modName, fixedFullPaths in partFullPaths.items():

                        fixedFullPathsLen = len(fixedFullPaths)
                        for i in range(fixedFullPathsLen):
                            fixedFullPath = fixedFullPaths[i]
                            origFullPath = None

                            if (needsSrcFile):
                                try:
                                    origFullPath = model.origFullPaths[partIndex][i]
                                except KeyError:
                                    if (not correctionDone):
                                        translations["onIniFirstCorrection"](iniPath)
                                        correctionDone = True

                                    translations["missingOrig"](fixedFullPath)
                                    iniLogged = True

                                    if (fixedFullPath not in resourceStats.skipped):
                                        error = RemapMissingBlendFile(fixedFullPath)
                                        currentBlendsSkipped[fixedFullPath] = error
                                        resourceStats.addSkipped(fixedFullPath, error, modFolder = self.path)
                                    break

                            # check if the file was already encountered and did not need to be fixed
                            if (origFullPath is not None and origFullPath in resourceStats.fixed):
                                break

                            if (not correctionDone):
                                translations["onIniFirstCorrection"](iniPath)
                                correctionDone = True

                            if (not iniLogged):
                                iniLogged = True
                            
                            # check if the file that did not need to be fixed already had encountered an error
                            if (origFullPath is not None and origFullPath in resourceStats.skipped):
                                translations["origAlreadyError"](origFullPath)
                                break
                            
                            # check if the file has been fixed
                            if (fixedFullPath in resourceStats.fixed):
                                translations["fixedAlreadyFixed"](fixedFullPath)
                                continue

                            # check if the file already had encountered an error
                            if (fixedFullPath in resourceStats.skipped):
                                translations["fixedAlreadyError"](fixedFullPath)
                                continue

                            # check if the fixed file already exists and we only want to fix mods without removing their previous fixes
                            if (fixOnly and os.path.isfile(fixedFullPath)):
                                translations["fixedAlreadyExists"](fixedFullPath)
                                continue
                            
                            # fix the file resource
                            correctedResourcePath = None
                            try:
                                correctedResourcePath = correctFile(origFullPath, fixedFullPath, modType, modName, partIndex, i, self.version, model, resourceStats)
                            except Exception as e:
                                currentBlendsSkipped[fixedFullPath] = e
                                resourceStats.addSkipped(fixedFullPath, e, modFolder = self.path)
                                translations["handleError"](e)
                            else:
                                pathToAdd = ""
                                if (correctedResourcePath is None):
                                    translations["noCorrectionNeeded"](origFullPath)
                                    pathToAdd = origFullPath
                                else:
                                    translations["correctionDone"](fixedFullPath)
                                    pathToAdd = fixedFullPath

                                currentBlendsFixed.add(pathToAdd)
                                resourceStats.addFixed(pathToAdd)

            if (iniLogged and iniInd < iniPathsLen - 1):
                translations["iniSpace"](iniPath)

        return [currentBlendsFixed, currentBlendsSkipped]
    
    def handleSrcFiles(self, resourceStats: FileStats, getResourceModels: Callable[[IniFile], List[IniSrcResourceModel]], 
                       handleFile: Callable[[str, str, ModType, str, int, int, int, IniFixResourceModel, FileStats], str],
                       iniPaths: Optional[List[str]] = None, fileTypeName: str = "", 
                       fixOnly: bool = False,
                       newTranslations: Optional[Dict[str, Callable[[List[str]], Any]]] = None) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Downloads the required files for the mod

        Parameters
        ----------
        resourceStats: :class:`FileStats`
            The stats to keep track of a particular resource

        getResourceModels: Callable[[:class:`IniFile`], List[:class:`IniSrcResourceModel`]]
            Function to retrieve all of the needed :class:`IniSrcResourceModel` from some .ini file

        handleFile: Callable[[:class:`str`, :class:`ModType`, :class:`int`, :class:`int`, :class:`int`, :class:`IniFixResourceModel`, :class:`FileStats`], :class:`str`]
            Function to handle the resource file :raw-html:`<br />` :raw-html:`<br />`

            The parameters for the function are as follows:

            #. The full file path to the resource
            #. The type of mod being fixed within the .ini files
            #. The index of the part within the :class:`IfTemplate`
            #. The index of the path within the particular part of the :class:`IfTemplate`
            #. The version of the game to fix to
            #. The current :class:`IniSrcResourceModel` being processed
            #. The stats for the particular resource

            :raw-html:`<br />` :raw-html:`<br />`

            The function returns a :class:`str` with the fixed file path to the resource

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have files downloaded. If this value is ``None``, then will download files from all the .ini file in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fileTypeName: :class:`str`
            The name of the file resource

        fixOnly: :class:`bool`
            Whether to not correct some resource file if its corresponding fixed resource file already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        newTranslations: Optional[Dict[:class:`str`, Callable[[...], Any]]]
            Event handlers to print output based on some event. :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the events and the values are the handlers.

            The argument supports the following event handlers:

            .. list-table::
                :widths: 20 40 40
                :header-rows: 1

                * - Event Name
                  - Parameters
                  - Description
                * - **alreadyHandled**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When some file has already been handled
                * - **alreadyError**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When some file already encountered an error
                * - **alreadyExists**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When some file already exists
                * - **handled**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When some file has already been handled
                * - **skipped**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When skipping the handling of some file
                * - **correctionDone**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When the correction has been done to the fixed file
                * - **onIniFirstCorrection**
                  - | fullPath: :class:`str`
                    |   The full path to the file to handle
                  - When handling the first file for a particular .ini file
                * - **handleError**
                  - | error: :class:`Exception`
                    |   The error that occured when trying to fix some resource
                  - When an error occurs during the correction of a file
                * - **iniSpace**
                  - | iniPath: :class:`str`
                    |   The path to the .ini file
                  - When printing out a seperator between .ini files

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the files that were handled
            #. The exceptions encountered when trying to handle some file :raw-html:`<br />` :raw-html:`<br />`

               The keys are expected absolute filepath to the downloaded file and the values are the exception encountered
        """

        if (newTranslations is None):
            newTranslations = {}

        translations =  {"alreadyHandled": lambda fullPath: self.print("log", f"{fileTypeName} has already been handled at {fullPath}"),
                         "alreadyError": lambda fullPath: self.print("log", f"{fileTypeName} has already previously encountered an error at {fullPath}"),
                         "alreadyExists": lambda fullPath: self.print("log", f"{fileTypeName} was previously handled at {fullPath}"),
                         "handled": lambda fullPath: self.print("log", f'{fileTypeName} handled at {fullPath}'),
                         "skipped": lambda fullPath: self.print("log", f"{fileTypeName} was skipped at {fullPath}"),
                         "onIniFirstCorrection": lambda iniPath: self.print("log", f"Handling the {fileTypeName} files for {os.path.basename(iniPath)}..."),
                         "handleError": lambda error: self.print("handleException", error),
                         "iniSpace": lambda iniPath: self.print("space")}
        
        translations.update(newTranslations)

        currentResourcesSkipped = {}
        currentResourcesHandled = set()
        handled = False

        if (iniPaths is None):
            iniPaths = list(self.inis.keys())
        else:
            iniPaths = ListTools.getDistinct(iniPaths, keepOrder = True)

        iniPathsLen = len(iniPaths)
        for iniInd in range(iniPathsLen):
            iniPath = iniPaths[iniInd]
            if (iniPath not in self.inis):
                continue

            ini = self.inis[iniPath]
            if (ini is None):
                continue
            
            modType = ini.availableType
            if (modType is None):
                continue

            resourceModels = getResourceModels(ini)

            for model in resourceModels:
                for partIndex, partFullPaths in model.fullPaths.items():

                    partFullPathsLen = len(partFullPaths)
                    for i in range(partFullPathsLen):
                        fullPath = partFullPaths[i]

                        if (not handled):
                            translations["onIniFirstCorrection"](iniPath)
                            handled = True

                        # check if the file was already encountered and did not need to be fixed
                        if (fullPath is not None and fullPath in resourceStats.fixed):
                            translations["alreadyHandled"](fullPath)
                            continue

                        # check if the file that did not need to be fixed already had encountered an error
                        if (fullPath is not None and fullPath in resourceStats.skipped):
                            translations["alreadyError"](fullPath)
                            continue

                        # check if the fixed file already exists and we only want to fix mods without removing their previous fixes
                        if (fixOnly and os.path.isfile(fullPath)):
                            translations["alreadyExists"](fullPath)
                            continue

                        # download the resource 
                        handledResourcePath = None
                        try:
                            handledResourcePath = handleFile(fullPath, modType, partIndex, i, self.version, model, resourceStats)
                        except Exception as e:
                            currentResourcesSkipped[fullPath] = e
                            resourceStats.addSkipped(fullPath, e, modFolder = self.path)
                            translations["handleError"](e)
                        else:
                            if (handledResourcePath is not None):
                                currentResourcesHandled.add(fullPath)
                                resourceStats.addFixed(fullPath)
                                translations["handled"](fullPath)
                            else:
                                translations["skipped"](fullPath)

            if (iniInd < iniPathsLen - 1):
                translations["iniSpace"](iniPath)

        return [currentResourcesHandled, currentResourcesSkipped]
    
    def correctTex(self, texAddStats: FileStats, texEditStats: FileStats, iniPaths: Optional[List[str]] = None, fixOnly: bool = False) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the texture .dds files reference by the mods

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.fix` function

        Parameters
        ----------
        texAddStats: :class:`FileStats`
            The stats to keep track of whether the particular .dds file have been newly created or skipped

        texEditStats: :class:`FileStats`
            The stats to keep track of whether the particular .dds file has been editted or skipped

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have their .dds files corrected. If this value is ``None``, then will correct all the .ini file in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fixOnly: :class:`bool`
            Whether to not correct some .dds file if its corresponding RemapTex.dds already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`], Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the .dds files that were added
            #. The exceptions encountered when trying to created some .dds files 
            #. The absolute file paths of the .dds files that were editted
            #. The exceptions encountered when trying to edit some .dds files :raw-html:`<br />` :raw-html:`<br />`

            For the exceptions, the keys are absolute filepath to the .dds file and the values are the exception encountered        
        """

        fixedTexAdds, skippedTexAdds = self.correctResource(texAddStats, lambda iniFile: iniFile.getTexAddModels(), 
                                    lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniTexModel, resourceStats: self._texCorrection(fixedFullPath, modName, iniTexModel, partInd, pathInd, texFile = origFullPath),
                                    fileTypeName = "Texture", fixOnly = fixOnly, iniPaths = iniPaths,
                                    newTranslations = {"onIniFirstCorrection": lambda iniPath: self.print("log", f"Adding the {FileTypes.Texture.value} files for {os.path.basename(iniPath)}...")})
        
        fixedTexEdits, skippedTexEdits = self.correctResource(texEditStats, lambda iniFile: iniFile.getTexEditModels(), 
                                    lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniTexModel, resourceStats: self._texCorrection(fixedFullPath, modName, iniTexModel, partInd, pathInd, texFile = origFullPath),
                                    fileTypeName = "Texture", fixOnly = fixOnly, iniPaths = iniPaths,
                                    newTranslations = {"onIniFirstCorrection": lambda iniPath: self.print("log", f"Editting the {FileTypes.Texture.value} files for {os.path.basename(iniPath)}...")})
        
        return fixedTexAdds, skippedTexAdds, fixedTexEdits, skippedTexEdits
    
    def correctBlend(self, blendStats: FileStats, iniPaths: Optional[List[str]] = None, fixOnly: bool = False) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the Blend.buf files reference by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        blendStats: :class:`FileStats`
            The stats to keep track of whether the particular the blend.buf files have been fixed or skipped

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have their blend.buf files corrected. If this value is ``None``, then will correct all the .ini file in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fixOnly: :class:`bool`
            Whether to not correct some Blend.buf file if its corresponding RemapBlend.buf already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the RemapBlend.buf files that were fixed
            #. The exceptions encountered when trying to fix some RemapBlend.buf files :raw-html:`<br />` :raw-html:`<br />`

            The keys are absolute filepath to the RemapBlend.buf file and the values are the exception encountered
        """

        return self.correctResource(blendStats, lambda iniFile: iniFile.remapBlendModels.values(), 
                                    lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniResourceModel, resourceStats: self.blendCorrection(origFullPath, modType, modName, fixedBlendFile = fixedFullPath, version = version),
                                    fileTypeName = "Blend", fixOnly = fixOnly, iniPaths = iniPaths,
                                    newTranslations = {"onIniFirstCorrection": lambda iniPath: self.print("log", f"Fixing the {FileTypes.Blend.value} files for {os.path.basename(iniPath)}...")})
    
    def correctPosition(self, positionStats: FileStats, iniPaths: Optional[List[str]] = None, fixOnly: bool = False) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the Position.buf files reference by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        positionStats: :class:`FileStats`
            The stats to keep track of whether the particular the Position.buf files have been fixed or skipped

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have their Position.buf files corrected. If this value is ``None``, then will correct all the .ini file in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fixOnly: :class:`bool`
            Whether to not correct some Position.buf file if its corresponding RemapPosition.buf already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the RemapPosition.buf files that were fixed
            #. The exceptions encountered when trying to fix some RemapPosition.buf files :raw-html:`<br />` :raw-html:`<br />`

            The keys are absolute filepath to the RemapPosition.buf file and the values are the exception encountered
        """

        return self.correctResource(positionStats, lambda iniFile: iniFile.remapPositionModels.values(), 
                            lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniResourceModel, resourceStats: self.positionCorrection(origFullPath, modType, modName, fixedPositionFile = fixedFullPath, version = version),
                            fileTypeName = "Position", fixOnly = fixOnly, iniPaths = iniPaths,
                            newTranslations = {"onIniFirstCorrection": lambda iniPath: self.print("log", f"Fixing the {FileTypes.Position.value} files for {os.path.basename(iniPath)}...")})
    
    def downloadFiles(self, downloadStats: CachedFileStats, iniPaths: Optional[List[str]] = None, fixOnly: bool = False, proxy: Optional[str] = None) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Downloads the necessary files for a mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        downloadStats: :class:`CachedFileStats`
            The stats to keep track of the downloads

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have downloads required. If this value is ``None``, then will download files from all the .ini files in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fixOnly: :class:`bool`
            Whether to not download a file if the corresponding downloaded file already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the downloaded files
            #. The exceptions encountered when trying to download some files :raw-html:`<br />` :raw-html:`<br />`

            The keys are absolute filepath to the download file and the values are the exception encountered
        """

        return self.handleSrcFiles(downloadStats, lambda iniFile: iniFile.fileDownloadModels.values(),
                                   lambda fullPath, modType, partInd, pathInd, version, iniResourceModel, resourceStats: self._downloadFile(fullPath, iniResourceModel, partInd, pathInd, resourceStats, proxy = proxy),
                                   iniPaths = iniPaths, fileTypeName = "Download", fixOnly = fixOnly,
                                   newTranslations = {
                                    "alreadyHandled": lambda fullPath: self.print("log", f"Download has already been downloaded at {fullPath}"),
                                    "alreadyError": lambda fullPath: self.print("log", f"Download has already previously encountered an error at {fullPath}"),
                                    "alreadyExists": lambda fullPath: self.print("log", f"Download was previously downloaded at {fullPath}"),
                                    "handled": lambda fullPath: 0,
                                    "skipped": lambda fullPath: 0,
                                    "onIniFirstCorrection": lambda iniPath: self.print("log", f"Downloading the required files for {os.path.basename(iniPath)}...")})
##### EndScript