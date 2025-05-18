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
from collections import deque
from typing import Optional, Dict, Callable, List
##### EndExtImports

##### LocalImports
from .constants.FilePathConsts import FilePathConsts
from .constants.FileTypes import FileTypes
from .controller.enums.CommandOpts import CommandOpts
from .constants.FileExt import FileExt
from .constants.FileEncodings import FileEncodings
from .constants.FilePrefixes import FilePrefixes
from .constants.ModTypes import ModTypes
from .constants.DownloadMode import DownloadMode
from .constants.Packages import PackageModules
from .constants.GlobalPackageManager import GlobalPackageManager
from .exceptions.InvalidModType import InvalidModType
from .exceptions.InvalidDownloadMode import InvalidDownloadMode
from .exceptions.ConflictingOptions import ConflictingOptions
from .view.Logger import Logger
from .model.strategies.ModType import ModType
from .model.Mod import Mod
from .model.stats.FileStats import FileStats
from .model.stats.CachedFileStats import CachedFileStats
from .model.stats.RemapStats import RemapStats
from .model.files.IniFile import IniFile
from .tools.files.FileService import FileService
from .tools.Heading import Heading
from .tools.concurrency.ProcessManager import ProcessManager
from .tools.concurrency.ThreadManager import ThreadManager
##### EndLocalImports


##### Script
class RemapService():
    """
    The overall class for remapping mods

    Parameters
    ----------
    path: Optional[:class:`str`]
        The file location of where to run the fix. :raw-html:`<br />` :raw-html:`<br />`

        If this attribute is set to ``None``, then will run the fix from wherever this class is called :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    keepBackups: :class:`bool`
        Whether to keep backup versions of any .ini files that the script fixes :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    fixOnly: :class:`bool`
        Whether to only fix the mods without removing any previous changes this fix script may have made :raw-html:`<br />` :raw-html:`<br />`

        .. warning::
            if this is set to ``True`` and :attr:`undoOnly` is also set to ``True``, then the fix will not run and will throw a :class:`ConflictingOptions` exception

        :raw-html:`<br />`

        **Default**: ``False``

    undoOnly: :class:`bool`
        Whether to only undo the fixes previously made by the fix :raw-html:`<br />` :raw-html:`<br />`

        .. warning::
            if this is set to ``True`` and :attr:`fixOnly` is also set to ``True``, then the fix will not run and will throw a :class:`ConflictingOptions` exception

        :raw-html:`<br />`

        **Default**: ``True``

    hideOrig: :class:`bool`
        Whether to not show the mod on the original character :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    readAllInis: :class:`bool`
        Whether to read all the .ini files that the fix encounters :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    types: Optional[List[:class:`str`]]
        The names for all the types of mods to fix.  :raw-html:`<br />` :raw-html:`<br />`

        If this argument is an empty list or this argument is ``None``, then will fix all the types of mods supported by this fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

        :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

    remappedTypes: Optional[List[:class:`str`]]
        The names for the types of mods to be remapped based from the types of mods specified at :attr:`RemapService.types`. :raw-html:`<br />` :raw-html:`<br />`

        For a mod specified at :attr:`RemapService.types`, if none of its corresponding mods to remap are specified in this attribute, then will remap the mod specified at :attr:`RemapService.types` to all its corresponding mods to remap.

        If this argument is an empty list or this argument is ``None``, then will fix the mods specified at :attr:`types` to all of their corresponding remapped mods :raw-html:`<br />` :raw-html:`<br />`

        eg.
        if :attr:`RemapService.types` is ``["Kequeen", "jean"]`` and this attribute is ``["jeanSea"]``, then this class will perform the following remaps:
        
        * Keqing --> KeqingOpulent
        * Jean --> JeanSea

        **Note: ** Jean --> JeanCN will not be remapped for the above example :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultType: Optional[:class:`str`]
        The name for the type to use if a mod has an unidentified type :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then mods with unidentified types will be skipped :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    forcedType: Optional[:class:`str`]
        The mod type to forcibly assume for the parsed .ini files :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    log: Optional[:class:`str`]
        The folder location to log the run of the fix into a seperate text file :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will not log the fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    verbose: :class:`bool`
        Whether to print the progress for fixing mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    handleExceptions: :class:`bool`
        When an exception is caught, whether to silently stop running the fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    version: Optional[:class:`str`]
        The game version we want the fix to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    proxy: Optional[:class:`str`]
        The link to the proxy server used for any internet network requests made :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will assume all internet network requests do not require the need to go through a proxy server.

    downloadMode: Optional[:class:`str`]
        The download mode to handle file downloads :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then the software will default to use :attr:`DownloadMode.HardTexDriven` as the download mode :raw-html:`<br />`

        .. note::
            For more information about the available download modes to specify, see :ref:`Download Modes`

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _loggerBasePrefix: :class:`str`
        The prefix string for the logger used when the fix returns back to the original directory that it started to run

    logger: :class:`Logger`
        The logger used to pretty print messages

    _path: :class:`str`
        The file location of where to run the fix.

    keepBackups: :class:`bool`
        Whether to keep backup versions of any .ini files that the script fixes

    fixOnly: :class:`bool`
        Whether to only fix the mods without removing any previous changes this fix script may have made

    undoOnly: :class:`bool`
        Whether to only undo the fixes previously made by the fix

    hideOrig: :class:`bool`
        Whether to not show the mod on the original character

    readAllInis: :class:`bool`
        Whether to read all the .ini files that the fix encounters

    types: Set[:class:`ModType`]
        All the types of mods that will be fixed. :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

    remappedTypes: Set[:class:`str`]
        The names for the types of mods to be remapped based from the types of mods specified at :attr:`RemapService.types`. :raw-html:`<br />` :raw-html:`<br />`

        For a mod specified at :attr:`RemapService.types`, if none of its corresponding mods to remap are specified in this attribute, then will remap the mod specified at :attr:`RemapService.types` to all its corresponding mods to remap.

        If this argument is an empty list or this argument is ``None``, then will fix the mods specified at :attr:`RemapService.types` to all of their corresponding remapped mods :raw-html:`<br />` :raw-html:`<br />`

        eg.
        if :attr:`RemapService.types` is ``["Kequeen", "jean"]`` and this attribute is ``["jeanSea"]``, then this class will perform the following remaps:
        
        * Keqing --> KeqingOpulent
        * Jean --> JeanSea

        **Note: ** Jean --> JeanCN will not be remapped for the above example :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

    defaultType: Optional[:class:`ModType`]
        The type to use if a mod has an unidentified type :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

    forcedType: Optional[:class:`ModType`]
        The mod type to forcibly assume for the parsed .ini files :raw-html:`<br />`

        .. note::
            For more information about the available mod names/aliases to reference, see :ref:`Mod Types`

    version: Optional[:class:`float`]
        The game version we want the fix to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version.

    downloadMode: :class:`DownloadMode`
        The download mode to handle file downloads :raw-html:`<br />`

        .. note::
            For more information about the available download modes to specify, see :ref:`Download Modes`

    handleExceptions: :class:`bool`
        When an exception is caught, whether to silently stop running the fix

    _logFile: :class:`str`
        The file path of where to generate a log .txt file

    _pathIsCWD: :class:`bool`
        Whether the filepath that the program runs from is the current directory where this module is loaded

    stats: :class:`RemapStats`
        The statistics gathered about the fix process
    """

    def __init__(self, path: Optional[str] = None, keepBackups: bool = True, fixOnly: bool = False, undoOnly: bool = False, hideOrig: bool = False,
                 readAllInis: bool = False, types: Optional[List[str]] = None, defaultType: Optional[str] = None, forcedType: Optional[str] = None, 
                 log: Optional[str] = None, verbose: bool = True, handleExceptions: bool = False, version: Optional[str] = None, remappedTypes: Optional[List[str]] = None,
                 proxy: Optional[str] = None, downloadMode: Optional[str] = None):
        self.proxy = proxy
        self.downloadMode = downloadMode

        self._loggerBasePrefix = ""
        self.logger = Logger(logTxt = bool(log), verbose = verbose)
        self.log = log

        self._path = path
        self.keepBackups = keepBackups
        self.fixOnly = fixOnly
        self.undoOnly = undoOnly
        self.hideOrig = hideOrig
        self.readAllInis = readAllInis
        self.types = types
        self.remappedTypes = remappedTypes
        self.defaultType = defaultType
        self.forcedType = forcedType
        self._verbose = verbose
        self.version = version
        self.handleExceptions = handleExceptions
        self._pathIsCwd = False
        self.__errorsBeforeFix = None

        # certain statistics about the fix
        self.stats = RemapStats()

        self._setupModPath()
        self._setupForcedModType()
        self._setupDefaultModType()
        self._setupToFixModTypes()
        self._setupRemappedTypes()
        self._setupVersion()
        self._setupDownloadMode()

        self._iniExecs = ThreadManager(jobNo = 10)

        if (self.__errorsBeforeFix is None):
            self._printModsToFix()

    @property
    def pathIsCwd(self):
        """
        Whether the filepath that the program runs from is the current directory where this module is loaded

        :getter: Returns whether the filepath that the program runs from is the current directory of where the module is loaded
        :type: :class:`bool`
        """

        return self._pathIsCwd
    
    @property
    def path(self) -> str:
        """
        The filepath of where the fix is running from

        :getter: Returns the path of where the fix is running
        :setter: Sets the path for where the fix runs
        :type: :class:`str`
        """

        return self._path
    
    @path.setter
    def path(self, newPath: Optional[str]):
        self._path = newPath
        self._setupModPath()
        self.clear()

    @property
    def log(self) -> str:
        """
        The folder location to log the run of the fix into a seperate text file

        :getter: Returns the file path to the log
        :setter: Sets the path for the log
        :type: :class:`str`
        """

        return self._log
    
    @log.setter
    def log(self, newLog: Optional[str]):
        self._log = newLog
        self._setupLogPath()
        self.logger.logTxt = bool(newLog)

    @property
    def verbose(self) -> bool:
        """
        Whether to print the progress for fixing mods

        :getter: Tells whether progress will be printed when fixing mods
        :setter: Sets the new flag for whether to print progress
        :type: :class:`bool`
        """

        return self._verbose
    
    @verbose.setter
    def verbose(self, newVerbose: bool):
        self._verbose = newVerbose
        self.logger.verbose = newVerbose

    @property
    def proxy(self) -> Optional[str]:
        """
        The link to the proxy server used for any internet network requests made :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will assume all internet network requests do not require the need to go through a proxy server.

        :getter: Retrieves the proxy link
        :setter: Sets the new proxy link
        :type: Optional[:class:`str`]
        """

        return self._proxy
    
    @proxy.setter
    def proxy(self, newProxy: str):
        self._proxy = newProxy
        GlobalPackageManager.Packager.value.proxy = self._proxy

    def clear(self, clearLog: bool = True):
        """
        Clears up all the saved data

        Paramters
        ---------
        clearLog: :class:`bool`
            Whether to also clear out any saved data in the logger
        """

        self.stats.clear()

        if (clearLog):
            self.logger.clear()
    
    def _setupModPath(self):
        """
        Sets the filepath of where the fix will run from
        """

        self._pathIsCwd = False
        if (self._path is None):
            self._path = FilePathConsts.DefaultPath
            self._pathIsCwd = True
            return

        self._path = FileService.parseOSPath(self._path)
        self._path = FileService.parseOSPath(os.path.abspath(self._path))
        self._pathIsCwd = (self._path == FilePathConsts.DefaultPath)

    def _setupLogPath(self):
        """
        Sets the folder path for where the log file will be stored
        """

        if (self._log is not None):
            self._log = FileService.parseOSPath(os.path.join(self._log, FileTypes.Log.value))

    def _setupModTypes(self, attr: str):
        """
        Sets the types of mods that will be fixed / fix to

        Parameters
        ----------
        attr: :class:`str`
            The name of the attribute within this class set the mods for
        """
        attrVal = getattr(self, attr)
        if (isinstance(attrVal, set)):
            return

        modTypes = set()
        if (attrVal is None or not attrVal):
            modTypes = ModTypes.getAll()

        # search for the types of mods to fix
        else:
            for typeStr in attrVal:
                modType = ModTypes.search(typeStr)
                modTypeFound = bool(modType is not None)

                if (modTypeFound):
                    modTypes.add(modType)
                elif (self.__errorsBeforeFix is None):
                    self.__errorsBeforeFix = InvalidModType(typeStr)
                    return

        setattr(self, attr, modTypes)

    def _setupToFixModTypes(self):
        """
        Sets the names for the type of mods that will be fixed
        """

        hasForcedModType = self.forcedType is not None
        if (hasForcedModType and isinstance(self.forcedType, ModType)):
            self.types = {self.forcedType}
            return

        elif (hasForcedModType and isinstance(self.forcedType, str)):
            self.types = [self.forcedType]

        elif (self.readAllInis):
            self.types = ModTypes.getAll()
            return

        self._setupModTypes("types")

    def _setupRemappedTypes(self):
        """
        Sets the names for the types of mods that will be fixed to
        """

        self._setupModTypes("remappedTypes")
        if (self.__errorsBeforeFix is not None):
            return
        
        self.remappedTypes = set(map(lambda remappedType: remappedType.name, self.remappedTypes))

    def _setupVersion(self):
        """
        Sets the game version to fix to
        """

        if (self.version is None):
            return

        try:
            self.version = float(self.version)
        except ValueError:
            if (self.__errorsBeforeFix is None):
                self.__errorsBeforeFix = ValueError("Please enter a float for the game version")

    def _setupDefaultModType(self):
        """
        Sets the default mod type to be used for an unidentified mod
        """

        if (not self.readAllInis or self.forcedType is not None):
            self.defaultType = None
            return

        elif (self.defaultType is None):
            self.defaultType = ModTypes.Raiden.value
            return

        elif (isinstance(self.defaultType, ModType)):
            return

        foundModType = ModTypes.search(self.defaultType)
        if (foundModType is None and self.__errorsBeforeFix is None):
            self.__errorsBeforeFix = InvalidModType(self.defaultType)
        
        self.defaultType = foundModType

    def _setupForcedModType(self):
        """
        Sets the forced mod type to assume for the .ini files
        """

        if (self.forcedType is None or isinstance(self.forcedType, ModType)):
            return

        foundModType = ModTypes.search(self.forcedType)
        if (foundModType is None and self.__errorsBeforeFix is None):
            self.__errorsBeforeFix = InvalidModType(self.forcedType)
            return
        
        self.forcedType = foundModType

    def _setupDownloadMode(self):
        """
        Sets the download mode the software will use for file downloads
        """

        if (self.downloadMode is None):
            self.downloadMode = DownloadMode.HardTexDriven
            return
        
        foundDownloadMode = DownloadMode.search(self.downloadMode)
        if (foundDownloadMode is None and self.__errorsBeforeFix is None):
            self.__errorsBeforeFix = InvalidDownloadMode(self.downloadMode)
            return

        self.downloadMode = foundDownloadMode

    def _printModsToFix(self):
        """
        Prints out the types of mods that will be fixed
        """

        self.logger.includePrefix = False

        self.logger.openHeading("Types of Mods To Fix", 5)
        self.logger.space()

        if (not self.types):
            self.logger.log("All mods")
        else:
            sortedModNames = list(map(lambda modType: modType.name, self.types))
            sortedModNames.sort()

            for name in sortedModNames:
                self.logger.bulletPoint(f"{name}")
        
        self.logger.space()
        self.logger.closeHeading()
        self.logger.split() 
        self.logger.includePrefix = True
    
    # fixes an ini file in a mod
    def fixIni(self, ini: IniFile, mod: Mod, flushIfTemplates: bool = True) -> bool:
        """
        Fixes an individual .ini file for a particular mod

        .. tip:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        ini: :class:`IniFile`
            The .ini file to fix

        mod: :class:`Mod`
            The mod being fixed

        flushIfTemplates: :class:`bool`
            Whether to re-parse the :class:`IfTemplates`s in the .ini files instead of using the saved cached values :raw-html:`<br />` :raw-html:`<br />`
             
            **Default**: ``True``

        Returns
        -------
        :class:`bool`
            Whether the particular .ini file has just been fixed
        """

        # check if the .ini is belongs to some mod
        if (ini is None or not ini.isModIni):
            return False

        if (self.undoOnly):
            return True

        fileBaseName = os.path.basename(ini.file)
        iniFullPath = FileService.absPathOfRelPath(ini.file, mod.path)

        if (iniFullPath in self.stats.ini.skipped):
            self.logger.log(f"the ini file, {fileBaseName}, has alreaedy encountered an error")
            return False
        
        if (iniFullPath in self.stats.ini.fixed):
            self.logger.log(f"the ini file, {fileBaseName}, is already fixed")
            return True

        # parse the .ini file
        self.logger.log(f"Parsing {fileBaseName}...")
        ini.parse(flushIfTemplates = flushIfTemplates)

        if (ini.isFixed):
            self.logger.log(f"the ini file, {fileBaseName}, is already fixed")
            return True
        
        # download the required files
        mod.downloadFiles(self.stats.download, iniPaths = [ini.file], fixOnly = self.fixOnly, proxy = self._proxy)

        # fix the blends
        mod.correctBlend(self.stats.blend, fixOnly = self.fixOnly, iniPaths = [ini.file])

        # fix the positions
        mod.correctPosition(self.stats.position, fixOnly = self.fixOnly, iniPaths = [ini.file])

        # writing the fixed file
        self.logger.log(f"Making the fixed ini file for {fileBaseName}")
        ini.fix(keepBackup = self.keepBackups, fixOnly = self.fixOnly, hideOrig = self.hideOrig)
        self.logger.space()

        # fix the textures
        mod.correctTex(self.stats.texAdd, self.stats.texEdit, fixOnly = self.fixOnly, iniPaths = [ini.file])

        return True

    # fixes a mod
    def fixMod(self, mod: Mod, flushIfTemplates: bool = True) -> bool:
        """
        Fixes a particular mod

        .. tip:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        mod: :class:`Mod`
            The mod being fixed

        flushIfTemplates: :class:`bool`
            Whether to re-parse the :class:`IfTemplates`s in the .ini files instead of using the saved cached values :raw-html:`<br />` :raw-html:`<br />`
             
            **Default**: ``True``

        Returns
        -------
        :class:`bool`
            Whether the particular mod has just been fixed
        """

        # remove any backups
        if (not self.keepBackups):
            mod.removeBackupInis()

        for iniPath in mod.inis:
            ini = mod.inis[iniPath]
            ini.classify()

        # undo any previous fixes
        if (not self.fixOnly):
            undoedInis, removedRemapBlends, removedRemapPositions, removedTextures, removedDownloads = mod.removeFix(self.stats,
                                                                                                                     keepBackups = self.keepBackups, fixOnly = self.fixOnly, 
                                                                                                                     readAllInis = self.readAllInis, writeBackInis = self.undoOnly)
            self.stats.blend.updateRemoved(removedRemapBlends)
            self.stats.position.updateRemoved(removedRemapPositions)
            self.stats.ini.updateUndoed(undoedInis)
            self.stats.texAdd.updateRemoved(removedTextures)
            self.stats.download.updateRemoved(removedDownloads)

        # clear the temporary models only used for undoing the fix
        if (not self.undoOnly):
            for iniPath in mod.inis:
                ini = mod.inis[iniPath]
                ini.clearModels()

        result = False
        firstIniException = None
        inisLen = len(mod.inis)

        i = 0
        for iniPath in mod.inis:
            ini = mod.inis[iniPath]
            iniFullPath = FileService.absPathOfRelPath(ini.file, mod.path)
            iniIsFixed = False

            try:
                iniIsFixed = self.fixIni(ini, mod, flushIfTemplates = True)
            except Exception as e:
                self.logger.handleException(e)
                self.stats.ini.addSkipped(iniFullPath, e)

                if (firstIniException is None):
                    firstIniException = e

            if (firstIniException is None and iniFullPath in self.stats.ini.skipped):
                firstIniException = self.stats.ini.skipped[iniFullPath]

            result = (result or iniIsFixed)

            if (not iniIsFixed):
                i += 1
                continue
            
            if (not self.undoOnly and i < inisLen - 1):
                self.logger.space()

            self.stats.ini.addFixed(iniFullPath)
            i += 1

        if (not result and firstIniException is not None):
            self.stats.mod.addSkipped(mod.path, firstIniException, modFolder = mod.path)

        return result
    
    def addTips(self):
        """
        Prints out any useful tips for the user to know
        """

        self.logger.includePrefix = False

        if (not self.undoOnly or self.keepBackups):
            self.logger.split()
            self.logger.openHeading("Tips", sideLen = 10)

            if (self.keepBackups):
                self.logger.bulletPoint(f'Hate deleting the "{FilePrefixes.BackupFilePrefix.value}" {FileExt.Ini.value}/{FileExt.Txt.value} files yourself after running this script? (cuz I know I do!) Run this script again (on CMD) using the {CommandOpts.DeleteBackup.value} option')

            if (not self.undoOnly):
                self.logger.bulletPoint(f"Want to undo this script's fix? Run this script again (on CMD) using the {CommandOpts.Revert.value} option")

            if (not self.hideOrig):
                self.logger.bulletPoint(f"Want the mod to only show on the remapped character and not the original character? Run this script again (on CMD) using the {CommandOpts.HideOriginal.value} options")

            if (not self.readAllInis):
                self.logger.bulletPoint(f"Were your {FileTypes.Ini.value}s not read? Run this script again (on CMD) using the {CommandOpts.All.value} option")

            self.logger.space()
            self.logger.log("For more info on command options, run this script (on CMD) using the --help option")
            self.logger.closeHeading()

        self.logger.includePrefix = True


    def reportSkippedAsset(self, assetName: str, assetDict: Dict[str, Exception], warnStrFunc: Callable[[str], str]):
        """
        Prints out the exception message for why a particular .ini file or Blend.buf file has been skipped

        Parameters
        ----------
        assetName: :class:`str`
            The name for the type of asset (files, folders, mods, etc...) that was skipped

        assetDict: Dict[:class:`str`, :class:`Exception`]
            Locations of where exceptions have occured for the particular asset :raw-html:`<br />` :raw-html:`<br />`

            The keys are the absolute folder paths to where the exception occured

        wantStrFunc: Callable[[:class:`str`], :class:`str`]
            Function for how we want to print out the warning for each exception :raw-html:`<br />` :raw-html:`<br />`

            Takes in the folder location of where the exception occured as a parameter
        """

        if (assetDict):
            message = f"\nWARNING: The following {assetName} were skipped due to warnings (see log above):\n\n"
            for dir in assetDict:
                message += warnStrFunc(dir)

            self.logger.error(message)
            self.logger.space()

    def warnSkippedIniResource(self, modPath: str, stats: FileStats):
        """
        Prints out all of the resource files from the .ini files that were skipped due to exceptions

        Parameters
        ----------
        modPath: :class:`str`
            The absolute path to a particular folder
        """

        parentFolder = os.path.dirname(self._path)
        relModPath = FileService.getRelPath(modPath, parentFolder)
        modHeading = Heading(f"Mod: {relModPath}", 5)
        message = f"{modHeading.open()}\n\n"
        fileWarnings = stats.skippedByMods[modPath]
        
        for filePath in fileWarnings:
            relBlendPath = FileService.getRelPath(filePath, self._path)
            message += self.logger.getBulletStr(f"{relBlendPath}:\n\t{Heading(type(fileWarnings[filePath]).__name__, 3, '-').open()}\n\t{fileWarnings[filePath]}\n\n")
        
        message += f"{modHeading.close()}\n"
        return message

    def reportSkippedMods(self):
        """
        Prints out all of the mods that were skipped due to exceptions

        .. tip:: 
            For more info about how we define a 'mod', go to :class:`Mod`
        """

        self.reportSkippedAsset(f"newly added {FileTypes.Texture.value} files", self.stats.texAdd.skippedByMods, lambda dir: self.warnSkippedIniResource(dir, self.stats.texAdd))
        self.reportSkippedAsset(f"editted {FileTypes.Texture.value} files", self.stats.texEdit.skippedByMods, lambda dir: self.warnSkippedIniResource(dir, self.stats.texEdit))
        self.reportSkippedAsset(f"{FileTypes.Ini.value}s", self.stats.ini.skipped, lambda file: self.logger.getBulletStr(f"{file}:\n\t{Heading(type(self.stats.ini.skipped[file]).__name__, 3, '-').open()}\n\t{self.stats.ini.skipped[file]}\n\n"))
        self.reportSkippedAsset(f"{FileTypes.Blend.value} files", self.stats.blend.skippedByMods, lambda dir: self.warnSkippedIniResource(dir, self.stats.blend))
        self.reportSkippedAsset(f"{FileTypes.Position.value}, files", self.stats.position.skippedByMods, lambda dir: self.warnSkippedIniResource(dir, self.stats.position))
        self.reportSkippedAsset("mods", self.stats.mod.skipped, lambda dir: self.logger.getBulletStr(f"{dir}:\n\t{Heading(type(self.stats.mod.skipped[dir]).__name__, 3, '-').open()}\n\t{self.stats.mod.skipped[dir]}\n\n"))

    def reportSummary(self):
        skippedMods = len(self.stats.mod.skipped)
        fixedMods = len(self.stats.mod.fixed)
        foundMods = fixedMods + skippedMods

        fixedBlends = len(self.stats.blend.fixed)
        skippedBlends = len(self.stats.blend.skipped)
        removedRemapBlends = len(self.stats.blend.removed)
        foundBlends = fixedBlends + skippedBlends

        fixedPositions = len(self.stats.position.fixed)
        skippedPositions = len(self.stats.position.skipped)
        removedRemapPositions = len(self.stats.position.removed)
        foundPositions = fixedPositions + skippedPositions

        fixedInis = len(self.stats.ini.fixed)
        skippedInis = len(self.stats.ini.skipped)
        undoedInis = len(self.stats.ini.undoed)
        foundInis = fixedInis + skippedInis

        fixedAddTextures = len(self.stats.texAdd.fixed)
        skippedAddTextures = len(self.stats.texAdd.skipped)
        removedTextures = len(self.stats.texAdd.removed)
        foundAddTextures = fixedAddTextures + skippedAddTextures

        fixedEditTextures = len(self.stats.texEdit.fixed)
        skippedEditTextures = len(self.stats.texEdit.skipped)
        foundEditTextures = fixedEditTextures + skippedEditTextures

        downloadedFiles = len(self.stats.download.fixed)
        cachedDownloadedFiles = len(self.stats.download.hit)
        skippedDownloads = len(self.stats.download.skipped)
        foundDownloads = downloadedFiles + cachedDownloadedFiles + skippedDownloads
        removedDownloads = len(self.stats.download.removed)

        self.logger.openHeading("Summary", sideLen = 10)
        self.logger.space()
        
        modFixMsg = ""
        blendFixMsg = ""
        positionFixMsg = ""
        iniFixMsg = ""
        removedRemapBlendMsg = ""
        removedRemapPositionMsg = ""
        undoedInisMsg = ""
        texAddFixMsg = ""
        texEditFixMsg = ""
        removedTexMsg = ""
        downloadMsg = ""
        removedDownloadMsg = ""

        if (not self.undoOnly):
            modFixMsg = f"Out of {foundMods} found mods, fixed {fixedMods} mods and skipped {skippedMods} mods"
            iniFixMsg = f"Out of the {foundInis} {FileTypes.Ini.value}s within the found mods, fixed {fixedInis} {FileTypes.Ini.value}s and skipped {skippedInis} {FileTypes.Ini.value}s"
            blendFixMsg = f"Out of the {foundBlends} {FileTypes.Blend.value} files within the found mods, fixed {fixedBlends} {FileTypes.Blend.value} files and skipped {skippedBlends} {FileTypes.Blend.value} files"

            if (foundPositions > 0):
                positionFixMsg = f"Out of the {foundPositions} {FileTypes.Position.value} files within the found mods, fixed {fixedPositions} {FileTypes.Position.value} files and skipped {skippedPositions} {FileTypes.Position.value} files"

            if (foundAddTextures > 0):
                texAddFixMsg = f"Out of the {foundAddTextures} {FileTypes.Texture.value} files that were attempted to be created in the found mods, created {fixedAddTextures} {FileTypes.Texture.value} files and skipped {skippedAddTextures} {FileTypes.Texture.value} files"

            if (foundEditTextures > 0):
                texEditFixMsg = f"Out of the {foundEditTextures} {FileTypes.Texture.value} files within the found mods, editted {fixedEditTextures} {FileTypes.Texture.value} files and skipped {skippedEditTextures} {FileTypes.Texture.value} files"

            if (foundDownloads > 0):
                downloadMsg = f"Out of {foundDownloads} download requests within the found mods, downloaded {downloadedFiles} files, copied {cachedDownloadedFiles} files from existing downloads and skipped {skippedDownloads} downloads"
        else:
            modFixMsg = f"Out of {foundMods} found mods, remove fix from {fixedMods} mods and skipped {skippedMods} mods"

        if (not self.fixOnly and undoedInis > 0):
            undoedInisMsg = f"Removed fix from up to {undoedInis} {FileTypes.Ini.value}s"

            if (self.undoOnly):
                undoedInisMsg += f" and skipped {skippedInis} {FileTypes.Ini.value}s"

        if (not self.fixOnly and removedRemapBlends > 0):
            removedRemapBlendMsg = f"Removed {removedRemapBlends} old {FileTypes.RemapBlend.value} files"

        if (not self.fixOnly and removedRemapPositions > 0):
            removedRemapPositionMsg = f"Removed {removedRemapPositions} old {FileTypes.RemapPosition.value} files"

        if (not self.fixOnly and removedTextures > 0):
            removedTexMsg = f"Removed {removedTextures} old {FileTypes.RemapTexture.value} files"

        if (not self.fixOnly and removedDownloads > 0):
            removedDownloadMsg = f"Removed {removedDownloads} old {FileTypes.RemapDownload.value} files"


        self.logger.bulletPoint(modFixMsg)
        if (iniFixMsg):
            self.logger.bulletPoint(iniFixMsg)

        if (blendFixMsg):
            self.logger.bulletPoint(blendFixMsg)

        if (positionFixMsg):
            self.logger.bulletPoint(positionFixMsg)

        if (texAddFixMsg):
            self.logger.bulletPoint(texAddFixMsg)

        if (texEditFixMsg):
            self.logger.bulletPoint(texEditFixMsg)

        if (downloadMsg):
            self.logger.bulletPoint(downloadMsg)

        if (undoedInisMsg):
            self.logger.bulletPoint(undoedInisMsg)

        if (removedRemapBlendMsg):
            self.logger.bulletPoint(removedRemapBlendMsg)

        if (removedRemapPositionMsg):
            self.logger.bulletPoint(removedRemapPositionMsg)

        if (removedTexMsg):
            self.logger.bulletPoint(removedTexMsg)

        if (removedDownloadMsg):
            self.logger.bulletPoint(removedDownloadMsg)

        self.logger.space()
        self.logger.closeHeading()

    def createLog(self):
        """
        Creates a log text file that contains all the text printed on the command line
        """

        if (self._log is None):
            return

        self.logger.includePrefix = False
        self.logger.space()

        self.logger.log(f"Creating log file, {FileTypes.Log.value}")

        self.logger.includePrefix = True

        with open(self._log, "w", encoding = FileEncodings.UTF8.value) as f:
            f.write(self.logger.loggedTxt)

    def createMod(self, path: Optional[str] = None, files: Optional[List[str]] = None) -> Mod:
        """
        Creates a mod

        .. tip:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        path: Optional[:class:`str`]
            The absolute path to the mod folder. :raw-html:`<br />` :raw-html:`<br />`
            
            If this argument is set to ``None``, then will use the current directory of where this module is loaded

        files: Optional[List[:class:`str`]]
            The direct children files to the mod folder (does not include files located in a folder within the mod folder). :raw-html:`<br />` :raw-html:`<br />`

            If this parameter is set to ``None``, then the module will search the folders for you

        Returns
        -------
        :class:`Mod`
            The mod that has been created
        """

        path = FileService.getPath(path)
        mod = Mod(path = path, files = files, logger = self.logger, types = self.types, defaultType = self.defaultType, 
                  version = self.version, remappedTypes = self.remappedTypes, forcedType = self.forcedType, downloadMode = self.downloadMode)
        return mod

    def _fix(self):
        """
        The overall logic for fixing a bunch of mods

        For finding out which folders may contain mods, this function:
            #. recursively searches all folders from where the :attr:`RemapService.path` is located
            #. for every .ini file in a valid mod and every Blend.buf file encountered that is encountered, recursively search all the folders from where the .ini file or Blend.buf file is located

        .. tip:: 
            For more info about how we define a 'mod', go to :class:`Mod`
        """

        if (self.__errorsBeforeFix is not None):
            raise self.__errorsBeforeFix

        if (self.fixOnly and self.undoOnly):
            raise ConflictingOptions([CommandOpts.FixOnly.value, CommandOpts.Revert.value])

        parentFolder = os.path.dirname(self._path)
        self._loggerBasePrefix = os.path.basename(self._path)
        self.logger.prefix = os.path.basename(FilePathConsts.DefaultPath)

        visitedDirs = set()
        visitingDirs = set()
        gotNeighbours = set()
        dirs = deque()
        dirs.append(self._path)
        visitingDirs.add(self._path)

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
    
        while (dirs):
            path = dirs.pop()
            fixedMod = False

            # skip if the directory has already been visited
            if (path in visitedDirs):
                visitingDirs.remove(path)
                visitedDirs.add(path)
                continue 
            
            self.logger.split()

            # get the relative path to where the program runs
            self.logger.prefix = FileService.getRelPath(path, parentFolder)

            # try to make the mod, skip if cannot be made
            try:
                mod = self.createMod(path = path)
            except Exception as e:
                visitingDirs.remove(path)
                visitedDirs.add(path)
                continue
            
            # fix the mod
            try:
                fixedMod = self.fixMod(mod, flushIfTemplates = False)
            except Exception as e:
                self.logger.handleException(e)
                if (mod.inis):
                    self.stats.mod.addSkipped(path, e, modFolder = path)

            # get all the folders that could potentially be other mods
            modDirs = []
            if (path not in gotNeighbours):
                modFiles, modDirs = FileService.getFilesAndDirs(path = path, recursive = True)

            gotNeighbours.update(set(modDirs))

            if (mod.inis):
                iniModDirs = OrderedSet([])

                for iniPath in mod.inis:
                    ini = mod.inis[iniPath]
                    currentIniModDirs = ini.getReferencedFolders()

                    for folder in currentIniModDirs:
                        iniModDirs.add(folder)

                modDirs += list(iniModDirs)
            
            # add in all the folders that need to be visited
            for dir in modDirs:
                if (dir in visitedDirs):
                    continue

                if (dir not in visitingDirs):
                    dirs.append(dir)
                visitingDirs.add(dir)

            # increment the count of mods found
            if (fixedMod):
                self.stats.mod.addFixed(path)

            visitingDirs.remove(path)
            visitedDirs.add(path)

        self.logger.split()
        self.logger.prefix = self._loggerBasePrefix
        self.reportSkippedMods()
        self.logger.space()
        self.reportSummary()


    def fix(self):
        """
        Fixes a bunch of mods

        see :meth:`_fix` for more info
        """
        
        try:
            self._fix()
        except Exception as e:
            if (self.handleExceptions):
                self.logger.handleException(e)
            else:
                self.createLog()
                raise e from e
        else:
            noErrors = bool(not self.stats.mod.skipped and not self.stats.blend.skippedByMods)

            if (noErrors):
                self.logger.space()
                self.logger.log("ENJOY")

            self.logger.split()

            if (noErrors):
                self.addTips()

        self.createLog()
##### EndScript
