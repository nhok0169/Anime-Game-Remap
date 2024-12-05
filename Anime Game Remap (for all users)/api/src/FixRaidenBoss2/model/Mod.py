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
from typing import Optional, List, Set, Union, Dict, Callable, Any
##### EndExtImports

##### LocalImports
from ..constants.FileExt import FileExt
from ..constants.FileTypes import FileTypes
from ..constants.FilePrefixes import FilePrefixes
from ..constants.FileSuffixes import FileSuffixes
from ..exceptions.RemapMissingBlendFile import RemapMissingBlendFile
from .strategies.ModType import ModType
from .Model import Model
from .files.BlendFile import BlendFile
from .files.TextureFile import TextureFile
from ..tools.files.FileService import FileService
from ..tools.ListTools import ListTools
from .files.IniFile import IniFile
from .FileStats import FileStats
from .iniresources.IniResourceModel import IniResourceModel
from .iniresources.IniTexModel import IniTexModel
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

    inis: Dict[:class:`str`, :class:`IniFile`]
        The .ini files found for the mod :raw-html:`<br />` :raw-html:`<br />`

        The keys are the file paths to the .ini file

    remapBlend: List[:class:`str`]
        The RemapBlend.buf files found for the mod

    backupInis: List[:class:`str`]
        The DISABLED_RemapBackup.txt files found for the mod

    remapCopies: List[:class:`str`]
        The *remapFix*.ini files found for the mod

    remapTextures: List[:class:`str`]
        The *remapFix*.dds files found for the mod
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

        self.inis, self.remapBlend, self.backupInis, self.remapCopies, self.remapTextures = self.getOptionalFiles()

        iniPaths = self.inis
        self.inis = {}
        for iniPath in iniPaths:
            iniFile = IniFile(iniPath, logger = self.logger, modTypes = self._types, defaultModType = self._defaultType, version = self.version, modsToFix = self._remappedTypes)
            self.inis[iniFile.file] = iniFile

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
        MultiFileFilters = [self.isSrcIni, self.isRemapBlend, self.isBackupIni, self.isRemapCopyIni, self.isRemapTexture]

        singleFiles = []
        if (SingleFileFilters):
            singleFiles = FileService.getSingleFiles(path = self.path, filters = SingleFileFilters, files = self._files, optional = True)
        multiFiles = FileService.getFiles(path = self.path, filters = MultiFileFilters, files = self._files)

        result = singleFiles
        if (not isinstance(result, list)):
            result = [result]

        result += multiFiles
        return result
    
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
        Removes all DISABLED_RemapBackup.txt contained in the mod
        """

        self._removeFileType("backupInis", lambda file: f"Removing the backup ini, {os.path.basename(file)}")

    def removeRemapCopies(self):
        """
        Removes all RemapFix.ini files contained in the mod
        """

        self._removeFileType("remapCopies", lambda file: f"Removing the ini remap copy, {os.path.basename(file)}")

    def _removeIniResources(self, ini: IniFile, result: Set[str], resourceName: str, resourceStats: FileStats, getIniResources: Callable[[IniFile], List[IniResourceModel]]) -> bool:
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

        getIniResource: Callable[[:class:`IniFile`], List[:class:`IniResourceModel`]]
            The function to retrieve the data related to the resource from the .ini file

        Returns
        -------
        :class:`bool`
            Whether there was a file that was attempted to be removed
        """

        iniResources = getIniResources(ini)
        hasRemovedResource = False

        for texModel in iniResources:
            for fixedPath, fixedFullPath, origPath, origFullPath in texModel:
                if (fixedFullPath not in resourceStats.fixed and fixedFullPath not in resourceStats.visitedAtRemoval):
                    try:
                        os.remove(fixedFullPath)
                    except FileNotFoundError as e:
                        self.print("log", f"No Previous {resourceName} found at {fixedFullPath}")
                    else:
                        self.print("log", f"Removing previous {resourceName} at {fixedFullPath}")
                        result.add(fixedFullPath)
                    
                    resourceStats.addVisitedAtRemoval(fixedFullPath)

                    if (not hasRemovedResource):
                        hasRemovedResource = True

        return hasRemovedResource

    def removeFix(self, blendStats: FileStats, iniStats: FileStats, texStats:FileStats, keepBackups: bool = True, fixOnly: bool = False, readAllInis: bool = False) -> List[Set[str]]:
        """
        Removes any previous changes done by this module's fix

        Parameters
        ----------
        blendStats: :class:`FileStats`
            The data about Blend.buf files

        iniStats: :class:`FileStats`
            The data about .ini files

        texStats: :class:`FileStats`
            The data about .dds files

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
        [Set[:class:`str`], Set[:class:`str`], Set[:class:`str`]]
            The removed files that have their fix removed, where the types of files for the return value is based on the list below:

            #. .ini files with their fix removed
            #. RemapBlend.buf files that got deleted
            #. RemapTex.dds files that got deleted
        """

        removedRemapBlends = set()
        removedTextures = set()
        undoedInis = set()

        for iniPath in self.inis:
            ini = self.inis[iniPath]

            remapBlendsRemoved = False
            texRemoved = False
            iniFilesUndoed = False
            iniFullPath = None
            iniHasErrors = False
            if (ini.file is not None):
                iniFullPath = FileService.absPathOfRelPath(ini.file, self.path)

            # parse the .ini file even if we are only undoing fixes for the case where some resource file (Blend.buf, .dds, etc...)
            #   forms a bridge with some disconnected folder subtree of a mod
            # Also, we only want to remove the resource files connected to particular types of .ini files, 
            #   instead of all the resource files in the folder
            if (iniFullPath is None or (iniFullPath not in iniStats.fixed and iniFullPath not in iniStats.skipped)):
                try:
                    ini.parse()
                except Exception as e:
                    iniStats.addSkipped(iniFullPath, e, modFolder = self.path)
                    iniHasErrors = True
                    self.print("handleException", e)

            # remove the fix from the .ini files
            if (not iniHasErrors and iniFullPath is not None and iniFullPath not in iniStats.fixed and iniFullPath not in iniStats.skipped and (ini.isModIni or readAllInis)):
                try:
                    ini.removeFix(keepBackups = keepBackups, fixOnly = fixOnly, parse = True)
                except Exception as e:
                    iniStats.addSkipped(iniFullPath, e, modFolder = self.path)
                    iniHasErrors = True
                    self.print("handleException", e)
                    continue

                undoedInis.add(iniFullPath)

                if (not iniFilesUndoed):
                    iniFilesUndoed = True

            if (iniFilesUndoed):
                self.print("space")

            # remove only the remap blends that have not been recently created
            remapBlendsRemoved = self._removeIniResources(ini, removedRemapBlends, FileTypes.RemapBlend.value, blendStats, lambda iniFile: iniFile.remapBlendModels.values())
            if (remapBlendsRemoved):
                self.print("space")

            # remove only the remap texture files that have not been recently created
            texRemoved = self._removeIniResources(ini, removedTextures, FileTypes.RemapTexture.value, texStats, lambda iniFile: iniFile.getTexAddModels())
            if (texRemoved):
                self.print("space")

        return [undoedInis, removedRemapBlends, removedTextures]

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
    
    @classmethod
    def _texCorrection(cls, fixedTexFile: str, modToFix: str, model: IniTexModel, partInd: int, pathInd: int, texFile: Optional[str] = None) -> str:
        texEditor = model.texEdits[partInd][modToFix][pathInd]
        return cls.texCorrection(fixedTexFile, texEditor, texFile = texFile)
    
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
        """
        if (texFile is None):
            texFile = fixedTexFile

        tex = TextureFile(texFile)
        texEditor.fix(tex, fixedTexFile)
        return fixedTexFile

    def correctResource(self, resourceStats: FileStats, getResourceModels: Callable[[IniFile], List[IniResourceModel]], correctFile: Callable[[str, str, ModType, str, int, IniResourceModel], str], 
                        iniPaths: Optional[List[str]] = None, fileTypeName: str = "", needsSrcFile: bool = True, fixOnly: bool = False) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the files for a particular type of resource referenced by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        resourceStats: :class:`FileStats`
            The stats to keep track of whether the particular resource has been fixed or skipped

        getResourceModels: Callable[[:class:`IniFile`], List[:class:`IniResourceModel`]]
            Function to retrieve all of the needed :class:`IniResourceModel` from some .ini file

        correctFile: Callable[[:class:`str`, :class:`str`, :class:`ModType`, :class:`str`, :class:`int`, :class:`IniResourceModel`], :class:`str`]
            Function to fix up the resource file :raw-html:`<br />` :raw-html:`<br />`

            The parameters for the function are as follows:

            #. The full file path to the original resource
            #. The fixed file path to the resource
            #. The type of mod being fixed within the .ini file
            #. The name of the mod to fix to
            #. The index of the part within the :class:`IfTemplate`
            #. The index of the path within the particular part of the :class:`IfTemplate`
            #. The version of the game to fix to
            #. The current :class:`IniResourceModel` being processed

            :raw-html:`<br />` :raw-html:`<br />`

            The function returns a :class:`str` with the fixed file path to the resource

        iniPaths: Optional[List[:class:`str`]]
            The file paths to the .ini file to have their resources corrected. If this value is ``None``, then will correct all the .ini file in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fileTypeName: :class:`str`
            The name of the file resource

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
        fileTypeName = "file" if (fileTypeName == "") else f"{fileTypeName} file"

        if (iniPaths is None):
            iniPaths = list(self.inis.keys())
        else:
            iniPaths = ListTools.getDistinct(iniPaths, keepOrder = True)

        for iniPath in iniPaths:
            ini = None
            try:
                ini = self.inis[iniPath]
            except KeyError:
                continue

            if (ini is None):
                continue
            
            modType = ini.availableType
            resourceModels = getResourceModels(ini)
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
                                    self.print("log", f"Missing Original {fileTypeName} for the RemapBlend file at {fixedFullPath}")
                                    if (fixedFullPath not in resourceStats.skipped):
                                        error = RemapMissingBlendFile(fixedFullPath)
                                        currentBlendsSkipped[fixedFullPath] = error
                                        resourceStats.addSkipped(fixedFullPath, error, modFolder = self.path)
                                    break

                            # check if the file was already encountered and did not need to be fixed
                            if ((origFullPath is not None and origFullPath in resourceStats.fixed) or modType is None):
                                break
                            
                            # check if the file that did not need to be fixed already had encountered an error
                            if (origFullPath is not None and origFullPath in resourceStats.skipped):
                                self.print("log", f"{fileTypeName} has already previously encountered an error at {origFullPath}")
                                break
                            
                            # check if the file has been fixed
                            if (fixedFullPath in resourceStats.fixed):
                                self.print("log", f"{fileTypeName} has already been corrected at {fixedFullPath}")
                                continue

                            # check if the file already had encountered an error
                            if (fixedFullPath in resourceStats.skipped):
                                self.print("log", f"{fileTypeName} has already previously encountered an error at {fixedFullPath}")
                                continue

                            # check if the fixed file already exists and we only want to fix mods without removing their previous fixes
                            if (fixOnly and os.path.isfile(fixedFullPath)):
                                self.print("log", f"{fileTypeName} was previously fixed at {fixedFullPath}")
                                continue
                            
                            # fix the file resource
                            correctedResourcePath = None
                            try:
                                correctedResourcePath = correctFile(origFullPath, fixedFullPath, modType, modName, partIndex, i, self.version, model)
                            except Exception as e:
                                currentBlendsSkipped[fixedFullPath] = e
                                resourceStats.addSkipped(fixedFullPath, e, modFolder = self.path)
                                self.print("handleException", e)
                            else:
                                pathToAdd = ""
                                if (correctedResourcePath is None):
                                    self.print("log", f"{fileTypeName} does not need to be corrected at {origFullPath}")
                                    pathToAdd = origFullPath
                                else:
                                    self.print("log", f'{fileTypeName} correction done at {fixedFullPath}')
                                    pathToAdd = fixedFullPath

                                currentBlendsFixed.add(pathToAdd)
                                resourceStats.addFixed(pathToAdd)

        return [currentBlendsFixed, currentBlendsSkipped]
    
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
                                    lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniTexModel: self._texCorrection(fixedFullPath, modName, iniTexModel, partInd, pathInd, texFile = origFullPath),
                                    fileTypeName = "Texture", fixOnly = fixOnly, iniPaths = iniPaths)
        
        fixedTexEdits, skippedTexEdits = self.correctResource(texEditStats, lambda iniFile: iniFile.getTexEditModels(), 
                                    lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniTexModel: self._texCorrection(fixedFullPath, modName, iniTexModel, partInd, pathInd, texFile = origFullPath),
                                    fileTypeName = "Texture", fixOnly = fixOnly, iniPaths = iniPaths)
        
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
                                    lambda origFullPath,  fixedFullPath, modType, modName, partInd, pathInd, version, iniResourceModel: self.blendCorrection(origFullPath, modType, modName, fixedBlendFile = fixedFullPath, version = version),
                                    fileTypeName = "Blend", fixOnly = fixOnly, iniPaths = iniPaths)
##### EndScript