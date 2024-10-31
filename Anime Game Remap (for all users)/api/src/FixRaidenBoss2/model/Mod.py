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
from typing import Optional, List, Set, Union, Dict
##### EndExtImports

##### LocalImports
from ..constants.FileExt import FileExt
from ..constants.FileTypes import FileTypes
from ..constants.FilePrefixes import FilePrefixes
from ..constants.FileSuffixes import FileSuffixes
from ..exceptions.RemapMissingBlendFile import RemapMissingBlendFile
from .strategies.ModType import ModType
from .Model import Model
from .BlendFile import BlendFile
from ..tools.files.FileService import FileService
from .IniFile import IniFile
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

    version: Optional[:class:`float`]
        The game version we want the fixed mod :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will fix the mod to using the latest hashes/indices.

    Attributes
    ----------
    path: Optional[:class:`str`]
        The file location to the mod folder

    version: Optional[:class:`float`]
        The game version we want the fixed mod

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

    logger: Optional[:class:`Logger`]
        The logger used to pretty print messages

    inis: List[:class:`str`]
        The .ini files found for the mod

    remapBlend: List[:class:`str`]
        The RemapBlend.buf files found for the mod

    backupInis: List[:class:`str`]
        The DISABLED_RemapBackup.txt files found for the mod

    remapCopies: List[:class:`str`]
        The *remapFix*.ini files found for the mod
    """
    def __init__(self, path: Optional[str] = None, files: Optional[List[str]] = None, logger: Optional[Logger] = None, types: Optional[Set[ModType]] = None, 
                 defaultType: Optional[ModType] = None, version: Optional[float] = None, remappedTypes: Optional[Set[str]] = None):
        super().__init__(logger = logger)
        self.path = FileService.getPath(path)
        self.version = version
        self._files = files

        if (types is None):
            types = set()
        if (remappedTypes is None):
            remappedTypes = set()

        self._types = types
        self._remappedTypes = remappedTypes
        self._defaultType = defaultType

        self.inis = []
        self.remapBlend = []
        self.backupInis = []
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

    def _setupFiles(self):
        """
        Searches the direct children files to the mod folder if :attr:`Mod.files` is set to ``None``        
        """

        if (self._files is None):
            self._files = FileService.getFiles(path = self.path)

        self.inis, self.remapBlend, self.backupInis, self.remapCopies = self.getOptionalFiles()
        self.inis = list(map(lambda iniPath: IniFile(iniPath, logger = self.logger, modTypes = self._types, defaultModType = self._defaultType, version = self.version, modsToFix = self._remappedTypes), self.inis))

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
        Determines whether the file is a DISABLED_RemapBackup.txt file that is used to make
        backup copies of .ini files

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a DISABLED_RemapBackup.txt file
        """

        fileBaseName = os.path.basename(file)
        return (fileBaseName.startswith(FilePrefixes.BackupFilePrefix.value) or fileBaseName.startswith(FilePrefixes.OldBackupFilePrefix.value)) and file.endswith(FileExt.Txt.value)
    
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

    def getOptionalFiles(self) -> List[Optional[str]]:
        """
        Retrieves a list of each type of files that are not mandatory for the mod

        Returns
        -------
        [ List[:class:`str`], List[:class:`str`], List[:class:`str`]]
            The resultant files found for the following file categories (listed in the same order as the return type):

            #. .ini files not created by this fix
            #. .RemapBlend.buf files
            #. DISABLED_RemapBackup.txt files
            #. RemapFix.ini files

            .. note::
                See :meth:`Mod.isIni`, :meth:`Mod.isRemapBlend`, :meth:`Mod.isBackupIni`, :meth:`Mod.isRemapCopyIni` for the specifics of each type of file
        """

        SingleFileFilters = {}
        MultiFileFilters = [self.isSrcIni, self.isRemapBlend, self.isBackupIni, self.isRemapCopyIni]

        singleFiles = []
        if (SingleFileFilters):
            singleFiles = FileService.getSingleFiles(path = self.path, filters = SingleFileFilters, files = self._files, optional = True)
        multiFiles = FileService.getFiles(path = self.path, filters = MultiFileFilters, files = self._files)

        result = singleFiles
        if (not isinstance(result, list)):
            result = [result]

        result += multiFiles
        return result
    
    def removeBackupInis(self):
        """
        Removes all DISABLED_RemapBackup.txt contained in the mod
        """

        for file in self.backupInis:
            self.print("log", f"Removing the backup ini, {os.path.basename(file)}")
            try:
                os.remove(file)
            except FileNotFoundError:
                pass

    def removeRemapCopies(self):
        """
        Removes all RemapFix.ini files contained in the mod
        """

        for file in self.remapCopies:
            self.print("log", f"Removing the ini remap copy, {os.path.basename(file)}")
            try:
                os.remove(file)
            except FileNotFoundError:
                pass

    def removeFix(self, fixedBlends: Set[str], fixedInis: Set[str], visitedRemapBlendsAtRemoval: Set[str], inisSkipped: Dict[str, Exception], keepBackups: bool = True, fixOnly: bool = False, readAllInis: bool = False) -> List[Set[str]]:
        """
        Removes any previous changes done by this module's fix

        Parameters
        ----------
        fixedBlend: Set[:class:`str`]
            The file paths to the RemapBlend.buf files that we do not want to remove

        fixedInis: Set[:class:`str`]
            The file paths to the .ini files that we do not want to remove

        visitedRemapBlendsAtRemoval: Set[:class:`str`]
            The file paths to the RemapBlend.buf that have already been attempted to be removed

        inisSkipped: Dict[:class:`str`, :class:`Exception`]
            The file paths to the .ini files that are skipped due to errors

        keepBackups: :class:`bool`
            Whether to create or keep DISABLED_RemapBackup.txt files in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to not undo any changes created in the .ini files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        readAllInis: :class:`bool`
            Whether to remove the .ini fix from all the .ini files encountered :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        [Set[:class:`str`], Set[:class:`str`]]
            The removed files that have their fix removed, where the types of files for the return value is based on the list below:

            #. .ini files with their fix removed
            #. RemapBlend.buf files that got deleted
        """

        removedRemapBlends = set()
        undoedInis = set()

        for ini in self.inis:
            remapBlendsRemoved = False
            iniFilesUndoed = False
            iniFullPath = None
            iniHasErrors = False
            if (ini.file is not None):
                iniFullPath = FileService.absPathOfRelPath(ini.file, self.path)

            # parse the .ini file even if we are only undoing fixes for the case where a Blend.buf file
            #   forms a bridge with some disconnected folder subtree of a mod
            # Also, we only want to remove the Blend.buf files connected to particular types of .ini files, 
            #   instead of all the Blend.buf files in the folder
            if (iniFullPath is None or (iniFullPath not in fixedInis and iniFullPath not in inisSkipped)):
                try:
                    ini.parse()
                except Exception as e:
                    inisSkipped[iniFullPath] = e
                    iniHasErrors = True
                    self.print("handleException", e)

            # remove the fix from the .ini files
            if (not iniHasErrors and iniFullPath is not None and iniFullPath not in fixedInis and iniFullPath not in inisSkipped and (ini.isModIni or readAllInis)):
                try:
                    ini.removeFix(keepBackups = keepBackups, fixOnly = fixOnly, parse = True)
                except Exception as e:
                    inisSkipped[iniFullPath] = e
                    iniHasErrors = True
                    self.print("handleException", e)
                    continue

                undoedInis.add(iniFullPath)

                if (not iniFilesUndoed):
                    iniFilesUndoed = True

            if (iniFilesUndoed):
                self.print("space")

            # remove only the remap blends that have not been recently created
            for _, blendModel in ini.remapBlendModels.items():
                for partIndex, partFullPaths in blendModel.fullPaths.items():
                    for modName in partFullPaths:
                        remapBlendFullPath = partFullPaths[modName]

                        if (remapBlendFullPath not in fixedBlends and remapBlendFullPath not in visitedRemapBlendsAtRemoval):
                            try:
                                os.remove(remapBlendFullPath)
                            except FileNotFoundError as e:
                                self.print("log", f"No Previous {FileTypes.RemapBlend.value} found at {remapBlendFullPath}")
                            else:
                                self.print("log", f"Removing previous {FileTypes.RemapBlend.value} at {remapBlendFullPath}")
                                removedRemapBlends.add(remapBlendFullPath)

                            visitedRemapBlendsAtRemoval.add(remapBlendFullPath)
                            if (not remapBlendsRemoved):
                                remapBlendsRemoved = True

            if (remapBlendsRemoved):
                self.print("space")

        return [undoedInis, removedRemapBlends]

    @classmethod
    def blendCorrection(cls, blendFile: Union[str, bytes], modType: ModType, modToFix: str, 
                        fixedBlendFile: Optional[str] = None, version: Optional[float] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Blend.buf file

        See :meth:`BlendFile.correct` for more info

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
        :class:`BlendFileNotRecognized`
            If the original Blend.buf file provided by the parameter ``blendFile`` cannot be read

        :class:`BadBlendData`
            If the bytes passed into this function do not correspond to the format defined for a Blend.buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedBlendFile`` is ``None``, then will return an array of bytes for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed RemapBlend.buf file if the provided Blend.buf file got corrected
        """

        blend = BlendFile(blendFile)
        vgRemap = modType.getVGRemap(modToFix, version = version)
        return blend.correct(vgRemap = vgRemap, fixedBlendFile = fixedBlendFile)
    
    def correctBlend(self, fixedRemapBlends: Set[str], skippedBlends: Dict[str, Exception], fixOnly: bool = False) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the Blend.buf files reference by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        fixedRemapBlends: Set[:class:`str`]
            All of the RemapBlend.buf files that have already been fixed.

        skippedBlends: Dict[:class:`str`, :class:`Exception`]
            All of the RemapBlend.buf files that have already been skipped due to some error when trying to fix them :raw-html:`<br />` :raw-html:`<br />`

            The keys are the absolute filepath to the RemapBlend.buf file that was attempted to be fixed and the values are the exception encountered

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

        currentBlendsSkipped = {}
        currentBlendsFixed = set()

        for ini in self.inis:
            if (ini is None):
                continue

            for _, model in ini.remapBlendModels.items():
                modType = self._defaultType
                if (ini.type is not None):
                    modType = ini.type

                for partIndex, partFullPaths in model.fullPaths.items():
                    for modName, fixedFullPath in partFullPaths.items():
                        try:
                            origFullPath = model.origFullPaths[partIndex]
                        except KeyError:
                            self.print("log", f"Missing Original Blend file for the RemapBlend file at {fixedFullPath}")
                            if (fixedFullPath not in skippedBlends):
                                error = RemapMissingBlendFile(fixedFullPath)
                                currentBlendsSkipped[fixedFullPath] = error
                                skippedBlends[fixedFullPath] = error
                            break

                        # check if the blend was already encountered and did not need to be fixed
                        if (origFullPath in fixedRemapBlends or modType is None):
                            break
                        
                        # check if the blend file that did not need to be fixed already had encountered an error
                        if (origFullPath in skippedBlends):
                            self.print("log", f"Blend file has already previously encountered an error at {origFullPath}")
                            break
                        
                        # check if the blend file has been fixed
                        if (fixedFullPath in fixedRemapBlends):
                            self.print("log", f"Blend file has already been corrected at {fixedFullPath}")
                            continue

                        # check if the blend file already had encountered an error
                        if (fixedFullPath in skippedBlends):
                            self.print("log", f"Blend file has already previously encountered an error at {fixedFullPath}")
                            continue

                        # check if the fixed RemapBlend.buf file already exists and we only want to fix mods without removing their previous fixes
                        if (fixOnly and os.path.isfile(fixedFullPath)):
                            self.print("log", f"Blend file was previously fixed at {fixedFullPath}")
                            continue
                        
                        # fix the blend
                        correctedBlendPath = None
                        try:
                            correctedBlendPath = self.blendCorrection(origFullPath, modType, modName, fixedBlendFile = fixedFullPath, version = self.version)
                        except Exception as e:
                            currentBlendsSkipped[fixedFullPath] = e
                            skippedBlends[fixedFullPath] = e
                            self.print("handleException", e)
                        else:
                            pathToAdd = ""
                            if (correctedBlendPath is None):
                                self.print("log", f"Blend file does not need to be corrected at {origFullPath}")
                                pathToAdd = origFullPath
                            else:
                                self.print("log", f'Blend file correction done at {fixedFullPath}')
                                pathToAdd = fixedFullPath

                            currentBlendsFixed.add(pathToAdd)
                            fixedRemapBlends.add(pathToAdd)

        return [currentBlendsFixed, currentBlendsSkipped]
##### EndScript