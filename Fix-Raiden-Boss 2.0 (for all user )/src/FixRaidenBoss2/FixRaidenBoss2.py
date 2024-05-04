# Author: NK#1321 raiden boss fix, if you used it to fix your raiden pls give credit for "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)
#   Albert Gold#2696 (for update the code for merged mods)


import os
import configparser
import re
import struct
import traceback
from typing import List, Callable, Optional, Union, Dict, Any, TypeVar, Hashable, Tuple, Set, DefaultDict
from collections import deque, defaultdict, OrderedDict
from functools import cmp_to_key
from enum import Enum
import argparse
import ntpath


# change our current working directory to this file, allowing users to run program
#   by clicking on the script instead of running by CLI
if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

DefaultFileType = "file"
DefaultPath = os.getcwd()
CurrentDir = "."
IniExt = ".ini"
TxtExt = ".txt"
BufExt = ".buf"
IniExtLen = len(IniExt)
MergedFile = f"merged{IniExt}"
BackupFilePrefix = "DISABLED_BossFixBackup_"
DuplicateFilePrefix = "DISABLED_RSDup_"
LogFile = f"RSFixLog{TxtExt}"

IniFileType = "*.ini file"
BlendFileType = f"Blend{BufExt}"
RemapBlendFile = f"Remap{BlendFileType}"
IniFileEncoding = "utf-8"
ReadEncodings = [IniFileEncoding, "latin1"]

Deprecated = "DEPRECATED"

DeleteBackupOpt = '--deleteBackup'
FixOnlyOpt = '--fixOnly'
RevertOpt = '--revert'
AllOpt = '--all'
TypeOpt = "--types"


# BossFixFormatter: Text formatting for the help page of the command 
class BossFixFormatter(argparse.MetavarTypeHelpFormatter, argparse.RawTextHelpFormatter):
    pass

# ConfigParserDict: Dictionary used to only keep the value of the first instance of a key
class ConfigParserDict(OrderedDict):
    def __setitem__(self, key, value):
        # All values updated into the dictionary of ConfigParser will first updated as a list of values, then
        #    the list of values will be turned into a string
        #
        # eg. the 'value' argument for the __setitem__ method in the case a key has 2 duplicates
        # >> value = ["val1"]           <----------- we only want this list
        # >> value = ["val1", "", "val2"]
        # >> value = ["val1", "", "val2", "", "val3"]
        # >> value = "val1\nval2\nval3"
        #
        # Note:
        #   For the case of duplicate keys, GIMI will only keep the value of the first valid instance of the key.
        #       Since checking for correct syntax and semantics is out of the scope of this program, we only get 
        #        the value of the first instance of the key
        if (key in self and isinstance(self[key], list) and isinstance(value, list)):
            return

        super().__setitem__(key, value)

argParser = argparse.ArgumentParser(description='Fixes Raiden Boss Phase 1 for all types of mods', formatter_class=BossFixFormatter)
argParser.add_argument('-s', '--src', action='store', type=str, help="The starting path to run this fix. If this option is not specified, then will run the fix from the current directory.")
argParser.add_argument('-d', DeleteBackupOpt, action='store_true', help=f'deletes backup copies of the original {IniExt} files')
argParser.add_argument('-f', FixOnlyOpt, action='store_true', help='only fixes the mod without cleaning any previous runs of the script')
argParser.add_argument('-r', RevertOpt, action='store_true', help='reverts back previous runs of the script')
argParser.add_argument('-l', '--log', action='store', type=str, help=f'The folder location to log the printed out text into a seperate {TxtExt} file. If this option is not specified, then will not log the printed out text.')
argParser.add_argument('-a', AllOpt, action='store_true', help=f'Parses all {IniFileType}s that the program encounters. This option supersedes the {TypeOpt} option')
argParser.add_argument('-n', '--defaultType', action='store', type=str, help=f'''The default mod type to use if the {IniFileType} belongs to some unknown mod
If the {AllOpt} is set to True, then this argument will be 'raiden'.
Otherwise, if this value is not specified, then any mods with unknown types will be skipped

See below for the different names/aliases of the supported types of mods.''')
argParser.add_argument('-t', TypeOpt, action='store', type=str, help=f'''Parses {IniFileType}s that the program encounters for only specific types of mods. If the {AllOpt} option has been specified, this option has no effect. 
By default, if this option is not specified, will parse the {IniFileType}s for all the supported types of mods. 

Please specify the types of mods using the the mod type's name or alias, then seperate each name/alias with a comma(,)
eg. raiden,arlecchino,ayaya

See below for the different names/aliases of the supported types of mods.''')

T = TypeVar('T')
Pattern = TypeVar('Pattern')
TextIoWrapper = TypeVar('TextIoWrapper')


class Error(Exception):
    """
    The base exception used by this module

    Parameters
    ----------
    message: :class:`str`
        the error message to print out
    """

    def __init__(self, message: str):
        super().__init__(f"ERROR: {message}")


class FileException(Error):
    """
    This Class inherits from :class:`Error`

    Exceptions relating to files

    Parameters
    ----------
    message: :class:`str`
        The error message to print out

    path: Optional[:class:`str`]
        The path where the error for the file occured. If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, message: str, path: Optional[str] = None):
        path = FileService.getPath(path)

        if (path != DefaultPath):
            message += f" at {path}"

        super().__init__(message)


class DuplicateFileException(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when there are multiple files of the same type in a folder

    Parameters
    ----------
    files: List[:class:`str`]
        The files that triggered the exception

    fileType: :class:`str`
        The name for the type of files :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "file"

    path: Optional[:class:`str`]
        The path to the folder where the files are located If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    files: List[:class:`str`]
        The files that triggered the exception

    fileType: :class:`str`
        The name for the type of files

        **Default**: ``None``
    """

    def __init__(self, files: List[str], fileType: str = DefaultFileType, path: Optional[str] = None):
        path = FileService.getPath(path)
        self.files = files
        self.fileType = fileType
        message = f"Ensure only one {fileType} exists"
        super().__init__(message, path = path)


class MissingFileException(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a certain type of file is missing from a folder

    Parameters
    ----------
    fileType: :class:`str`
        The type of file searching in the folder :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "file"

    path: :class:`str`
        The path to the folder that is being searched. If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    fileType: :class:`str`
        The type of file searching in the folder
    """
    def __init__(self, fileType: str = DefaultFileType, path: Optional[str] = None):
        path = FileService.getPath(path)
        message = f"Unable to find {fileType}. Ensure it is in the folder"
        self.fileType = fileType
        super().__init__(message, path = path)


class RemapMissingBlendFile(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a RemapBlend.buf file is missing its corresponding Blend.buf file

    Parameters
    ----------
    remapBlend: :class:`str`
        The path to the RemapBlend.buf file
    """

    def __init__(self, remapBlend: str):
        super().__init__(f"Missing the corresponding Blend.buf file for the RemapBlend.buf", path = remapBlend)


class BlendFileNotRecognized(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a Blend.buf file cannot be read

    Parameters
    ----------
    blendFile: :class:`str`
        The file path to the Blend.buf file
    """
    def __init__(self, blendFile: str):
        super().__init__(f"Blend file format not recognized for {os.path.basename(blendFile)}", path = os.path.dirname(blendFile))

class BadBlendData(Error):
    """
    This Class inherits from :class:`Error`

    Exception when certain bytes do not correspond to the format defined for a Blend.buf file
    """

    def __init__(self):
        super().__init__(f"Bytes do not corresponding to the defined format for a Blend.buf file")


class ConflictingOptions(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the script or :class:`BossFixService` is ran with options that cannot be used together

    Parameters
    ----------
    options: List[:class:`str`]
        The options that cannot be used together
    """
    def __init__(self, options: List[str]):
        optionsStr = ", ".join(options)
        super().__init__(f"The following options cannot be used toghether: {optionsStr}")

class InvalidModType(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the type of mod specified to fix is not found

    Parameters
    ----------
    type: :class:`str`
        The name for the type of mod specified
    """
    def __init__(self, type: str):
        super().__init__(f"Unable to find the type of mod by the search string, '{type}'")

class NoModType(Error):
    """
    This Class inherits from :class:`Error`

    Exception when trying to fix a mod of some unidentified mod type

    Parameters
    ----------
    type: :class:`str`
        The name for the type of mod specified 
    """

    def __init__(self):
        super().__init__(f"No mod type specified when fixing the .ini file")


class DictTools():
    """
    Tools for handling with Dictionaries
    """

    @classmethod
    def getFirstKey(cls, dict: Dict[Any, Any]) -> Any:
        """
        Retrieves the first key in a dictionary

        Parameters
        ----------
        dict: Dict[Any, Any]
            The dictionary we are working with

            .. note::
                The dictionary must not be empty

        Returns
        -------
        Any
            The first key of the dictionary
        """

        return next(iter(dict))

    @classmethod
    def getFirstValue(cls, dict: Dict[Any, Any]) -> Any:
        """
        Retrieves the first value in a dictionary

        Parameters
        ----------
        dict: Dict[Any, Any]
            The dictionary we are working with

        Returns
        -------
        Any
            The first value of the dictionary
        """

        return dict[cls.getFirstKey(dict)]
    
    # combine(dst, src, combine_duplicate): Combines dictionaries to 'dst'
    @classmethod
    def combine(cls, dict1: Dict[Hashable, Any], dict2: Dict[Hashable, Any], combineDuplicate: Optional[Callable[[Any, Any], Any]] = None) -> Dict[Hashable, Any]:
        """
        Combines 2 dictionaries

        Parameters
        ----------
        dict1: Dict[Hashable, Any]
            The destination of where we want the combined dictionaries to be stored

        dict2: Dict[Hashable, Any]
            The that we want to combine values with

        combineDuplicate: Optional[Callable[[Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries

            If this value is set to ``None``, then will use the key from 'dict2' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """
        new_dict = {**dict1, **dict2}
        if (combineDuplicate is None):
            return new_dict

        for key, value in new_dict.items():
            if key in dict1 and key in dict2:
                new_dict[key] = combineDuplicate(value, dict1[key])
        return new_dict


class FileService():
    """
    Tools for handling with files and folders :raw-html:`<br />` :raw-html:`<br />`
    """

    @classmethod
    def getFilesAndDirs(cls, path: Optional[str] = None, recursive: bool = False) -> List[List[str]]:
        """
        Retrieves the files and folders contained in a certain folder

        Parameters
        ----------
        path: Optional[:class:`str`]
            The path to the target folder we are working with. If this argument is ``None``, then will use the current directory of where this module is loaded
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        recursive: :class:`bool`
            Whether to recursively check all the folders from our target folder :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        [List[:class:`str`], List[:class:`str`]]
            The files and directories within the folder. The order for the result is:

            #. files
            #. folders
        """
        path = cls.getPath(path)
        files = []
        dirs = []

        pathItems = []
        
        if (recursive):
            for root, currentDirs, currentFiles in os.walk(path, topdown = True):
                for dir in currentDirs:
                    dirs.append(os.path.join(root, dir))

                for file in currentFiles:
                    files.append(os.path.join(root, file))

            return [files, dirs]
        
        pathItems = os.listdir(path)
        for itemPath in pathItems:
            fullPath = os.path.join(path, itemPath)
            if (os.path.isfile(fullPath)):
                files.append(fullPath)
            else:
                dirs.append(fullPath)

        return [files, dirs]

    # filters and partitions the files based on the different filters specified
    @classmethod
    def getFiles(cls, path: Optional[str] = None, filters: Optional[List[Callable[[str], bool]]] = None, files: Optional[List[str]] = None) -> Union[List[str], List[List[str]]]:
        """
        Retrieves many different types of files within a folder

        .. note::
            Only retrieves files that are the direct children of the folder (will not retrieve files nested in a folder within the folder we are searching)

        Parameters
        ----------
        path: Optional[:class:`str`]
            The path to the target folder we are working with. If this value is set to ``None``, then will use the current directory of where this module is loaded
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        filters: Optional[List[Callable[[:class:`str`], :class:`bool`]]]
            Different filter functions for each type of file we are trying to get. If this values is either ``None`` or ``[]``, then will default to a filter to get all the files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        files: Optional[List[:class:`str`]]
            The files contained in the target folder

            If this value is set to ``None``, then the function will search for the files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Union[List[:class:`str`], List[List[:class:`str`]]]
            The files partitioned into the different types specified by the filters

            If 'filters' only has 1 element, then the function returns List[:class:`str`]
            Otherwise, will return List[List[:class:`str`]]
        """

        path = cls.getPath(path)
        result = []

        if (filters is None):
            filters = []

        if (not filters):
            filters.append(lambda itemPath: True)

        filtersLen = len(filters)
        usePathFiles = False
        if (files is None):
            files = os.listdir(path)
            usePathFiles = True

        for i in range(filtersLen):
            result.append([])
        
        for itemPath in files:
            for filterInd in range(filtersLen):
                pathFilter = filters[filterInd]
                if (not pathFilter(itemPath) or (usePathFiles and not os.path.isfile(os.path.join(path, itemPath)))):
                    continue

                fullPath = os.path.join(path, itemPath)

                result[filterInd].append(fullPath)

        if (filtersLen == 1):
            return result[0]
        
        return result
    
    # retrieves only a single file for each filetype specified by the filters
    @classmethod
    def getSingleFiles(cls, path: Optional[str] = None, filters: Optional[Dict[str, Callable[[str], bool]]] = None, files: Optional[List[str]] = None, optional: bool = False) -> Union[Optional[str], List[str], List[Optional[str]]]:
        """
        Retrieves exactly 1 of each type of file in a folder

        Parameters
        ----------
        path: Optional[:class:`str`]
            The path to the target folder we are searching. :raw-html:`<br />` :raw-html:`<br />`
            
            If this value is set to ``None``, then will use the current directory of where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        filters: Optional[Dict[str, Callable[[:class:`str`], :class:`bool`]]]
            Different filter functions for each type of file we are trying to get. If this value is ``None`` or ``{}``, then will default to use a filter to get all files

            The keys are the names for the file type :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        files: Optional[List[:class:`str`]]
            The files contained in the target folder

            If this value is set to ``None``, then the function will search for the files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        optional: :class:`bool`
            Whether we want to send an exception if there is not exactly 1 file for a certain type of file :raw-html:`<br />` :raw-html:`<br />`

            #. If this value is ``False`` and there are no files for a certain type of file, then will raise a :class:`MissingFileException`
            #. If this value is ``False`` and there are more than 1 file for a certain type of file, then will raise a :class:`DuplicateFileException`
            #. If this value is ``True`` and there are no files for a certain type of file, then the file for that type of file will be ``None``
            #. If this value is ``True`` and there are more than 1 file for a certain type of file, then will retrieve the first file for that type of file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Raises
        ------
        :class:`MissingFileException`
            if ``optional`` is set to ``False`` and there are not files for a certain type of file

        :class:`DuplicateFileException`
            if ``optional`` is set to ``False`` and there are more than 1 file for a certain type of file

        Returns
        -------
        Union[Optional[:class:`str`], List[:class:`str`], List[Optional[:class:`str`]]]
            The files partitioned for each type of file

            * If ``filters`` only contains 1 element and ``optional`` is ``False``, then will return :class:`str`
            * If ``filters`` contains more than 1 element and ``optional`` is ``False`, then will return List[:class:`str`]
            * If ``filters`` only contains 1 element and ``optional`` is ``True``, then will return Optional[:class:`str`]
            * Otherwise, returns List[Optional[:class:`str`]]
        """
        path = cls.getPath(path)
        if (filters is None):
            filters = {}

        if (not filters):
            filters[DefaultFileType] = lambda itemPath: True
        
        filesPerFileTypes = cls.getFiles(path = path, filters = list(filters.values()), files = files)
        filtersLen = len(filters)

        onlyOneFilter = filtersLen == 1
        if (onlyOneFilter):
            filesPerFileTypes = [filesPerFileTypes]

        result = []
        i = 0
        for fileType in filters:
            fileTypeFiles = filesPerFileTypes[i]
            filesLen = len(fileTypeFiles)

            if (not optional and not filesLen):
                raise MissingFileException(fileType = fileType, path = path)
            elif (not optional and filesLen > 1):
                raise DuplicateFileException(fileTypeFiles, fileType = fileType, path = path)
            
            if (fileTypeFiles):
                result.append(fileTypeFiles[0])
            else:
                result.append(None)
            i += 1

        if (onlyOneFilter):
            return result[0]
        
        return result
    
    @classmethod
    def rename(cls, oldFile: str, newFile: str):
        """
        Renames a file

        .. warning::
            If the new name for the file already exists, then the function deletes
            the file with the new name and renames the target file with the new name

        Parameters
        ----------
        oldFile: :class:`str`
            file path to the target file we are working with

        newFile: :class:`str`
            new file path for the target file 
        """
        if (oldFile == newFile):
            return

        try:
            os.rename(oldFile, newFile)
        except FileExistsError:
            os.remove(newFile)
            os.rename(oldFile, newFile)

    @classmethod
    def changeExt(cls, file: str, newExt: str) -> str:
        """
        Changes the extension for a file

        Parameters
        ----------
        file: :class:`str`
            The file path to the file we are working with

        newExt: :class:`str`
            The name of the new extension for the file (without the dot at front)

        Returns
        -------
        :class:`str`
            the new file path with the extension changed
        """

        dotPos = file.rfind(".")

        if (not newExt.startswith(".")):
            newExt = f".{newExt}"

        if (dotPos != -1):
            file = file[:dotPos] + newExt

        return file

    @classmethod
    def disableFile(cls, file: str, filePrefix: str = BackupFilePrefix):
        """
        Marks a file as 'DISABLED' and changes the file to a .txt file

        Parameters
        ----------
        file: :class:`str`
            The file path to the file we are working with

        filePrefix: :class:`str`
            Prefix name we want to add in front of the file name :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "DISABLED_BossFixBackup\_"
        """

        baseName = os.path.basename(file)
        baseName = FileService.changeExt(baseName, TxtExt)

        backupFile = os.path.join(os.path.dirname(file), filePrefix + baseName)
        FileService.rename(file, backupFile)

    @classmethod
    def parseOSPath(cls, path: str):
        """
        Retrieves a normalized file path from a string

        Parameters
        ----------
        path: :class:`str`
            The string containing some sort of file path
        """

        result = ntpath.normpath(path)
        result = cls.ntPathToPosix(result)
        return result

    @classmethod
    def ntPathToPosix(cls, path: str) -> str:
        """
        Converts a file path from the `ntpath <https://opensource.apple.com/source/python/python-3/python/Lib/ntpath.py.auto.html>`_ library to a file path for the `os <https://docs.python.org/3/library/os.html>`_ library

        .. note::
            The character for the folder paths (``/`` or ``\\``) used in both libraries may be different depending on the OS

        Parameters
        ----------
        path: :class:`str`
            The file path we are working that is generated from the 'ntpath' library

        Returns
        -------
        :class:`str`
            The file path generated by the 'os' library
        """

        return path.replace(ntpath.sep, os.sep)
    
    @classmethod
    def absPathOfRelPath(cls, dstPath: str, relFolder: str) -> str:
        """
        Retrieves the absolute path of the relative path of a file with respect to a certain folder

        Parameters
        ----------
        dstPath: :class:`str`
            The target file path we are working with

        relFolder: :class:`str`
            The folder that the target file path is relative to

        Returns
        -------
        :class:`str`
            The absolute path for the target file
        """

        relFolder = os.path.abspath(relFolder)
        result = dstPath
        if (not os.path.isabs(result)):
            result = os.path.join(relFolder, result)

        return cls.parseOSPath(result)
    
    @classmethod
    def getRelPath(cls, path: str, start: str) -> str:
        """
        Tries to get the relative path of a file/folder relative to another folder, if possible.

        If it is not possible to get the relative path, will return back the original file path

        .. note::
            An example where it would not be possible to get the relative path would be:
            
            * If the file is located in one mount (eg. C:/ drive) and the folder is located in another mount (eg. D:/ drive)

        Parameters
        ----------
        path: :class:`str`
            The path to the target file/folder we are working with

        start: :class:`str`
            The path that the target file/folder is relative to

        Returns
        -------
        :class:`str`
            Either the relative path or the original path if not possible to get the relative paths
        """

        result = path
        try:
            result = os.path.relpath(path, start)

        # if the path is in another mount than 'start'
        except ValueError:
            pass

        return cls.parseOSPath(result)
    
    # read(file, fileCode, postProcessor): Tries to read a file using different encodings
    @classmethod
    def read(cls, file: str, fileCode: str, postProcessor: Callable[[TextIoWrapper], Any]) -> Any:
        """
        Tries to read a file using different file encodings

        Will interact with the file using the following order of encodings:

        #. utf-8 
        #. latin1

        Parameters
        ----------
        file: :class:`str`
            The file we are trying to read from

        fileCode: :class:`str`
            What `file mode <https://docs.python.org/3/tutorial/inputoutput.html#reading-and-writing-files>`_ to interact with the file (eg. r, rb, r+, etc...)

        postProcessor: Callable[[`TextIoWrapper`_], Any]
            A function used to process the file pointer of the opened file

        Returns
        -------
        Any
            The result after processing the file pointer of the opened file
        """

        error = None
        for encoding in ReadEncodings:
            try:
                with open(file, fileCode, encoding = encoding) as f:
                    return postProcessor(f)
            except UnicodeDecodeError as e:
                error = e

        if (error is not None):
            raise UnicodeDecodeError(f"Cannot decode the file using any of the following encodings: {ReadEncodings}")

    @classmethod
    def getPath(cls, path: Optional[str]) -> str:
        if (path is None):
            return DefaultPath
        return path



class Heading():
    """
    Class for handling information about a heading for pretty printing

    Examples
    --------

    .. code-block:: python
        :linenos:
        :emphasize-lines: 1,3

        ======= Title: Fix Raiden Boss 2 =======
        ...
        ========================================

    Parameters
    ----------
    title: :class:`str`
        The title for the heading :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    sideLen: :class:`int`
        The number of characters we want one side for the border of the opening heading to have :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 0

    sideChar: :class:`str`
        The type of character we want the border for the heading to have  :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "="

    Attributes
    ----------
    title: :class:`str`
        The title for the heading

    sideLen: :class:`int`
        The number of characters we want one side for the border of the opening heading to have

    sideChar: :class:`str`
        The type of character we want the border for the heading to have
    """

    def __init__(self, title: str = "", sideLen: int = 0, sideChar: str = "="):
        self.title = title
        self.sideLen = sideLen
        self.sideChar = sideChar

    def copy(self):
        """
        Makes a new copy of a heading

        Returns
        -------
        :class:`Heading`
            The new copy of the heading
        """
        return Heading(title = self.title, sideLen = self.sideLen, sideChar = self.sideChar)

    def open(self) -> str:
        """
        Makes the opening heading (see line 1 of the example at :class:`Heading`)

        Returns
        -------
        :class:`str`
            The opening heading created
        """

        side = self.sideLen * self.sideChar
        return f"{side} {self.title} {side}"

    def close(self) -> str:
        """
        Makes the closing heading (see line 3 of the example at :class:`Heading`)

        Returns
        -------
        :class:`str`
            The closing heading created
        """

        return self.sideChar * (2 * (self.sideLen + 1) + len(self.title))


class Logger():
    """
    Class for pretty printing output to display on the console

    Parameters
    ----------
    prefix: :class:`str`
        line that is printed before any message is printed out :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    verbose: :class:`bool`
        Whether to print out output :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    includePrefix: :class:`bool`
        Whether to include the prefix string when printing out a message

    verbose: :class:`bool`
        Whether to print out output

    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done

    _prefix: :class:`str`
        line that is printed before any message is printed out

    _headings: Deque[:class:`Heading`]
        A stack of headings that have been opened (by calling :meth:`Heading.open`), but have not been closed yet (have not called :meth:`Heading.close` yet)

    _loggedTxt: :class:`str`
        The text that will be logged into a .txt file
    """

    DefaultHeadingSideLen = 2
    DefaultHeadingChar = "="

    def __init__(self, prefix: str = "", logTxt: bool = False, verbose: bool = True):
        self._prefix = prefix
        self.includePrefix = True
        self.verbose = verbose
        self.logTxt = logTxt
        self._loggedTxt = ""
        self._headings = deque()
        self._currentPrefixTxt = ""

        self._setDefaultHeadingAtts()

    @property
    def prefix(self):
        """
        The line of text that is printed before any message is printed out

        :getter: Returns such a prefix
        :setter: Sets up such a prefix for the logger
        :type: :class:`str`
        """
        return self._prefix
    
    @prefix.setter
    def prefix(self, newPrefix):
        self._prefix = newPrefix
        self._currentPrefixTxt = ""

    @property
    def loggedTxt(self):
        """
        The text to be logged into a .txt file

        :getter: Returns such a prefix
        :type: :class:`str`
        """
        return self._loggedTxt

    def _setDefaultHeadingAtts(self):
        """
        Sets the default attributes for printing out a header line
        """

        self._headingTxtLen = 0
        self._headingSideLen = self.DefaultHeadingSideLen
        self._headingChar = self.DefaultHeadingChar

    def _addLogTxt(self, txt: str):
        """
        Appends the text to the logged output to be printed to a .txt file

        Parameters
        ----------
        txt: :class:`str`
            The text to be added onto the logged output
        """

        if (self.logTxt):
            self._loggedTxt += f"{txt}\n"

    def getStr(self, message: str):
        """
        Retrieves the string to be printed out by the logger

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out

        Returns
        -------
        :class:`str`
            The transformed text that the logger prints out
        """

        return f"# {self._prefix} --> {message}"

    def log(self, message: str):
        """
        Regularly prints text onto the console

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """

        if (self.includePrefix):
            message = self.getStr(message)

        self._addLogTxt(message)
        self._currentPrefixTxt += f"{message}\n"

        if (self.verbose):
            print(message)

    def split(self):
        """
        Prints out a new line
        """

        if (self._currentPrefixTxt):
            self.log("\n")

    def space(self):
        """
        Prints out a space
        """
        self.log("")

    def openHeading(self, txt: str, sideLen: int = DefaultHeadingSideLen, headingChar = DefaultHeadingChar):
        """
        Prints out an opening heading

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        sideLen: :class:`int`
            How many characters we want for the side border of the heading :raw-html:`<br />`
            (see line 1 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: 2

        headingChar: :class:`str`
            The type of character used to print the side border of the heading :raw-html:`<br />`
            (see line 3 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "="
        """

        heading = Heading(title = txt, sideLen = sideLen, sideChar = headingChar)
        self._headings.append(heading)
        self.log(heading.open())

    def closeHeading(self):
        """
        Prints out a closing heading that corresponds to a previous opening heading printed (see line 3 of the example at :class:`Heading`)
        """

        if (not self._headings):
            return

        heading = self._headings.pop()
        self.log(heading.close())

    @classmethod
    def getBulletStr(self, txt: str) -> str:
        """
        Creates the string for an item in an unordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        Returns
        -------
        :class:`str`
            The text formatted as an item in an unordered list
        """
        return f"- {txt}"
    
    @classmethod
    def getNumberedStr(self, txt: str, num: int) -> str:
        """
        Creates the string for an ordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        num: :class:`str`
            The number we want to print out before the text for the ordered list

        Returns
        -------
        :class:`str`
            The text formatted as an item in an ordered list
        """
        return f"{num}. {txt}"

    def bulletPoint(self, txt: str):
        """
        Prints out an item in an unordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        """
        self.log(self.getBulletStr(txt))

    def list(self, lst: List[str], transform: Optional[Callable[[str], str]] = None):
        """
        Prints out an ordered list

        Parameters
        ----------
        lst: List[:class:`str`]
            The list of messages we want to print out

        transform: Optional[Callable[[:class:`str`], :class:`str`]]
            A function used to do any processing on each message in the list of messages

            If this parameter is ``None``, then the list of message will not go through any type of processing :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (transform is None):
            transform = lambda txt: txt

        lstLen = len(lst)
        for i in range(lstLen):
            newTxt = transform(lst[i])
            self.log(self.getNumberedStr(newTxt, i + 1))

    def box(self, message: str, header: str):
        """
        Prints the message to be sandwiched by the text defined in the argument, ``header``

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out

        header: :class:`str`
            The string that we want to sandwich our message against
        """

        self.log(header)

        messageList = message.split("\n")
        for messagePart in messageList:
            self.log(messagePart)

        self.log(header)

    def error(self, message: str):
        """
        Prints an error message

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """

        prevVerbose = self.verbose
        if (not self.logTxt):
            self.verbose = True

        self.space()

        self.box(message, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        self.space()
        self.verbose = prevVerbose

    def handleException(self, exception: Exception):
        """
        Prints the message for an error

        Parameters
        ----------
        exception: :class:`Exception`
            The error we want to handle
        """

        message = f"\n{type(exception).__name__}: {exception}\n\n{traceback.format_exc()}"
        self.error(message)

    def input(self, desc: str) -> str:
        """
        Handles user input from the console

        Parameters
        ----------
        desc: :class:`str`
            The question/description being asked to the user for input

        Returns
        -------
        :class:`str`
            The resultant input the user entered
        """

        if (self.includePrefix):
            desc = self.getStr(desc)

        self._addLogTxt(desc)
        result = input(desc)
        self._addLogTxt(f"Input: {result}")

        return result

    def waitExit(self):
        """
        Prints the message used when the script finishes running
        """

        prevIncludePrefix = self.includePrefix
        self.includePrefix = False
        self.input("\n== Press ENTER to exit ==")
        self.includePrefix = prevIncludePrefix 


# our model objects in MVC
class Model():
    """
    Generic class used for any data models in the fix

    Parameters
    ----------
    logger: Optional[:class:`Logger`]
        The logger used to print messages to the console :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    logger: Optional[:class:`Logger`]
        The logger used to print messages to the console
    """
    def __init__(self, logger: Optional[Logger] = None):
        self.logger = logger

    def print(self, funcName: str, *args, **kwargs):
        """
        Prints out output

        Parameters
        ----------
        funcName: :class:`str`
            The name of the function in the logger for printing out the output

        \*args: List[:class:`str`]
            Arguments to pass to the function in the logger

        \*\*kwargs: Dict[:class:`str`, Any]
            Keyword arguments to pass to the function in the logger

        Returns
        -------
        :class:`Any`
            The return value from running the corresponding function in the logger 
        """

        if (self.logger is not None):
            func = getattr(self.logger, funcName)
            return func(*args, **kwargs)


# Needed data model to inject into the .ini file
class RemapBlendModel():
    """
    Contains data for fixing a particular resource in a .ini file

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedBlendName: :class:`str`
        The new name of the resource once all the Blend.buf files for the resource has been fixed

    fixedBlendPaths: Dict[:class:`int`, :class:`str`]
        The file paths to the fixed RemapBlend.buf files for the resource
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource

    origBlendName: Optional[:class:`str`]
        The original name of the resource in the .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    origBlendPaths: Optional[Dict[:class:`int`, :class:`str`]]
        The file paths to the Blend.buf files for the resource
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedBlendName: :class:`str`
        The new name of the resource once all the Blend.buf files for the resource has been fixed

    fixedBlendPaths: Dict[:class:`int`, :class:`str`]
        The file paths to the fixed RemapBlend.buf files for the resource
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource

    origBlendName: Optional[:class:`str`]
        The original name of the resource in the .ini file

    origBlendPaths: Optional[Dict[:class:`int`, :class:`str`]]
        The file paths to the Blend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource

    fullPaths: Dict[:class:`int`, :class:`str`]
        The absolute paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource

    origFullPaths: Dict[:class:`int`, :class:`str`]
        The absolute paths to the Blend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource
    """

    def __init__(self, iniFolderPath: str, fixedBlendName: str, fixedBlendPaths: Dict[int, str], origBlendName: Optional[str] = None,
                 origBlendPaths: Optional[Dict[int, str]] = None):
        self.fixedBlendName = fixedBlendName
        self.fixedBlendPaths = fixedBlendPaths
        self.origBlendName = origBlendName
        self.origBlendPaths = origBlendPaths
        self.iniFolderPath = iniFolderPath

        self.fullPaths = {}
        self.origFullPaths = {}

        # retrieve the absolute paths
        for partIndex in self.fixedBlendPaths:
            path = self.fixedBlendPaths[partIndex]
            self.fullPaths[partIndex] = FileService.absPathOfRelPath(path, iniFolderPath)

        if (self.origBlendPaths is not None):
            for partIndex in self.origBlendPaths:
                path = self.origBlendPaths[partIndex]
                self.origFullPaths[partIndex] = FileService.absPathOfRelPath(path, iniFolderPath)


class ModType():
    """
    Class for defining a generic type of mod

    Parameters
    ----------
    name: :class:`str`
        The default name for the type of mod

    check: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
        The specific check used to identify the .ini file belongs to the specific type of mod when checking arbitrary line in a .ini file :raw-html:`<br />` :raw-html:`<br />`

        #. If this argument is a string, then will check if a line in the .ini file equals to this argument
        #. If this argument is a regex pattern, then will check if a line in the .ini file matches this regex pattern
        #. If this argument is a function, then will check if a line in the .ini file will make the function for this argument return `True`

    bossHash: :class:`str`
        The hash for the Vertex Group Blend of the boss

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    vgRemap: Optional[Dict[int, int]]
        Maps the blend indices from the vertex group of the mod's blend to the blend indices for the vertex group of the boss :raw-html:`<br />`
        If this value is ``None``, then the blend indices of the mod and the boss are one-to-one

        The keys are the blend indices in the mod and the values are the blend indices in the boss :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    name: :class:`str`
        The default name for the type of mod

    check: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
        The specific check used to identify the .ini file belongs to the specific type of mod when checking arbitrary line in a .ini file

    bossHash: :class:`str`
        The hash for the Vertex Group Blend of the boss

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod
    """

    def __init__(self, name: str, check: Union[str, Pattern, Callable[[str], bool]], bossHash:str, aliases: Optional[List[str]] = None, vgRemap: Optional[Dict[int, int]] = None ):
        self.name = name
        self.bossHash = bossHash

        self.check = check
        if (isinstance(check, str)):
            self._check = lambda line: line == check
        elif (callable(check)):
            self._check = check
        else:
            self._check = lambda line: bool(check.search(line))
        
        if (aliases is None):
            aliases = []
        self.aliases = list(set(aliases))
        
        self._maxVgIndex = None
        if (vgRemap is None):
            vgRemap = {}
        self.vgRemap = vgRemap

    @property
    def vgRemap(self) -> Dict[int, int]:
        """
        The mapping for remapping vertex group blend indices of the mod to the vertex group blend indices of the boss

        :getter: Returns whether the .ini file has already been fixed
        :setter: Sets a new mapping for remapping the indices
        :type: Dict[:class:`int`, :class:`int`]
        """

        return self._vgRemap

    @vgRemap.setter
    def vgRemap(self, newVgRemap: Dict[int, int]):
        self._vgRemap = newVgRemap
        if (self._vgRemap):
            self._maxVgIndex = max(list(self._vgRemap.keys()))
        else:
            self._maxVgIndex = None

    @property
    def maxVgIndex(self) -> Optional[int]:
        """
        The max vertex group blend index of the mod (key of the mapping) in :attr:`ModType.vgRemap`

        :getter: Returns the following index
        :type: Optional[:class:`int`]
        """

        return self._maxVgIndex

    def isName(self, name: str) -> bool:
        """
        Determines whether a certain name matches with the names defined for this type of mod

        Parameters
        ----------
        name: :class:`str`
            The name being searched

        Returns
        -------
        :class:`bool`
            Whether the searched name matches with the names for this type of mod
        """

        name = name.lower()
        if (self.name.lower() == name):
            return True
        
        for alias in self.aliases:
            if (alias.lower() == name):
                return True

        return False
    
    def isType(self, iniLine: str) -> bool:
        """
        Determines whether a line in the .ini file correponds with this mod type

        Parameters
        ----------
        iniLine: :class:`str`
            An arbitrary line in a .ini file

        Returns
        -------
        :class:`bool`
            Whether the line in the .ini file corresponds with this type of mod
        """

        return self._check(iniLine)
    

class ModTypes(Enum):
    """
    The supported types of mods that can be fixed

    Attributes
    ----------
    Raiden: :class:`ModType`
        **Raiden mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``[TextureOverride.*(Raiden|Shogun).*Blend]``
    """

    Raiden = ModType("Raiden", re.compile(r"^\s*\[\s*TextureOverride.*(Raiden|Shogun)((?!RemapBlend).)*Blend.*\s*\]"), "fe5c0180",
                     aliases = ["Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"], 
                     vgRemap = {0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 60, 9: 61, 10: 66, 11: 67,
                                12: 8, 13: 9, 14: 10, 15: 11, 16: 12, 17: 13, 18: 14, 19: 15, 20: 16, 21: 17,
                                22: 18, 23: 19, 24: 20, 25: 21, 26: 22, 27: 23, 28: 24, 29: 25, 30: 26, 31: 27,
                                32: 28, 33: 29, 34: 30, 35: 31, 36: 32, 37: 33, 38: 34, 39: 35, 40: 36, 41: 37,
                                42: 38, 43: 39, 44: 40, 45: 41, 46: 42, 47: 94, 48: 43, 49: 44, 50: 45, 51: 46,
                                52: 47, 53: 48, 54: 49, 55: 50, 56: 51, 57: 52, 58: 53, 59: 54, 60: 55, 61: 56,
                                62: 57, 63: 58, 64: 59, 65: 114, 66: 116, 67: 115, 68: 117, 69: 74, 70: 62, 71: 64,
                                72: 106, 73: 108, 74: 110, 75: 75, 76: 77, 77: 79, 78: 87, 79: 89, 80: 91, 81: 95,
                                82: 97, 83: 99, 84: 81, 85: 83, 86: 85, 87: 68, 88: 70, 89: 72, 90: 104, 91: 112,
                                92: 93, 93: 63, 94: 65, 95: 107, 96: 109, 97: 111, 98: 76, 99: 78, 100: 80, 101: 88,
                                102: 90, 103: 92, 104: 96, 105: 98, 106: 100, 107: 82, 108: 84, 109: 86, 110: 69,
                                111: 71, 112: 73, 113: 105, 114: 113, 115: 101, 116: 102, 117: 103})
    
    @classmethod
    def getAll(cls) -> Set[ModType]:
        """
        Retrieves a set of all the mod types available

        Returns
        -------
        Set[:class:`ModType`]
            All the available mod types
        """

        result = set()
        for modTypeEnum in cls:
            result.add(modTypeEnum.value)
        return result
    
    @classmethod
    def search(cls, name: str):
        """
        Searches a mod type based off the provided name

        Parameters
        ----------
        name: :class:`str`
            The name of the mod to search for

        Returns
        -------
        Optional[:class:`ModType`]
            The found mod type based off the provided name
        """

        result = None
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            if (modType.isName(name)):
                result = modType
                break
        
        return result
    
    @classmethod
    def getHelpStr(cls) -> str:
        result = ""
        helpHeading = Heading("supported types of mods", 15)
        result += f"{helpHeading.open()}\n\nThe names/aliases for the mod types are not case sensitive\n\n"

        modTypeHelpTxt = []
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            modTypeHeading = Heading(modType.name, 8, "-")

            currentHelpStr = f"{modTypeHeading.open()}"
            currentHelpStr += f"\n\nname: {modType.name}"
            
            if (modType.aliases):
                aliasStr = ", ".join(modType.aliases)
                currentHelpStr += f"\naliases: {aliasStr}"

            if (isinstance(modType.check, str)):
                currentHelpStr += f"\ndescription: check if the .ini file contains the section named, '{modType.check}'"
            elif (not callable(modType.check)):
                currentHelpStr += f"\ndescription: check if the .ini file contains a section matching the regex, {modType.check.pattern}"

            currentHelpStr += f"\n\n{modTypeHeading.close()}"
            modTypeHelpTxt.append(currentHelpStr)

        modTypeHelpTxt = "\n".join(modTypeHelpTxt)
        result += f"{modTypeHelpTxt}\n\n{helpHeading.close()}"
        return result

argParser.epilog = ModTypes.getHelpStr()


# IfTemplate: Data class for the if..else template of the .ini file
class IfTemplate():
    """
    Data for storing information about a `section`_ in a .ini file

    :raw-html:`<br />`

    .. note::
        Assuming every `if/else` clause must be on its own line, we have that an :class:`IfTemplate` have a form looking similar to this:

        .. code-block:: ini
            :linenos:
            :emphasize-lines: 1,2,5,7,12,16,17

            ...(does stuff)...
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                endif
            endif
            ...(does stuff)...
            ...(does stuff)...

        We split the above structure into parts where each part is either:

        #. **An If Part**: a single line containing the keywords "if", "else" or "endif" :raw-html:`<br />` **OR** :raw-html:`<br />`
        #. **A Content Part**: a group of lines that *"does stuff"*

        **Note that:** an :class:`ifTemplate` does not need to contain any parts containing the keywords "if", "else" or "endif". This case covers the scenario
        when the user does not use if..else statements for a particular `section`_
        
        Based on the above assumptions, we can assume that every ``[section]`` in a .ini file contains this :class:`IfTemplate`

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for element in x

            Iterates over all the parts of the :class:`IfTemplate`, ``x``

        .. describe:: x[num]

            Retrieves the part from the :class:`IfTemplate`, ``x``, at index ``num``

        .. describe:: x[num] = newPart

            Sets the part at index ``num`` of the :class:`IfTemplate`, ``x``, to have the value of ``newPart``

    :raw-html:`<br />`

    Parameters
    ----------
    parts: List[Union[:class:`str`, Dict[:class:`str`, Any]]]
        The individual parts of how we divided an :class:`IfTemplate` described above

    calledSubCommands: Optional[Dict[:class:`int`, :class:`str`]]
        Any other sections that this :class:`IfTemplate` references
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices to the part in the :class:`IfTemplate` that the section is called :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    parts: List[Union[:class:`str`, Dict[:class:`str`, Any]]]
        The individual parts of how we divided an :class:`IfTemplate` described above

    calledSubCommands: Optional[Dict[:class:`int`, :class:`str`]]
        Any other sections that this :class:`IfTemplate` references
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices to the part in the :class:`IfTemplate` that the section is called
    """

    def __init__(self, parts: List[Union[str, Dict[str, Any]]], calledSubCommands: Optional[Dict[int, str]] = None):
        self.parts = parts
        self.calledSubCommands = calledSubCommands

        if (calledSubCommands is None):
            self.calledSubCommands = {}

    def __iter__(self):
        return self.parts.__iter__()
    
    def __getitem__(self, key: int) -> Union[str, Dict[str, Any]]:
        return self.parts[key]
    
    def __setitem__(self, key: int, value: Union[str, Dict[str, Any]]):
        self.parts[key] = value

    def add(self, part: Union[str, Dict[str, Any]]):
        """
        Adds a part to the :class:`ifTemplate`

        Parameters
        ----------
        part: Union[:class:`str`, Dict[:class:`str`, Any]]
            The part to add to the :class:`IfTemplate`
        """
        self.parts.append(part)

    # find(pred, postProcessor): Searches each part in the if template based on 'pred'
    def find(self, pred: Optional[Callable[[Union[str, Dict[str, Any]]], bool]] = None, postProcessor: Optional[Callable[[Union[str, Dict[str, Any]]], Any]] = None) -> Dict[int, Any]:
        """
        Searches the :class:`IfTemplate` for parts that meet a certain condition

        Parameters
        ----------
        pred: Optional[Callable[[Union[:class:`str`, Dict[:class:`str`, Any]]], :class:`bool`]]
            The predicate used to filter the parts :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then this function will return all the parts :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        postProcessor: Optional[Callable[[Union[:class:`str`, Dict[str, Any]]], Any]]
            A function that performs any post-processing on the found part that meets the required condition :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``

        Returns
        -------
        Dict[:class:`int`, Any]
            The filtered parts that meet the search condition :raw-html:`<br />` :raw-html:`<br />`

            The keys are the index locations of the parts and the values are the found parts
        """

        result = {}
        if (pred is None):
            pred = lambda part: True

        if (postProcessor is None):
            postProcessor = lambda part: part

        partsLen = len(self.parts)
        for i in range(partsLen):
            part = self.parts[i]
            if (pred(part)):
                result[i] = (postProcessor(part))

        return result


# IniFile: Class to handle .ini files
class IniFile(Model):
    """
    This class inherits from :class:`Model`

    Class for handling .ini files

    :raw-html:`<br />`

    .. note::
        We analyse the .ini file using Regex which is **NOT** the right way to do things
        since the modified .ini language that GIMI interprets is a **CFG** (context free grammer) and **NOT** a regular language.
   
        But since we are lazy and don't want make our own compiler with tokenizers, parsing algorithms (eg. SLR(1)), type checking, etc...
        this module should handle regular cases of .ini files generated using existing scripts (assuming the user does not do anything funny...)

    :raw-html:`<br />`

    Parameters
    ----------
    file: Optional[:class:`str`]
        The file path to the .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    logger: Optional[:class:`Logger`]
        The logger to print messages if necessary

    txt: :class:`str`
        Used as the text content of the .ini file if :attr:`IniFile.file` is set to ``None`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    modTypes: Optional[Set[:class:`ModType`]]
        The types of mods that the .ini file should belong to :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultModType: Optional[:class:`ModType`]
        The type of mod to use if the .ini file has an unidentified mod type :raw-html:`<br />` :raw-html:`<br />`
        If this value is ``None``, then will skip the .ini file with an unidentified mod type :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    file: :class:`str`
        The file path to the .ini file

    _parser: `ConfigParser`_
        Parser used to parse very basic cases in a .ini file

    modTypes: Optional[Set[:class:`ModType`]]
        The types of mods that the .ini file should belong to

    defaultModType: Optional[:class:`ModType`]
        The type of mod to use if the .ini file has an unidentified mod type

    _textureOverrideBlendRoot: Optional[:class:`str`]
        The name for the `section`_ containing the keywords: ``[.*TextureOverride.*Blend.*]``

    _sectionIfTemplates: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ in the .ini file that can be parsed into an :class:`IfTemplate`

        For more info see :class:`IfTemplate`

        .. warning::
            The modified .ini language that GIMI uses introduces keywords that can be used before the key of a key-value pair :raw-html:`<br />`

            *eg. defining constants*

            .. code-block:: ini
                :linenos:

                [Constants]
                global persist $swapvar = 0
                global persist $swapscarf = 0
                global $active
                global $creditinfo = 0

            :raw-html:`<br />`

            `Sections`_ containing this type of pattern will not be parsed. But generally, these sections are irrelevant to fixing the Raiden Boss

    _resourceBlends: Dict[:class:`str`, :class:`IfTemplate`]
        `Sections`_ that are linked to 1 or more Blend.buf files.

        The keys are the name of the sections.

    _blendCommands: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ that use some ``[Resource.*Blend.*]`` section. :raw-html:`<br />` :raw-html:`<br />`

        The keys are the name of the sections.

    _blendCommandsRemapNames: Dict[:class`str`, :class:`str`]
        The new names for the `sections`_ that use some ``[Resource.*Blend.*]`` section that will be used in the fix. :raw-html:`<br />` :raw-html:`<br />`

        The keys are the original names of the `sections`_ in the .ini file

    _blendCommandsTuples: List[Tuple[:class:`str`, :class:`IfTemplate`]]
        All the `sections`_ that use some ``[Resource.*Blend.*]`` section while maitaining the order that sections have been called

        .. note::
            This attribute is the same as :attr:`IniFile._blendCommands` except that the order that `sections`_ are called in the call stack is preserved

    _resourceCommands: Dict[:class:`str`, :class:`IfTemplate`]
        All the related `sections`_ to the ``[Resource.*Blend.*]`` sections that are used by `sections`_ related to the ``[TextureOverride.*Blend.*]`` sections.
        The keys are the name of the `sections`_.

    _resourceCommandsRemapNames: Dict[:class:`str`, :class:`str`]
        The new names to be used in the fix for all the related `sections`_ to the ``[Resource.*Blend.*]`` `sections`_ that are used by `sections`_ related to ``[TextureOverride.*Blend.*]`` `sections`_.

        The keys are the original names of the `sections`_ in the .ini file

    _resouceCommandsTuples: List[Tuple[:class:`str`, :class:`IfTemplate`]]
       All the related `sections`_ to the ``[Resource.*Blend.*]`` `sections`_ that are used by `sections`_ related to ``[TextureOverride.*Blend.*]`` `sections`_ 
       while maitaining the order that the `sections`_ have been called

        .. note::
            This attribute is the same as :attr:`IniFile._resourceCommands` except that the order that `sections`_ are called in the call stack is preserved

    remapBlendModelsDict: Dict[:class:`str`, :class:`RemapBlendModel`]
        The data for the ``[Resource.*RemapBlend.*]`` `sections`_ used in the fix

        The keys are the original names of the resource with the pattern ``[Resource.*Blend.*]``

    remapBlendModels: List[:class:`RemapBlendModel`]
        The data for the ``[Resource.*RemapBlend.*]`` `sections`_ used in the fix

        .. note::
            This attribute is the same as the values of :attr:`IniFile.remapBlendModelsDict` by calling: 
            
            .. code-block:: python
                :linenos:

                list(remapBlendModelsDict.values())
    """

    ModTypeNameReplaceStr = "{{modTypeName}}"
    ModTypeBossNameReplaceStr = "{{modTypeBossName}}"
    Credit = f'\n; {ModTypeBossNameReplaceStr}fixed by NK#1321 if you used it for fix your {ModTypeNameReplaceStr}mods pls give credit for "Nhok0169"\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support'

    _defaultHeading = Heading(".*Boss Fix", 15, "-")

    Hash = "hash"
    Vb1 = "vb1"
    Handling = "handling"
    Draw = "draw"
    Resource = "Resource"
    Blend = "Blend"
    Run = "run"
    RemapBlend = f"Remap{Blend}"

    # -- regex strings ---

    _textureOverrideBlendPatternStr = r"^\s*\[\s*TextureOverride.*" + Blend + r".*\s*\]"
    _fixedTextureOverrideBlendPatternStr = r"^\s*\[\s*TextureOverride.*" + RemapBlend + r".*\s*\]"

    # --------------------
    # -- regex objects ---
    _sectionPattern = re.compile(r"^\s*\[.*\]")
    _textureOverrideBlendPattern = re.compile(_textureOverrideBlendPatternStr)
    _fixedTextureOverrideBlendPattern = re.compile(_fixedTextureOverrideBlendPatternStr)
    _fixRemovalPattern = re.compile(f"; {_defaultHeading.open()}(.|\n)*; {_defaultHeading.close()[:-2]}(-)*")
    _removalPattern = re.compile(f"^\s*\[.*" + RemapBlend + r".*\]")

    # -------------------

    _ifStructurePattern = re.compile(r"\s*(endif|if|else)")

    def __init__(self, file: Optional[str] = None, logger: Optional[Logger] = None, txt: str = "", modTypes: Optional[Set[ModType]] = None, defaultModType: Optional[ModType] = None):
        super().__init__(logger = logger)
        self.file = file
        self._parser = configparser.ConfigParser(dict_type = ConfigParserDict, strict = False)

        self._fileLines = []
        self._fileTxt = ""
        self._fileLinesRead = False
        self._setupFileLines(fileTxt = txt)

        self._isFixed = False
        self._type = None
        self._isModIni = False

        if (modTypes is None):
            modTypes = set()
        self.defaultModType = defaultModType
        self.modTypes = modTypes
        self._heading = self._defaultHeading.copy()
        self._heading.title = None

        self._textureOverrideBlendRoot: Optional[str] = None
        self._textureOverrideBlendSectionName: Optional[str] = None
        self._sectionIfTemplates: Dict[str, IfTemplate] = {}
        self._resourceBlends: Dict[str, IfTemplate] = {}

        self._blendCommands: Dict[str, IfTemplate] = {}
        self._blendCommandsRemapNames: Dict[str, str] = {}
        self._blendCommandsTuples: List[Tuple[str, IfTemplate]] = []

        self._resourceCommands: Dict[str, IfTemplate] = {}
        self._resourceCommandsRemapNames:Dict[str, str] = {}
        self._resourceCommandsTuples: List[Tuple[str, IfTemplate]] = []

        self.remapBlendModelsDict: Dict[str, RemapBlendModel] = {}
        self.remapBlendModels: List[RemapBlendModel] = []

    @property
    def isFixed(self) -> bool:
        """
        Whether the .ini file has already been fixed

        :getter: Returns whether the .ini file has already been fixed
        :type: :class:`bool`
        """

        return self._isFixed
    
    @property
    def type(self) -> Optional[ModType]:
        """
        The type of mod the .ini file belongs to

        :getter: Returns the type of mod the .ini file belongs to
        :type: Optional[:class:`ModType`]
        """

        return self._type
    
    @property
    def isModIni(self) -> bool:
        """
        Whether the .ini file belongs to a mod

        :getter: Returns whether the .ini file belongs to a mod
        :type: :class:`bool`
        """

        return self._isModIni
    
    @property
    def fileLinesRead(self) -> bool:
        """
        Whether the .ini file has been read

        :getter: Determines whether the .ini file has been read
        :type: :class:`bool`
        """

        return self._fileLinesRead
    
    @property
    def fileTxt(self) -> str:
        """
        The text content of the .ini file

        :getter: Returns the content of the .ini file
        :setter: Reads the new value for both the text content of the .ini file and the text lines of the .ini file 
        :type: :class:`str`
        """

        return self._fileTxt
    
    @fileTxt.setter
    def fileTxt(self, newFileTxt: str):
        self._fileTxt = newFileTxt

        self._fileLines = self._fileTxt.split("\n")

        if (self._fileTxt):
            fileLinesLen = len(self._fileLines)
            for i in range(fileLinesLen):
                if (i < fileLinesLen - 1):
                    self._fileLines[i] += "\n"
        else:
            self._fileLines = []

        self._fileLinesRead = True
        self._isFixed = False
        self._textureOverrideBlendRoot = None
        self._textureOverrideBlendSectionName = None

    @property
    def fileLines(self) -> List[str]:
        """
        The text lines of the .ini file :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            For the setter, each line must end with a newline character (same behaviour as `readLines`_)

        :getter: Returns the text lines of the .ini file
        :setter: Reads the new value for both the text lines of the .ini file and the text content of the .ini file
        :type: List[:class:`str`]
        """

        return self._fileLines
    
    @fileLines.setter
    def fileLines(self, newFileLines: List[str]):
        self._fileLines = newFileLines
        self._fileTxt = "".join(self._fileLines)

        self._fileLinesRead = True
        self._isFixed = False
        self._textureOverrideBlendRoot = None
        self._textureOverrideBlendSectionName = None

    def clearRead(self, eraseSourceTxt: bool = False):
        """
        Clears the saved text read in from the .ini file

        .. note::
            If :attr:`IniFile.file` is set to ``None``, then the default run of this function
            with the argument ``eraseSourceTxt`` set to ``False`` will have no effect since the provided text from :attr:`IniFile._fileTxt` is the only source of data for the :class:`IniFile`

            If you also want to clear the above source text data, then run this function with the ``eraseSourceTxt`` argument set to ``True``

        Parameters
        ----------
        eraseSourceTxt: :class:`bool`
            Whether to erase the only data source for this class if :attr:`IniFile.file` is set to ``None``, see the note above for more info :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (self.file is not None or eraseSourceTxt):
            self._fileLines = []
            self._fileTxt = ""
            self._fileLinesRead = False

            self._isFixed = False
            self._textureOverrideBlendRoot = None
            self._textureOverrideBlendSectionName = None

    def clear(self, eraseSourceTxt: bool = False):
        """
        Clears all the saved data for the .ini file

        .. note::
            Please see the note at :meth:`IniFile.clearRead`

        Parameters
        ----------
        eraseSourceTxt: :class:`bool`
            Whether to erase the only data source for this class if :attr:`IniFile.file` is set to ``None``, see the note at :meth:`IniFile.clearRead` for more info :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        self.clearRead(eraseSourceTxt = eraseSourceTxt)
        self._type = None
        self._isModIni = False
        self._heading = self._defaultHeading.copy()
        self._heading.title = None

        self._sectionIfTemplates = {}
        self._resourceBlends = {}

        self._blendCommands = {}
        self._blendCommandsRemapNames = {}
        self._blendCommandsTuples = []

        self._resourceCommands = {}
        self._resourceCommandsRemapNames = {}
        self._resourceCommandsTuples = []

        self.remapBlendModelsDict = {}
        self.remapBlendModels = []

    def read(self) -> str:
        """
        Reads the .ini file :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will read the existing value from :attr:`IniFile.fileTxt`

        Returns
        -------
        :class:`str`
            The text content of the .ini file
        """

        if (self.file is not None):
            self.fileTxt = FileService.read(self.file, "r", lambda filePtr: filePtr.read())
        return self._fileTxt
    
    def write(self) -> str:
        """
        Writes back into the .ini files based off the content in :attr:`IniFile._fileLines`

        Returns
        -------
        :class:`str`
            The text that is written to the .ini file
        """

        if (self.file is None):
            return self._fileTxt

        with open(self.file, "w", encoding = IniFileEncoding) as f:
            f.write(self._fileTxt)

        return self._fileTxt

    def _setupFileLines(self, fileTxt: str = ""):
        if (self.file is None):
            self.fileTxt = fileTxt
            self._fileLinesRead = True

    def readFileLines(self) -> List[str]:
        """
        Reads each line in the .ini file :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will read the existing value from :attr:`IniFile.fileLines`

        Returns
        -------
        List[:class:`str`]
            All the lines read from the .ini file
        """

        if (self.file is not None):
            self.fileLines = FileService.read(self.file, "r", lambda filePtr: filePtr.readlines())
        return self._fileLines

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
                for line in self._fileLines:
                    print(f"LINE: {line}")
        """

        def readLinesWrapper(self, *args, **kwargs):
            if (not self._fileLinesRead):
                self.readFileLines()
            return func(self, *args, **kwargs)
        return readLinesWrapper
    
    def checkIsMod(self) -> bool:
        """
        Reads the entire .ini file and checks whether the .ini file belongs to a mod

        .. note::
            If the .ini file has already been parsed (eg. calling :meth:`IniFile.checkModType` or :meth:`IniFile.parse`), then

            you only need to read :meth:`IniFile.isModIni`

        Returns
        -------
        :class:`bool`
            Whether the .ini file is a .ini file that belongs to some mod
        """
        
        self.clearRead()
        section = lambda line: False
        self.getSectionOptions(section, postProcessor = lambda startInd, endInd, fileLines, sectionName, srcTxt: "")
        return self._isModIni
    
    def _checkModType(self, line: str):
        """
        Checks if a line of text contains the keywords to identify whether the .ini file belongs to the types of mods in :attr:`IniFile.modTypes` :raw-html:`<br />` :raw-html:`<br />`

        * If :attr:`IniFile.modTypes` is not empty, then will find the first :class:`ModType` that where the line makes :meth:`ModType.isType` return ``True``
        * Otherwise, will see if the line matches with the regex, ``[.*TextureOverride.*Blend.*]`` 

        Parameters
        ----------
        line: :class:`str`
            The text to check
        """

        if (not self._isModIni and self.defaultModType is not None and self._textureOverrideBlendSectionName is None and 
            self._textureOverrideBlendPattern.search(line)):
            self._isModIni = True
            self._textureOverrideBlendSectionName = self._getSectionName(line)

        if (self._textureOverrideBlendRoot is not None):
            return
        
        if (not self.modTypes and self._textureOverrideBlendPattern.search(line)):
            self._textureOverrideBlendRoot = self._getSectionName(line)
            self._isModIni = True
            return

        for modType in self.modTypes:
            if (modType.isType(line)):
                self._textureOverrideBlendRoot = self._getSectionName(line)
                self._type = modType
                self._heading.title = None
                self._isModIni = True
                break

    def _checkFixed(self, line: str):
        """
        Checks if a line of text matches the regex, ``[.*TextureOverride.*RemapBlend.*]`` ,to identify whether the .ini file has been fixed

        Parameters
        ----------
        line: :class:`str`
            The line of text to check
        """

        if (not self._isFixed and self._fixedTextureOverrideBlendPattern.search(line)):
            self._isFixed = True

    def _parseSection(self, sectionName: str, srcTxt: str, save: Optional[Dict[str, Any]] = None) -> Optional[Dict[str, str]]:
        """
        Regularly parses the key-value pairs of a certain `section`_

        The function parses uses `ConfigParser`_ to parse the `section`_.

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_

        srcTxt: :class:`str`
            The text containing the entire `section`_

        save: Optional[Dict[:class:`str`, Any]]
            Place to save the parsed result for the `section`_  :raw-html:`<br />` :raw-html:`<br />`

            The result for the parsed `section`_ will be saved as a value in the dictionary while section's name will be used in the key for the dictionary :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Optional[Dict[:class:`str`, :class:`str`]]
            The result from parsing the `section`_

            .. note:: 
                If `ConfigParser`_ is unable to parse the section, then ``None`` is returned
        """

        result = None
        try:
            self._parser.read_string(srcTxt)
            result = dict(self._parser[sectionName])
        except Exception:
            return result

        try:
            save[sectionName] = result
        except TypeError:
            pass

        return result
    
    def _getSectionName(self, line: str) -> str:
        currentSectionName = line
        rightPos = currentSectionName.rfind("]")
        leftPos = currentSectionName.find("[")

        if (rightPos > -1 and leftPos > -1):
            currentSectionName = currentSectionName[leftPos + 1:rightPos]
        elif (rightPos > -1):
            currentSectionName = currentSectionName[:rightPos]
        elif (leftPos > -1):
            currentSectionName = currentSectionName[leftPos + 1:]

        return currentSectionName.strip()

    # retrieves the key-value pairs of a section in the .ini file. Manually parsed the file since ConfigParser
    #   errors out on conditional statements in .ini file for mods. Could later inherit from the parser (RawConfigParser) 
    #   to custom deal with conditionals
    @_readLines
    def getSectionOptions(self, section: Union[str, Pattern, Callable[[str], bool]], postProcessor: Optional[Callable[[int, int, List[str], str, str], Any]] = None, 
                          handleDuplicateFunc: Optional[Callable[[List[Any]], Any]] = None) -> Dict[str, Any]:
        """
        Reads the entire .ini file for a certain type of `section`_

        Parameters
        ----------
        section: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
            The type of section to find

            * If this argument is a :class:`str`, then will check if the line in the .ini file exactly matches the argument
            * If this argument is a `Pattern`_, then will check if the line in the .ini file matches the specified Regex pattern
            * If this argument is a function, then will check if the line in the .ini file passed as an argument for the function will make the function return ``True``

        postProcessor: Optional[Callable[[:class:`int`, :class:`int`, List[:class:`str`], :class:`str`, :class:`str`], Any]]
            Post processor used when a type of `section`_ has been found

            The order of arguments passed into the post processor will be:

            #. The starting line index of the `section`_ in the .ini file
            #. The ending line index of the `section`_ in the .ini file
            #. All the file lines read from the .ini file
            #. The name of the `section`_ found
            #. The entire text for the `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: `None`

        handleDuplicateFunc: Optional[Callable[List[Any], Any]]
            Function to used to handle the case of multiple sections names :raw-html:`<br />` :raw-html:`<br />`

            If this value is set to ``None``, will keep all sections with the same names

            .. note::
                For this case, GIMI only keeps the first instance of all sections with same names

            :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[:class:`str`, Any]
            The resultant `sections`_ found

            The keys are the names of the `sections`_ found and the values are the content for the `section`_,
        """

        sectionFilter = None
        if (isinstance(section, str)):
            sectionFilter = lambda line: line == section
        elif callable(section):
            sectionFilter = section
        else:
            sectionFilter = lambda line: section.search(line)

        if (postProcessor is None):
            postProcessor = lambda startInd, endInd, fileLines, sectionName, srcTxt: self._parseSection(sectionName, srcTxt)

        result = {}
        currentSectionName = None
        currentSectionToParse = None
        currentSectionStartInd = -1

        fileLinesLen = len(self._fileLines)

        for i in range(fileLinesLen):
            line = self._fileLines[i]
            self._checkFixed(line)
            self._checkModType(line)

            # process the resultant section
            if (currentSectionToParse is not None and self._sectionPattern.search(line)):
                currentResult = postProcessor(currentSectionStartInd, i, self._fileLines, currentSectionName, currentSectionToParse)
                if (currentResult is None):
                    continue

                # whether to keep sections with the same name
                try:
                    result[currentSectionName]
                except KeyError:
                    result[currentSectionName] = [currentResult]
                else:
                    result[currentSectionName].append(currentResult)

                currentSectionToParse = None
                currentSectionName = None
                currentSectionStartInd = -1

            elif (currentSectionToParse is not None):
                currentSectionToParse += f"{line}"

            # keep track of the found section
            if (sectionFilter(line)):
                currentSectionToParse = f"{line}"
                currentSectionName = self._getSectionName(currentSectionToParse)
                currentSectionStartInd = i

        # get any remainder section
        if (currentSectionToParse is not None):
            currentResult = postProcessor(currentSectionStartInd, fileLinesLen, self._fileLines, currentSectionName, currentSectionToParse)
            try:
                result[currentSectionName]
            except:
                result[currentSectionName] = [currentResult]
            else:
                result[currentSectionName].append(currentResult)

        if (handleDuplicateFunc is None):
            return result

        # handle the duplicate sections with the same names
        for sectionName in result:
            result[sectionName] = handleDuplicateFunc(result[sectionName])

        return result

    def _removeSection(self, startInd: int, endInd: int, fileLines: List[str], sectionName: str, srcTxt: str) -> Tuple[int, int]:
        """
        Retrieves the starting line index and ending line index of where to remove a certain `section`_ from the read lines of the .ini file

        Parameters
        ----------
        startInd: :class:`int`
            The starting line index of the `section`_

        endInd: :class:`int`
            The ending line index of the `section`_

        fileLines: List[:class:`str`]
            All the file lines read from the .ini file

        sectionName: :class:`str`
            The name of the `section`_

        srcTxt: :class:`str`
            The text content of the `section`_

        Returns
        -------
        Tuple[:class:`int`, :class:`int`]
            The starting line index and the ending line index of the `section`_ to remove
        """

        fileLinesLen = len(fileLines)
        if (endInd > fileLinesLen):
            endInd = fileLinesLen

        if (startInd > fileLinesLen):
            startInd = fileLinesLen

        return (startInd, endInd)
    
    def removeSectionOptions(self, section: Union[str, Pattern, Callable[[str], bool]]):
        """
        Removes a certain type of `section`_ from the .ini file

        Parameters
        ----------
        section: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
            The type of `section`_ to remove
        """

        rangesToRemove = self.getSectionOptions(section, postProcessor = self._removeSection)

        for sectionName in rangesToRemove:
            ranges = rangesToRemove[sectionName]
            for range in ranges:
                startInd = range[0]
                endInd = range[1]

                self._fileLines[startInd:endInd] =  [0] * (endInd - startInd)

        self.fileLines = list(filter(lambda line: line != 0, self._fileLines))

    def _processIfTemplate(self, startInd: int, endInd: int, fileLines: List[str], sectionName: str, srcTxt: str) -> IfTemplate:
        """
        Parses a `section`_ in the .ini file as an :class:`IfTemplate`

        .. note::
            See :class:`IfTemplate` to see how we define an 'IfTemplate'

        Parameters
        ----------
        startInd: :class:`int`
            The starting line index of the `section`_

        endInd: :class:`int`
            The ending line index of the `section`_

        fileLines: List[:class:`str`]
            All the file lines read from the .ini file

        sectionName: :class:`str`
            The name of the `section`_

        srcTxt: :class:`str`
            The text content of the `section`_

        Returns
        -------
        :class:`IfTemplate`
            The generated :class:`IfTemplate` from the `section`_
        """

        ifTemplate = []
        dummySectionName = "dummySection"
        currentDummySectionName = f"{dummySectionName}"
        replaceSection = ""
        atReplaceSection = False

        for i in range(startInd + 1, endInd):
            line = fileLines[i]
            isConditional = bool(self._ifStructurePattern.match(line))

            if (isConditional and atReplaceSection):
                currentDummySectionName = f"{dummySectionName}{i}"
                replaceSection = f"[{currentDummySectionName}]\n{replaceSection}"

                currentPart = self._parseSection(currentDummySectionName, replaceSection)
                if (currentPart is None):
                    currentPart = {}

                ifTemplate.append(currentPart)
                replaceSection = ""

            if (isConditional):
                ifTemplate.append(line)
                atReplaceSection = False
                continue
            
            replaceSection += line
            atReplaceSection = True

        # get any remainder replacements in the if..else template
        if (replaceSection != ""):
            currentDummySectionName = f"{dummySectionName}END{endInd}"
            replaceSection = f"[{currentDummySectionName}]\n{replaceSection}"
            currentPart = self._parseSection(currentDummySectionName, replaceSection)
            if (currentPart is None):
                currentPart = {}

            ifTemplate.append(currentPart)

        # get all the sections called by the current section
        result = IfTemplate(ifTemplate)
        calledSubCommands = result.find(pred = lambda part: isinstance(part, dict) and self._isIfTemplateSubCommand(part), postProcessor = self._getIfTemplateSubCommand)
        result.calledSubCommands = calledSubCommands

        return result
                
    
    @classmethod
    def getMergedResourceIndex(cls, mergedResourceName: str) -> Optional[int]:
        """
        Retrieves the index number of a resource created by GIMI's ``genshin_merge_mods.py`` script

        Examples
        --------
        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend.8")
        8


        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend.Score.-100")
        -100


        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend.UnitTests")
        None


        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend")
        None

        Parameters
        ----------
        mergedResourceName: :class:`str`
            The name of the `section`_

        Returns
        -------
        Optional[:class:`int`]
            The index for the resource `section`_, if found and the index is an integer
        """
        result = None

        try:
            result = int(mergedResourceName.rsplit(".", 1)[-1])
        except:
            pass
            
        return result
    
    def _compareResources(self, resourceTuple1: Tuple[str, Optional[int]], resourceTuple2: Tuple[str, Optional[int]]) -> int:
        """
        Compare function used for sorting resources :raw-html:`<br />` :raw-html:`<br />`

        The order for sorting is the resources is:
        
        #. Resources that do are not suffixed by an index number
        #. Resource that are suffixed by an index number (see :meth:`IniFile.getMergedResourceIndex` for more info)

        Parameters
        ----------
        resourceTuple1: Tuple[:class:`str`, Optional[:class:`int`]]
            Data for the first resource in the compare function, contains:

            * Name of the resource
            * The index for the resource

        resourceTuple2: Tuple[:class:`str`, Optional[:class:`int`]]
            Data for the second resource in the compare function, contains:

            * Name of the resource
            * The index for the resource

        Returns
        -------
        :class:`int`
            The result for a typical compare function used in sorting

            * returns -1 if ``resourceTuple1`` should come before ``resourceTuple2``
            * returns 1 if ``resourceTuple1`` should come after ``resourceTuple2``
            * returns 0 if ``resourceTuple1`` is equal to ``resourceTuple2`` 
        """

        resourceKey1 = resourceTuple1[1]
        resourceKey2 = resourceTuple2[1]
        resource1MissingIndex = resourceKey1 is None
        resource2MissingIndex = resourceKey2 is None

        if (resource1MissingIndex):
            resourceKey1 = resourceTuple1[0]
        
        if (resource2MissingIndex):
            resourceKey2 = resourceTuple2[0]

        if ((resource1MissingIndex == resource2MissingIndex and resourceKey1 < resourceKey2) or (resource1MissingIndex and not resource2MissingIndex)):
            return -1
        elif ((resource1MissingIndex == resource2MissingIndex and resourceKey1 > resourceKey2) or (not resource1MissingIndex and resource2MissingIndex)):
            return 1
        
        return 0

    # Disabling the OLD ini
    def disIni(self):
        """
        Disables the .ini file

        .. note::
            For more info, see :meth:`FileService.disableFile`
        """

        if (self.file is not None):
            FileService.disableFile(self.file)

    @classmethod
    def getFixedBlendFile(cls, blendFile: str) -> str:
        """
        Retrieves the file path for the fixed RemapBlend.buf file

        Parameters
        ----------
        blendFile: :class:`str`
            The file path to the original Blend.buf file

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapBlend.buf file
        """

        blendFolder = os.path.dirname(blendFile)
        blendBaseName = os.path.basename(blendFile)
        blendBaseName = blendBaseName.rsplit(".", 1)[0]
        
        return os.path.join(blendFolder, f"{cls.getRemapName(blendBaseName)}.buf")
    
    def getFixModTypeName(self) -> Optional[str]:
        """
        Retrieves the name of the type of mod corresponding to the .ini file to be used for the comment of the fix

        Returns
        -------
        Optional[:class:`str`]
            The name for the type of mod corresponding to the .ini file
        """
        if (self._type is None):
            return None
        return self._type.name.replace("\n", "").replace("\t", "")
    
    def getFixModTypeHeadingname(self):
        """
        Retrieves the name of the type of mod corresponding to the .ini file to be used in the header/footer divider comment of the fix

        Returns
        -------
        Optional[:class:`str`]
            The name for the type of mod to be displayed in the header/footer divider comment
        """

        modTypeName = self.getFixModTypeName()
        if (modTypeName is None):
            modTypeName = "GI"

        if (modTypeName is not None and modTypeName):
            modTypeName += " "

        return modTypeName

    def getFixHeader(self) -> str:
        """
        Retrieves the header text used to identify a code section has been changed by this fix
        in the .ini file

        Returns
        -------
        :class:`str`
            The header section comment to be used in the .ini file
        """
        
        if (self._heading.title is None):
            modTypeName = self.getFixModTypeHeadingname()
            self._heading.title = f"{modTypeName}Boss Fix"
        return f"; {self._heading.open()}"
    
    def getFixFooter(self) -> str:
        """
        Retrieves the footer text used to identify a code section has been changed by this fix
        in the .ini file

        Returns
        -------
        :class:`str`
            The footer section comment to be used in the .ini file
        """

        if (self._heading.title is None):
            modTypeName = self.getFixModTypeHeadingname()
            self._heading.title = f"{modTypeName}Boss Fix"
        return f"\n\n; {self._heading.close()}"
    
    def getFixCredit(self) -> str:
        """
        Retrieves the credit text for the code generated in the .ini file

        Returns
        -------
        :class:`str`
            The credits to be displayed in the .ini file
        """

        modTypeName = self.getFixModTypeName()
        if (modTypeName is None):
            modTypeName = ""

        if (modTypeName):
            modTypeName += " "

        bossModTypeName = f"{modTypeName}Boss "
        return self.Credit.replace(self.ModTypeBossNameReplaceStr, bossModTypeName).replace(self.ModTypeNameReplaceStr, modTypeName)

    def _addFixBoilerPlate(func):
        """
        Decorator used to add the boilerplate code to identify a code section has been changed by this fix in the .ini file

        Examples
        --------
        .. code-block:: python
            :linenos:

            @_addFixBoilerPlate
            def helloWorld(self) -> str:
                return "Hello World"
        """

        def addFixBoilerPlateWrapper(self, *args, **kwargs):
            addFix = self.getFixHeader()
            addFix += self.getFixCredit()
            addFix += func(self, *args, **kwargs)
            addFix += self.getFixFooter()

            return addFix
        return addFixBoilerPlateWrapper
    
    @classmethod
    def getResourceName(cls, name: str) -> str:
        """
        Makes the name of a `section`_ to be used for the resource `sections`_ of a .ini file

        Examples
        --------
        >>> IniFile.getResourceName("CuteLittleEi")
        "ResourceCuteLittleEi"


        >>> IniFile.getResourceName("ResourceCuteLittleEi")
        "ResourceCuteLittleEi"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ as a resource in a .ini file
        """

        if (not name.startswith(cls.Resource)):
            name = f"{cls.Resource}{name}"
        return name
    
    @classmethod
    def removeResourceName(cls, name: str) -> str:
        """
        Removes the 'Resource' prefix from a section's name

        Examples
        --------
        >>> IniFile.removeResourceName("ResourceCuteLittleEi")
        "CuteLittleEi"


        >>> IniFile.removeResourceName("LittleMissGanyu")
        "LittleMissGanyu"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the 'Resource' prefix removed
        """

        if (name.startswith(cls.Resource)):
            name = name[len(cls.Resource):]

        return name
    
    @classmethod
    def getRemapName(cls, name: str) -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapBlend' to identify that the `section`_
        is created by this fix


        Examples
        --------
        >>> IniFile.getRemapName("EiTriesToUseBlenderAndFails")
        "EiTriesToUseRemapBlenderAndFails"


        >>> IniFile.getRemapName("EiBlendsTheBlender")
        "EiBlendsTheRemapBlender"
    

        >>> IniFile.getRemapName("ResourceCuteLittleEi")
        "ResourceCuteLittleEiRemapBlend"


        >>> IniFile.getRemapName("ResourceCuteLittleEiRemapRemapBlend")
        "ResourceCuteLittleEiRemapRemapBlend"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapBlend' keyword
        """

        nameParts = name.rsplit(cls.Blend, 1)
        namePartsLen = len(nameParts)

        if (namePartsLen > 1):
            name = cls.RemapBlend.join(nameParts)
        else:
            name += cls.RemapBlend

        return name

    @classmethod
    def getRemapResourceName(cls, name: str) -> str:
        """
        Changes the name of a section to be a new resource that this fix will create

        .. note::
            See :meth:`IniFile.getResourceName` and :meth:`IniFile.getRemapName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the keyword 'Remap' added
        """

        name = cls.getRemapName(name)
        name = cls.getResourceName(name)
        return name

    def _isIfTemplateResource(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'vb1'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        :class:`bool`
            Whether 'vb1' is contained in the part
        """

        return self.Vb1 in ifTemplatePart
    
    def _isIfTemplateDraw(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'draw'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        :class:`bool`
            Whether 'draw' is contained in the part
        """


        return self.Draw in ifTemplatePart
    
    def _isIfTemplateSubCommand(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'run'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a section

        Returns
        -------
        :class:`bool`
            Whether 'run' is contained in the part
        """
                
        return self.Run in ifTemplatePart
    
    def _getIfTemplateResourceName(self, ifTemplatePart: Dict[str, Any]) -> Any:
        """
        Retrieves the value from the key, 'vb1', for some part of a `section`_

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        Any
            The corresponding value for the key 'vb1'
        """

        return ifTemplatePart[self.Vb1]
    
    def _getIfTemplateSubCommand(self, ifTemplatePart: Dict[str, Any]) -> Any:
        """
        Retrieves the value from the key, 'run', for some part of a `section`_

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        Any
            The corresponding value for the key 'run'
        """

        return ifTemplatePart[self.Run]
    
    # fills the attributes for the sections related to the texture override blend
    def _fillTextureOverrideRemapBlend(self, sectionName: str, part: Dict[str, Any], partIndex: int, linePrefix: str, origSectionName: str) -> str:
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix related to the ``[TextureOverride.*Blend.*]`` `sections`_

        .. note::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        sectionName: :class:`str`
            The new name for the section

        part: Dict[:class:`str`, Any]
            The content part of the :class:`IfTemplate` of the original [TextureOverrideBlend] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName in part:
            varValue = part[varName]

            # filling in the subcommand
            if (varName == self.Run):
                subCommandStr = f"{self.Run} = {self._blendCommandsRemapNames[varValue]}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the hash
            elif (varName == self.Hash):
                hash = ""
                if (self._type is not None):
                    hash = self._type.bossHash
                elif (self.defaultModType is not None):
                    hash = self.defaultModType.bossHash
                else:
                    raise NoModType()

                addFix += f"{linePrefix}hash = {hash}\n"

            # filling in the vb1 resource
            elif (varName == self.Vb1):
                blendName = self._getIfTemplateResourceName(part)
                remapModel = self.remapBlendModelsDict[blendName]
                fixStr = f'{self.Vb1} = {remapModel.fixedBlendName}'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the handling
            elif (varName == self.Handling):
                fixStr = f'{self.Handling} = skip'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the draw value
            elif (varName == self.Draw):
                fixStr = f'{self.Draw} = {varValue}'
                addFix += f"{linePrefix}{fixStr}\n"

        return addFix
    
    # fill the attributes for the sections related to the resources
    def _fillRemapResource(self, sectionName: str, part: Dict[str, Any], partIndex: int, linePrefix: str, origSectionName: str):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new `sections`_ created by this fix related to the ``[Resource.*Blend.*]`` `sections`_

        .. note::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        sectionName: :class:`str`
            The new name for the `section`_

        part: Dict[:class:`str`, Any]
            The content part of the :class:`IfTemplate` of the original ``[Resource.*Blend.*]`` `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName in part:
            varValue = part[varName]

            # filling in the subcommand
            if (varName == self.Run):
                subCommandStr = f"{self.Run} = {self._resourceCommandsRemapNames[varValue]}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # add in the type of file
            elif (varName == "type"):
                addFix += f"{linePrefix}type = Buffer\n"

            # add in the stride for the file
            elif (varName == "stride"):
                addFix += f"{linePrefix}stride = 32\n"

            # add in the file
            elif (varName == "filename"):
                remapModel = self.remapBlendModelsDict[origSectionName]
                fixedBlendFile = remapModel.fixedBlendPaths[partIndex]
                addFix += f"{linePrefix}filename = {fixedBlendFile}\n"

        return addFix
    
    # fills the if..else template in the .ini for each section
    def fillIfTemplate(self, sectionName: str, ifTemplate: IfTemplate, fillFunc: Callable[[str, Union[str, Dict[str, Any]], int, int, str], str], origSectionName: Optional[str] = None) -> str:
        """
        Creates a new :class:`IfTemplate` for an existing `section`_ in the .ini file

        Parameters
        ----------
        sectionName: :class:`str`
            The new name of the `section`_

        ifTemplate: :class:`IfTemplate`
            The :class:`IfTemplate` of the orginal `section`_

        fillFunc: Callable[[:class:`str`, Union[:class:`str`, Dict[:class:`str`, Any], :class:`int`, :class:`str`, :class:`str`], :class:`str`]]
            The function to create a new **content part** for the new :class:`IfTemplate`
            :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                For more info about an 'IfTemplate', see :class:`IfTemplate`

            :raw-html:`<br />`
            The parameter order for the function is:

            #. The new section name
            #. The corresponding **content part** in the original :class:`IfTemplate`
            #. The index for the content part in the original :class:`IfTemplate`
            #. The string to prefix every line in the **content part** of the :class:`IfTemplate`
            #. The original name of the section

        origSectionName: Optional[:class:`str`]
            The original name of the section.

            If this argument is set to ``None``, then will assume this argument has the same value as the argument for ``sectionName`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The text for the newly created :class:`IfTemplate`
        """

        addFix = f"[{sectionName}]\n"
        partIndex = 0
        linePrefix = ""

        if (origSectionName is None):
            origSectionName = sectionName

        for part in ifTemplate:
            # adding in the if..else statements
            if (isinstance(part, str)):
                addFix += part
                
                linePrefix = re.match(r"^[( |\t)]*", part)
                if (linePrefix):
                    linePrefix = linePrefix.group(0)
                    linePrefixLen = len(linePrefix)

                    linePrefix = part[:linePrefixLen]
                    lStrippedPart = part[linePrefixLen:]

                    if (lStrippedPart.find("endif") == -1):
                        linePrefix += "\t"
                partIndex += 1
                continue
            
            # add in the content within the if..else statements
            addFix += fillFunc(sectionName, part, partIndex, linePrefix, origSectionName)

            partIndex += 1
            
        return addFix

    # get the needed lines to fix the .ini file
    @_addFixBoilerPlate
    def getFixStr(self, fix: str = "") -> str:
        """
        Generates the newly added code in the .ini file for the fix

        Parameters
        ----------
        fix: :class:`str`
            Any existing text we want the result of the fix to add onto :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ""

        Returns
        -------
        :class:`str`
            The text for the newly generated code in the .ini file
        """
        hasResources = bool(self.remapBlendModels)
        if (self._blendCommands or hasResources):
            fix += "\n\n"

        # get the fix string for all the texture override blends
        for commandTuple in self._blendCommandsTuples:
            section = commandTuple[0]
            ifTemplate = commandTuple[1]
            commandName = self.getRemapName(section)
            fix += self.fillIfTemplate(commandName, ifTemplate, self._fillTextureOverrideRemapBlend)
            fix += "\n"

        if (hasResources):
            fix += "\n"

        # get the fix string for the resources
        resourceCommandsLen = len(self._resourceCommandsTuples)
        for i in range(resourceCommandsLen):
            commandTuple = self._resourceCommandsTuples[i]
            section = commandTuple[0]
            ifTemplate = commandTuple[1]

            resourceName = self.getRemapName(section)
            fix += self.fillIfTemplate(resourceName, ifTemplate, self._fillRemapResource, origSectionName = section)

            if (i < resourceCommandsLen - 1):
                fix += "\n"

        return fix

    @_readLines
    def injectAddition(self, addition: str, beforeOriginal: bool = True, keepBackup: bool = True, fixOnly: bool = False) -> str:
        """
        Adds and writes new text to the .ini file

        Parameters
        ----------
        addition: :class:`str`
            The text we want to add to the file

        beforeOriginal: :class:`bool`
            Whether to add the new text before the original text :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        keepBackup: :class:`bool`
            Whether we want to make a backup copy of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether we are only fixing the .ini file without removing any previous changes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The content of the .ini file with the new text added
        """

        original = "".join(self._fileLines)

        if (keepBackup and fixOnly and self.file is not None):
            self.print("log", "Cleaning up and disabling the OLD STINKY ini")
            self.disIni()

        result = ""
        if (beforeOriginal):
            result = f"{addition}\n\n{original}"
        else:
            result = f"{original}\n{addition}"

        # writing the fixed file
        if (self.file is not None):
            with open(self.file, "w", encoding = IniFileEncoding) as f:
                f.write(result)

        self._isFixed = True
        return result

    @_readLines
    def _removeScriptFix(self) -> str:
        """
        Removes the dedicated section of the code in the .ini file that this script has made  

        Returns
        -------
        :class:`str`
            The new text content of the .ini file
        """

        self._fileTxt = re.sub(self._fixRemovalPattern, "", self._fileTxt)
        self.fileTxt = self._fileTxt.strip()

        result = self.write()
        self.clearRead()
        self._isFixed = False
        return result

    def _removeFix(self) -> str:
        """
        Removes any previous changes that were probably made by this script :raw-html:`<br />` :raw-html:`<br />`

        For the .ini file will remove:

        #. All code surrounded by the *'---...--- .* Boss Fix ---...----'* header/footer
        #. All `sections`_ containing the keywords ``RemapBlend``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file with the changes removed
        """

        self._removeScriptFix()        
        if (not self._fileLinesRead):
            self.readFileLines()

        self.removeSectionOptions(self._removalPattern)
        result = self.write()

        self.clearRead()
        self._isFixed = False
        return result

    @_readLines
    def removeFix(self, keepBackups: bool = True, fixOnly: bool = False) -> str:
        """
        Removes any previous changes that were probably made by this script and creates backup copies of the .ini file

        .. note::
            For more info about what gets removed from the .ini file, see :meth:`IniFile._removeFix`

        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether we want to make a backup copy of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether we are only fixing the .ini file without removing any previous changes :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If this value is set to ``True``, then the previous changes made by this script will not be removed

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file with the changes removed
        """
        
        if (keepBackups and not fixOnly and self.file is not None):
            self.print("log", f"Creating Backup for {os.path.basename(self.file)}")
            self.disIni()

        if (fixOnly):
            return self._fileTxt

        if (self.file is not None):
            self.print("log", f"Removing any previous changes from this script in {os.path.basename(self.file)}")

        result = self._removeFix()
        return result


    def _makeRemapModels(self) -> Dict[str, RemapBlendModel]:
        """
        Low level function to create all the data needed for fixing the ``[Resource.*Blend.*]`` `sections`_ in the .ini file

        Returns
        -------
        Dict[:class:`str`, :class:`RemapBlendModel`]
            The data for fixing the resource `sections`_

            The keys are the names for the resource `sections`_ and the values are the required data for fixing the `sections`_
        """

        folderPath = CurrentDir
        if (self.file is not None):
            folderPath = os.path.dirname(self.file)
        
        for resourceKey in self._resourceCommands:
            resourceIftemplate = self._resourceCommands[resourceKey]
            fixedBlendName = self.getRemapResourceName(resourceKey)
            origBlendPaths = {}
            fixedBlendPaths = {}

            partIndex = 0
            for part in resourceIftemplate:
                if (isinstance(part,str)):
                    partIndex += 1
                    continue

                origBlendFile = None
                try:
                    origBlendFile = FileService.parseOSPath(part['filename'])
                except KeyError:
                    partIndex += 1
                    continue

                fixedBlendPath = self.getFixedBlendFile(origBlendFile)
                origBlendPaths[partIndex] = origBlendFile
                fixedBlendPaths[partIndex] = fixedBlendPath

                partIndex += 1

            remapBlendModel = RemapBlendModel(folderPath, fixedBlendName, fixedBlendPaths, origBlendName = resourceKey, origBlendPaths = origBlendPaths)
            self.remapBlendModelsDict[resourceKey] = remapBlendModel

        self.remapBlendModels = list(self.remapBlendModelsDict.values())
        return self.remapBlendModelsDict

    def _getSubCommands(self, ifTemplate: IfTemplate, currentSubCommands: Set[str], subCommands: Set[str], subCommandLst: List[str]):
        for partIndex in ifTemplate.calledSubCommands:
            subCommand = ifTemplate.calledSubCommands[partIndex]
            if (subCommand not in subCommands):
                currentSubCommands.add(subCommand)
                subCommands.add(subCommand)
                subCommandLst.append(subCommand)

    def _getCommandIfTemplate(self, sectionName: str, raiseException: bool = True) -> Optional[IfTemplate]:
        """
        Low level function for retrieving the :class:`IfTemplate` for a certain `section`_ from `IniFile._sectionIfTemplate`

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_

        raiseException: :class:`bool`
            Whether to raise an exception when the section's :class:`IfTemplate` is not found

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` for the `section`_ is not found and ``raiseException`` is set to `True`

        Returns
        -------
        Optional[:class:`IfTemplate`]
            The corresponding :class:`IfTemplate` for the `section`_
        """
        try:
            ifTemplate = self._sectionIfTemplates[sectionName]
        except Exception as e:
            if (raiseException):
                raise KeyError(f"The section by the name '{sectionName}' does not exist") from e
            else:
                return None
        else:
            return ifTemplate

    def _getBlendResources(self, sectionName: str, blendResources: Set[str], subCommands: Set[str], subCommandLst: List[str]):
        """
        Low level function for retrieving all the referenced resources that were called by `sections`_ related to the ``[TextureOverride.*Blend.*]`` `sections`_

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_ in the .ini file that we want to get the blend resources from

        blendResources: Set[:class:`str`]
            The result for all the resource `sections`_ that were referenced

        subCommands: Set[:class:`str`]
            The result for all of the sub-sections that were called from the ``[TextureOverride.*Blend.*]`` `section`_

        subCommandLst: List[:class:`str`]
            The result for all of the sub-sections that were called from the ``[TextureOverride.*Blend.*]`` `section`_ that maintains the order
            the `sections`_ are called in the call stack

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` is not found for some `section`_ related to the ``[TextureOverride.*Blend.*]`` `section`_
        """

        ifTemplate = self._getCommandIfTemplate(sectionName)
        currentSubCommands = set()

        for part in ifTemplate:
            if (isinstance(part, str)):
                continue

            if (self._isIfTemplateResource(part)):
                resource = self._getIfTemplateResourceName(part)
                blendResources.add(resource)
        
        # get all the unvisited subcommand sections to visit
        self._getSubCommands(ifTemplate, currentSubCommands, subCommands, subCommandLst)

        # get the blend resources from other subcommands
        for sectionName in currentSubCommands:
            self._getBlendResources(sectionName, blendResources, subCommands, subCommandLst)

    def _getCommands(self, sectionName: str, subCommands: Set[str], subCommandLst: List[str]):
        """
        Low level function for retrieving all the commands/`sections`_ that are called from a certain `section`_ in the .ini file

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_ we are starting from

        subCommands: Set[:class:`str`]
            The result for all of the `sections`_ that were called

        subCommandLst: List[:class:`str`]
            The result for all of the `sections`_ that were called while maintaining the order
            the `sections`_ are called in the call stack

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` is not found for some `section`_
        """

        currentSubCommands = set()
        ifTemplate = self._getCommandIfTemplate(sectionName)

        # add in the current command if it has not been added yet
        if (sectionName not in subCommands):
            subCommands.add(sectionName)
            subCommandLst.append(sectionName)

        # get all the unvisited subcommand sections to visit
        self._getSubCommands(ifTemplate, currentSubCommands, subCommands, subCommandLst)

        # visit the children subcommands that have not been visited yet
        for sectionName in currentSubCommands:
            self._getCommands(sectionName, subCommands, subCommandLst)

    # parse(): Parses the merged.ini file for any info needing to keep track of
    def parse(self):
        """
        Parses the .ini file

        Raises
        ------
        :class:`KeyError`
            If a certain resource `section`_ is not found :raw-html:`<br />` :raw-html:`<br />`
            
            (either the name of the `section`_ is not found in the .ini file or the `section`_ was skipped due to some error when parsing the `section`_)
        """
        self._blendCommands = {}
        self._blendCommandsRemapNames = {}
        self._resourceCommands = {}
        self._resourceCommandsRemapNames = {}
        self._blendCommandsTuples = []
        self._resourceCommandsTuples = []

        self._sectionIfTemplates = self.getSectionOptions(self._sectionPattern, postProcessor = self._processIfTemplate, handleDuplicateFunc = lambda duplicates: duplicates[0])

        if (self.defaultModType is not None and self._textureOverrideBlendSectionName is not None and self._textureOverrideBlendRoot is None):
            self._textureOverrideBlendRoot = self._textureOverrideBlendSectionName

        try:
            self._sectionIfTemplates[self._textureOverrideBlendRoot]
        except:
            return

        blendResources = set()
        subCommands = { self._textureOverrideBlendRoot }
        subCommandLst = [self._textureOverrideBlendRoot]
        resourceCommands = set()
        resourceCommandLst = []

        # keep track of all the needed blend dependencies
        self._getBlendResources(self._textureOverrideBlendRoot, blendResources, subCommands, subCommandLst)

        # read in all the needed dependencies
        for blend in blendResources:
            try:
                self._resourceBlends[blend] = self._sectionIfTemplates[blend]
            except Exception as e:
                raise KeyError(f"The resource by the name, '{blend}', does not exist") from e
            else:
                resourceCommands.add(blend)
                resourceCommandLst.append(blend)

        # sort the resources
        resourceCommandLst = list(map(lambda resourceName: (resourceName, self.getMergedResourceIndex(resourceName)), resourceCommandLst))
        resourceCommandLst.sort(key = cmp_to_key(self._compareResources))
        resourceCommandLst = list(map(lambda resourceTuple: resourceTuple[0], resourceCommandLst))

        # keep track of all the subcommands that the resources call
        for blend in blendResources:
            self._getCommands(blend, resourceCommands, resourceCommandLst)

        for subCommand in subCommands:
            self._blendCommands[subCommand] = self._sectionIfTemplates[subCommand]
            self._blendCommandsRemapNames[subCommand] = self.getRemapName(subCommand)

        for subCommand in resourceCommands:
            self._resourceCommands[subCommand] = self._sectionIfTemplates[subCommand]
            self._resourceCommandsRemapNames[subCommand] = self.getRemapName(subCommand)

        self._blendCommandsTuples = list(map(lambda subCommand: (subCommand, self._blendCommands[subCommand]), subCommandLst))
        self._resourceCommandsTuples = list(map(lambda subCommand: (subCommand, self._resourceCommands[subCommand]), resourceCommandLst))   

        self._makeRemapModels()


    def fix(self, keepBackup: bool = True, fixOnly: bool = False) -> str:
        """
        Fixes the .ini file

        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether we want to make a backup copy of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: `True`

        fixOnly: :class:`bool`
            Whether we are only fixing the .ini file without removing any previous changes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: `False`

        Returns
        -------
        :class:`str`
            The new content of the .ini file which includes the fix
        """

        fix = ""
        fix += self.getFixStr(fix = fix)
        result = self.injectAddition(f"\n\n{fix}", beforeOriginal = False, keepBackup = keepBackup, fixOnly = fixOnly)
        self._isFixed = True
        return result


class Mod(Model):
    """
    This Class inherits from :class:`Model`

    Used for handling a mod

    .. note::
        We define **a mod** based off the following criteria:

        * A folder that contains at least 1 .ini file
        * At least 1 of the .ini files in the folder contains:

            * a section with the regex ``[TextureOverride.*Blend]`` if :attr:`BossFixService.readAllInis` is set to ``True`` or the script is ran with the ``--all`` flag :raw-html:`<br />`  :raw-html:`<br />` **OR** :raw-html:`<br />` :raw-html:`<br />`
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

    defaultType: Optional[:class:`ModType`]
        The type of mod to use if a mod has an unidentified type :raw-html:`<br />` :raw-html:`<br />`
        If this argument is ``None``, then will skip the mod with an identified type :raw-html:`<br />` :raw-html:`<br />` 

        **Default**: ``None``

    Attributes
    ----------
    path: Optional[:class:`str`]
        The file location to the mod folder

    _files: List[:class:`str`]
        The direct children files to the mod folder (does not include files located in a folder within the mod folder).

    _types: Set[:class:`ModType`]
        The types of mods this mod should be

    _defaultType: Optional[:class:`ModType`]
        The type of mod to use if a mod has an unidentified type

    logger: Optional[:class:`Logger`]
        The logger used to pretty print messages

    inis: List[:class:`str`]
        The .ini files found for the mod

    remapBlend: List[:class:`str`]
        The RemapBlend.buf files found for the mod

    backupInis: List[:class:`str`]
        The DISABLED_BossFixBackup.txt files found for the mod

    backupDups: List[:class:`str`]
        The DISABLED_RSDup.txt files found for the mod

        .. warning::
            This attribute is now DEPRECATED. Now, the fix does not care whether there are duplicate .ini files or Blend.buf files
    """
    def __init__(self, path: Optional[str] = None, files: Optional[List[str]] = None, logger: Optional[Logger] = None, types: Optional[Set[ModType]] = None, defaultType: Optional[ModType] = None):
        super().__init__(logger = logger)
        self.path = FileService.getPath(path)
        self._files = files
        if (types is None):
            types = set()
        self._types = types
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

        self.inis, self.remapBlend, self.backupInis = self.getOptionalFiles()
        self.inis = list(map(lambda iniPath: IniFile(iniPath, logger = self.logger, modTypes = self._types, defaultModType = self._defaultType), self.inis))

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

        return file.endswith(IniExt)
    
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
        if (not baseName.endswith(BufExt)):
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

        return bool(file.endswith(BlendFileType) and not cls.isRemapBlend(file))
   
    @classmethod
    def isBackupIni(cls, file: str) -> bool:
        """
        Determines whether the file is a DISABLED_BossFixBackup.txt file that is used to make
        backup copies of .ini files

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a DISABLED_BossFixBackup.txt file
        """

        fileBaseName = os.path.basename(file)
        return fileBaseName.startswith(BackupFilePrefix) and file.endswith(TxtExt)

    def getOptionalFiles(self) -> List[Optional[str]]:
        """
        Retrieves a list of each type of files that are not mandatory for the mod

        Returns
        -------
        [ List[:class:`str`], List[:class:`str`], List[:class:`str`]]
            The resultant files found for the following file categories (listed in the same order as the return type):

            #. .ini files
            #. .RemapBlend.buf files
            #. DISABLED_BossFixBackup.txt files

            .. note::
                See :meth:`Mod.isIni`, :meth:`Mod.isRemapBlend`, :meth:`Mod.isBackupIni` for the specifics of each type of file
        """

        SingleFileFilters = {}
        MultiFileFilters = [self.isIni, self.isRemapBlend, self.isBackupIni]

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
        Removes all DISABLED_BossFixBackup.txt contained in the mod
        """

        for file in self.backupInis:
            self.print("log", f"Removing the backup ini, {os.path.basename(file)}")
            os.remove(file)

    def removeFix(self, fixedBlends: Set[str], fixedInis: Set[str], visitedRemapBlendsAtRemoval: Set[str], inisSkipped: Dict[str, Exception], keepBackups: bool = True, fixOnly: bool = False) -> List[Set[str]]:
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
            Whether to create or keep DISABLED_BossFixBackup.txt files in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to not undo any changes created in the .ini files :raw-html:`<br />` :raw-html:`<br />`

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

            # remove only the remap blends that have not been recently created
            for blendModel in ini.remapBlendModels:
                for partIndex in blendModel.fullPaths:
                    remapBlendFullPath = blendModel.fullPaths[partIndex]

                    if (remapBlendFullPath not in fixedBlends and remapBlendFullPath not in visitedRemapBlendsAtRemoval):
                        try:
                            os.remove(remapBlendFullPath)
                        except FileNotFoundError as e:
                            self.print("log", f"No Previous {RemapBlendFile} found at {remapBlendFullPath}")
                        else:
                            self.print("log", f"Removing previous {RemapBlendFile} at {remapBlendFullPath}")
                            removedRemapBlends.add(remapBlendFullPath)

                        visitedRemapBlendsAtRemoval.add(remapBlendFullPath)
                        if (not remapBlendsRemoved):
                            remapBlendsRemoved = True

            if (remapBlendsRemoved):
                self.print("space")

            if (iniHasErrors):
                continue

            # remove the fix from the .ini files
            if (iniFullPath is not None and iniFullPath not in fixedInis and iniFullPath not in inisSkipped and 
                ini.isModIni and (not self._types or ini.type in self._types)):
                try:
                    ini.removeFix(keepBackups = keepBackups, fixOnly = fixOnly)
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

        return [undoedInis, removedRemapBlends]

    # correcting the blend file
    @classmethod
    def blendCorrection(self, blendFile: Union[str, bytes], modType: ModType, fixedBlendFile: Optional[str] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Blend.buf file

        .. note::
            We observe that a Blend.buf file is a binary file defined as:

            * each line contains 32 bytes (256 bits)
            * each line uses little-endian mode (MSB is to the right while LSB is to the left)
            * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
            * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
            * the blend weights are floating points while the blend indices are unsigned integers

        Parameters
        ----------
        blendFile: Union[:class:`str`, :class:`bytes`]
            The file path to the Blend.buf file to fix

        fixedBlendFile: Optional[:class:`str`]
            The file path for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`

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

        # if no correction is needed to be done
        blendIsFile = isinstance(blendFile, str)
        if (not modType.vgRemap and blendIsFile):
            return None
        elif (not modType.vgRemap):
            return bytearray(blendFile)

        blendData = None
        if (blendIsFile):
            with open(blendFile, "rb") as f:
                blendData = f.read()
        else:
            blendData = blendFile

        if (len(blendData)%32 != 0):
            if (blendIsFile):
                raise BlendFileNotRecognized(blendFile)
            else:
                raise BadBlendData()

        result = bytearray()
        for i in range(0,len(blendData),32):
            blendweights = [struct.unpack("<f", blendData[i+4*j:i+4*(j+1)])[0] for j in range(4)]
            blendindices = [struct.unpack("<I", blendData[i+16+4*j:i+16+4*(j+1)])[0] for j in range(4)]
            outputweights = bytearray()
            outputindices = bytearray()

            # replaces the blend index in the original mod with the corresponding blend index
            #   for the boss
            for weight, index in zip(blendweights, blendindices):
                if weight != 0 and index <= modType.maxVgIndex:
                    index = int(modType.vgRemap[index])
                outputweights += struct.pack("<f", weight)
                outputindices += struct.pack("<I", index)
            result += outputweights
            result += outputindices

        if (fixedBlendFile is not None):
            with open(fixedBlendFile, "wb") as f:
                f.write(result)

            return fixedBlendFile

        return result
    
    def correctBlend(self, fixedRemapBlends: Dict[str, RemapBlendModel], skippedBlends: Dict[str, Exception]) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the Blend.buf files reference by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        fixedRemapBlends: Dict[:class:`str`, :class:`RemapBlendModel`]
            All of the RemapBlend.buf files that have already been fixed. :raw-html:`<br />` :raw-html:`<br />`

            The keys are the absolute filepath to the fixed RemapBlend.buf file and the values contains the data related
            to the fixed RemapBlend.buf file

        skippedBlends: Dict[:class:`str`, :class:`Exception`]
            All of the RemapBlend.buf files that have already been skipped due to some error when trying to fix them :raw-html:`<br />` :raw-html:`<br />`

            The keys are the absolute filepath to the RemapBlend.buf file that was attempted to be fixed and the values are the exception encountered

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

            for model in ini.remapBlendModels:
                modType = self._defaultType
                if (ini.type is not None):
                    modType = ini.type

                for partIndex in model.fullPaths:
                    blendFixed = True
                    fixedFullPath = model.fullPaths[partIndex]

                    try:
                        origFullPath = model.origFullPaths[partIndex]
                    except KeyError:
                        self.print("log", f"Missing Original Blend file for the RemapBlend file at {fixedFullPath}")
                        if (fixedFullPath not in skippedBlends):
                            error = RemapMissingBlendFile(fixedFullPath)
                            currentBlendsSkipped[fixedFullPath] = error
                            skippedBlends[fixedFullPath] = error
                        continue

                    # check if the blend file has been fixed
                    try:
                        fixedRemapBlends[fixedFullPath]
                    except:
                        blendFixed = False
                    else:
                        self.print("log", f"Blend file has already been corrected at {fixedFullPath}")

                    # check if the blend was already encountered and did not need to be fixed
                    try:
                        fixedRemapBlends[origFullPath]
                    except KeyError:
                        pass
                    else:
                        blendFixed = True

                    # check if the blend file already had encountered an error
                    fixedPathWasSkipped = bool(fixedFullPath in skippedBlends)
                    if (fixedPathWasSkipped or origFullPath in skippedBlends):
                        targetFullPath = fixedFullPath
                        if (not fixedPathWasSkipped):
                            targetFullPath = origFullPath

                        self.print("log", f"Blend file has already previously encountered an error at {targetFullPath}")
                        continue

                    if (blendFixed or modType is None):
                        continue
                    
                    correctedBlendPath = None
                    try:
                        correctedBlendPath = self.blendCorrection(origFullPath, modType, fixedBlendFile = fixedFullPath)
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
                        fixedRemapBlends[pathToAdd] = model

        return [currentBlendsFixed, currentBlendsSkipped]


class BossFixService():
    """
    The overall class for fixing bosses for particular mods

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

    readAllInis: :class:`bool`
        Whether to read all the .ini files that the fix encounters :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    types: Optional[:class:`str`]
        A string containing the names for all the types of mods to fix. Each type of mod is seperated using a comma (,)  :raw-html:`<br />` :raw-html:`<br />`

        If this argument is the empty string or this argument is ``None``, then will fix all the types of mods supported by this fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultType: Optional[:class:`str`]
        The name for the type to use if a mod has an unidentified type :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then mods with unidentified types will be skipped :raw-html:`<br />` :raw-html:`<br />`

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

    Attributes
    ----------
    log: Optional[:class:`str`]
        The folder location to log the run of the fix into a seperate text file

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

    readAllInis: :class:`bool`
        Whether to read all the .ini files that the fix encounters

    types: Set[:class:`ModType`]
        All the types of mods that will be fixed.

    defaultType: Optional[:class:`ModType`]
        The type to use if a mod has an unidentified type

    verbose: :class:`bool`
        Whether to print the progress for fixing mods

    handleExceptions: :class:`bool`
        When an exception is caught, whether to silently stop running the fix

    _logFile: :class:`str`
        The file path of where to generate a log .txt file

    _pathIsCWD: :class:`bool`
        Whether the filepath that the program runs from is the current directory where this module is loaded

    modsFixed: :class:`int`
        The number of mods that have been fixed

    skippedMods: Dict[:class:`str`, :class:`Exception`]
        All the mods that have been skipped :raw-html:`<br />` :raw-html:`<br />`

        The keys are the absolute path to the mod folder and the values are the exception that caused the mod to be skipped

    blendsFixed: Set[:class:`str`]
        The absolute paths to all the Blend.buf files that have been fixed

    skippedBlendsByMods: DefaultDict[:class:`str`, Dict[:class:`str`, :class:`Exception`]]
        The RemapBlend.buf files that got skipped :raw-html for each mod :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the absolute path to the mod folder
        * The inner key is the absolute path to the RemapBlend.buf file
        * The value in the inner dictionary is the exception that caused the RemapBlend.buf file to be skipped

    skippedBlends: Dict[:class:`str`, :class:`Exception`]
        The RemapBlend.buf files that got skipped  :raw-html:`<br />` :raw-html:`<br />`

        The keys are the absolute path to the RemapBlend.buf file and the values are the exception that caused the RemapBlend.buf file to be skipped

    inisFixed: Set[:class:`str`]
        The absolute paths to the fixed .ini files

    inisSkipped: Dict[:class:`str`, :class:`Exception`]
        The .ini files that got skipped :raw-html:`<br />` :raw-html:`<br />`

        The keys are the absolute file paths to the .ini files and the values are exceptions that caused the .ini file to be skipped

    removedRemapBlends: Set[:class:`str`]
        Previous RemapBlend.buf files that are removed
    """

    def __init__(self, path: Optional[str] = None, keepBackups: bool = True, fixOnly: bool = False, undoOnly: bool = False, 
                 readAllInis: bool = False, types: Optional[str] = None, defaultType: Optional[str] = None, log: Optional[str] = None, verbose: bool = True, handleExceptions: bool = False):
        self.log = log
        self._loggerBasePrefix = ""
        self.logger = Logger(logTxt = log, verbose = verbose)
        self._path = path
        self.keepBackups = keepBackups
        self.fixOnly = fixOnly
        self.undoOnly = undoOnly
        self.readAllInis = readAllInis
        self.types = types
        self.defaultType = defaultType
        self.verbose = verbose
        self.handleExceptions = handleExceptions
        self._pathIsCwd = False
        self.__errorsBeforeFix = None

        # certain statistics about the fix
        self.modsFixed = 0
        self.skippedMods: Dict[str, Exception] = {}
        self.blendsFixed: Set[str] = set()
        self.skippedBlendsByMods: DefaultDict[str, Dict[str, Exception]] = defaultdict(lambda: {})
        self.skippedBlends: Dict[str, Exception] = {}
        self.inisFixed = set()
        self.inisSkipped: Dict[str, Exception] = {}
        self.removedRemapBlends: Set[str] = set()
        self._visitedRemapBlendsAtRemoval: Set[str] = set()

        self._setupModPath()
        self._setupModTypes()
        self._setupDefaultModType()
        self._setupLogPath()

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

    def clear(self):
        """
        Clears up all the saved data
        """

        self.modsFixed = 0
        self.skippedMods = {}
        self.blendsFixed = set()
        self.skippedBlendsByMods = defaultdict(lambda: {})
        self.skippedBlends = {}
        self.inisFixed = set()
        self.inisSkipped = {}
        self.removedRemapBlends = set()
        self._visitedRemapBlendsAtRemoval = set()
    
    def _setupModPath(self):
        """
        Sets the filepath of where the fix will run from
        """

        self._pathIsCwd = False
        if (self._path is None):
            self._path = DefaultPath
            self._pathIsCwd = True
            return

        self._path = FileService.parseOSPath(self._path)
        self._path = FileService.parseOSPath(os.path.abspath(self._path))
        self._pathIsCwd = (self._path == DefaultPath)

    def _setupLogPath(self):
        """
        Sets the folder path for where the log file will be stored
        """

        if (self.log is not None):
            self.log = FileService.parseOSPath(os.path.join(self.log, LogFile))

    def _setupModTypes(self):
        """
        Sets the types of mods that will be fixed
        """

        if (isinstance(self.types, set)):
            return

        modTypes = set()
        if (self.types is None or self.readAllInis):
            modTypes = ModTypes.getAll()

        # search for the types of mods to fix
        else:
            typesLst = self.types.split(",")

            for typeStr in typesLst:
                modType = ModTypes.search(typeStr)
                modTypeFound = bool(modType is not None)

                if (modTypeFound):
                    modTypes.add(modType)
                elif (self.__errorsBeforeFix is None):
                    self.__errorsBeforeFix = InvalidModType(typeStr)
                    return

        self.types = modTypes

    def _setupDefaultModType(self):
        """
        Sets the default mod type to be used for an unidentified mod
        """

        if (not self.readAllInis):
            self.defaultType = None
        elif (self.defaultType is None):
            self.defaultType = ModTypes.Raiden.value
            return

        if (self.defaultType is None or isinstance(self.defaultType, ModType)):
            return

        self.defaultType = ModTypes.search(self.defaultType)

        if (self.defaultType is None and self.__errorsBeforeFix is None):
            self.__errorsBeforeFix = InvalidModType(self.defaultType)

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
            for type in self.types:
                self.logger.bulletPoint(f"{type.name}")
        
        self.logger.space()
        self.logger.closeHeading()
        self.logger.split() 
        self.logger.includePrefix = True
    
    # fixes an ini file in a mod
    def fixIni(self, ini: IniFile, mod: Mod, fixedRemapBlends: Dict[str, RemapBlendModel]) -> bool:
        """
        Fixes an individual .ini file for a particular mod

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        ini: :class:`IniFile`
            The .ini file to fix

        mod: :class:`Mod`
            The mod being fixed

        fixedRemapBlends: Dict[:class:`str`, :class:`RemapBlendModel`]
            All of the RemapBlend.buf files that have already been fixed.
            :raw-html:`<br />`
            :raw-html:`<br />`
            The keys are the absolute filepath to the fixed RemapBlend.buf file and the values contains the data related
            to the fixed RemapBlend.buf file

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

        if (iniFullPath in self.inisSkipped):
            self.logger.log(f"the ini file, {fileBaseName}, has alreaedy encountered an error")
            return False
        
        if (iniFullPath in self.inisFixed):
            self.logger.log(f"the ini file, {fileBaseName}, is already fixed")
            return False

        # parse the .ini file
        self.logger.log(f"Parsing {fileBaseName}...")
        ini.parse()
        if (ini.isFixed):
            self.logger.log(f"the ini file, {fileBaseName}, is already fixed")
            return False

        # fix the blends
        self.logger.log(f"Fixing the {BlendFileType} files for {fileBaseName}...")
        currentBlendsFixed, currentBlendsSkipped = mod.correctBlend(fixedRemapBlends = fixedRemapBlends, skippedBlends = self.skippedBlends)
        self.blendsFixed = self.blendsFixed.union(currentBlendsFixed)

        if (currentBlendsSkipped):
            self.skippedBlendsByMods[mod.path] = DictTools.combine(self.skippedBlendsByMods[mod.path], currentBlendsSkipped)

        # writing the fixed file
        self.logger.log(f"Making the fixed ini file for {fileBaseName}")
        ini.fix(keepBackup = self.keepBackups, fixOnly = self.fixOnly)

        return True

    # fixes a mod
    def fixMod(self, mod: Mod, fixedRemapBlends: Dict[str, RemapBlendModel]) -> bool:
        """
        Fixes a particular mod

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        mod: :class:`Mod`
            The mod being fixed

        fixedRemapBlends: Dict[:class:`str`, :class:`RemapBlendModel`]
            all of the RemapBlend.buf files that have already been fixed.
            :raw-html:`<br />` :raw-html:`<br />`
            The keys are the absolute filepath to the fixed RemapBlend.buf files and the values contains the data related
            to the fixed RemapBlend.buf file

        Returns
        -------
        :class:`bool`
            Whether the particular mod has just been fixed
        """

        # remove any backups
        if (not self.keepBackups):
            mod.removeBackupInis()

        for ini in mod.inis:
            ini.checkIsMod()

        # undo any previous fixes
        if (not self.fixOnly):
            undoedInis, removedRemapBlends = mod.removeFix(self.blendsFixed, self.inisFixed, self._visitedRemapBlendsAtRemoval, self.inisSkipped, keepBackups = self.keepBackups, fixOnly = self.fixOnly)
            self.removedRemapBlends = self.removedRemapBlends.union(removedRemapBlends)

        result = False
        firstIniException = None
        inisLen = len(mod.inis)

        for i in range(inisLen):
            ini = mod.inis[i]
            iniFullPath = FileService.absPathOfRelPath(ini.file, mod.path)
            iniIsFixed = False

            try:
                iniIsFixed = self.fixIni(ini, mod, fixedRemapBlends)
            except Exception as e:
                self.logger.handleException(e)
                self.inisSkipped[iniFullPath] = e 

                if (firstIniException is None):
                    firstIniException = e

            if (firstIniException is None and iniFullPath in self.inisSkipped):
                firstIniException = self.inisSkipped[iniFullPath]

            result = (result or iniIsFixed)

            if (not iniIsFixed):
                continue
            
            if (i < inisLen - 1):
                self.logger.space()

            self.inisFixed.add(iniFullPath)

        if (not result and firstIniException is not None):
            self.skippedMods[mod.path] = firstIniException

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
                self.logger.bulletPoint(f'Hate deleting the "{BackupFilePrefix}" {IniExt}/{TxtExt} files yourself after running this script? (cuz I know I do!) Run this script again (on CMD) using the {DeleteBackupOpt} option')

            if (not self.undoOnly):
                self.logger.bulletPoint(f"Want to undo this script's fix? Run this script again (on CMD) using the {RevertOpt} option")

            if (not self.readAllInis):
                self.logger.bulletPoint(f"Were your {IniFileType}s not read? Run this script again (on CMD) using the {AllOpt} option")

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

    def warnSkippedBlends(self, modPath: str):
        """
        Prints out all of the Blend.buf files that were skipped due to exceptions

        Parameters
        ----------
        modPath: :class:`str`
            The absolute path to a particular folder
        """

        parentFolder = os.path.dirname(self._path)
        relModPath = FileService.getRelPath(modPath, parentFolder)
        modHeading = Heading(f"Mod: {relModPath}", 5)
        message = f"{modHeading.open()}\n\n"
        blendWarnings = self.skippedBlendsByMods[modPath]
        
        for blendPath in blendWarnings:
            relBlendPath = FileService.getRelPath(blendPath, self._path)
            message += self.logger.getBulletStr(f"{relBlendPath}:\n\t{Heading(type(blendWarnings[blendPath]).__name__, 3, '-').open()}\n\t{blendWarnings[blendPath]}\n\n")
        
        message += f"{modHeading.close()}\n"
        return message

    def reportSkippedMods(self):
        """
        Prints out all of the mods that were skipped due to exceptions

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`
        """

        self.reportSkippedAsset("mods", self.skippedMods, lambda dir: self.logger.getBulletStr(f"{dir}:\n\t{Heading(type(self.skippedMods[dir]).__name__, 3, '-').open()}\n\t{self.skippedMods[dir]}\n\n"))
        self.reportSkippedAsset(f"{IniFileType}s", self.inisSkipped, lambda file: self.logger.getBulletStr(f"{file}:\n\t{Heading(type(self.inisSkipped[file]).__name__, 3, '-').open()}\n\t{self.inisSkipped[file]}\n\n"))
        self.reportSkippedAsset(f"{BlendFileType} files", self.skippedBlendsByMods, lambda dir: self.warnSkippedBlends(dir))

    def reportSummary(self):
        skippedMods = len(self.skippedMods)
        foundMods = self.modsFixed + skippedMods
        fixedBlends = len(self.blendsFixed)
        skippedBlends = len(self.skippedBlends)
        foundBlends = fixedBlends + skippedBlends
        fixedInis = len(self.inisFixed)
        skippedInis = len(self.inisSkipped)
        foundInis = fixedInis + skippedInis
        removedRemapBlends = len(self.removedRemapBlends)

        self.logger.openHeading("Summary", sideLen = 10)
        self.logger.space()
        
        modFixMsg = ""
        blendFixMsg = ""
        iniFixMsg = ""
        removedRemappedMsg = ""
        if (not self.undoOnly):
            modFixMsg = f"Out of {foundMods} found mods, fixed {self.modsFixed} mods and skipped {skippedMods} mods"
            iniFixMsg = f"Out of the {foundInis} {IniFileType}s within the found mods, fixed {fixedInis} {IniFileType}s and skipped {skippedInis} {IniFileType} files"
            blendFixMsg = f"Out of the {foundBlends} {BlendFileType} files within the found mods, fixed {fixedBlends} {BlendFileType} files and skipped {skippedBlends} {BlendFileType} files"
        else:
            modFixMsg = f"Out of {foundMods} found mods, remove fix from {self.modsFixed} mods and skipped {skippedMods} mods"

        if (not self.fixOnly and removedRemapBlends > 0):
            removedRemappedMsg = f"Removed {removedRemapBlends} old {RemapBlendFile} files"


        self.logger.bulletPoint(modFixMsg)
        if (iniFixMsg):
            self.logger.bulletPoint(iniFixMsg)

        if (blendFixMsg):
            self.logger.bulletPoint(blendFixMsg)

        if (removedRemappedMsg):
            self.logger.bulletPoint(removedRemappedMsg)

        self.logger.space()
        self.logger.closeHeading()

    def createLog(self):
        """
        Creates a log text file that contains all the text printed on the command line
        """

        if (self.log is None):
            return

        self.logger.includePrefix = False
        self.logger.space()

        self.logger.log(f"Creating log file, {LogFile}")

        self.logger.includePrefix = True

        with open(self.log, "w", encoding = IniFileEncoding) as f:
            f.write(self.logger.loggedTxt)

    def createMod(self, path: Optional[str] = None, files: Optional[List[str]] = None) -> Mod:
        """
        Creates a mod

        .. note:: 
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
        mod = Mod(path = path, files = files, logger = self.logger, types = self.types, defaultType = self.defaultType)
        return mod

    def _fix(self):
        """
        The overall logic for fixing a bunch of mods

        For finding out which folders may contain mods, this function:
            #. recursively searches all folders from where the :attr:`BossFixService.path` is located
            #. for every .ini file in a valid mod and every Blend.buf file encountered that is encountered, recursively search all the folders from where the .ini file or Blend.buf file is located

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`
        """

        if (self.__errorsBeforeFix is not None):
            raise self.__errorsBeforeFix

        if (self.fixOnly and self.undoOnly):
            raise ConflictingOptions([FixOnlyOpt, RevertOpt])

        parentFolder = os.path.dirname(self._path)
        self._loggerBasePrefix = os.path.basename(self._path)
        self.logger.prefix = os.path.basename(DefaultPath)

        visitedDirs = set()
        visitingDirs = set()
        dirs = deque()
        dirs.append(self._path)
        visitingDirs.add(self._path)
        fixedRemapBlends = {}
    
        while (dirs):
            path = dirs.popleft()
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
            except:
                visitingDirs.remove(path)
                visitedDirs.add(path)
                continue

            # fix the mod
            try:
                fixedMod = self.fixMod(mod, fixedRemapBlends)
            except Exception as e:
                self.logger.handleException(e)
                if (mod.inis):
                    self.skippedMods[path] = e

            # get all the folders that could potentially be other mods
            modFiles, modDirs = FileService.getFilesAndDirs(path = path, recursive = True)

            if (mod.inis):
                for ini in mod.inis:
                    for blendModel in ini.remapBlendModels:
                        resourceModDirs = map(lambda partIndex: os.path.dirname(blendModel.origFullPaths[partIndex]), blendModel.origFullPaths) 
                        modDirs += resourceModDirs
            
            # add in all the folders that need to be visited
            for dir in modDirs:
                if (dir in visitedDirs):
                    continue

                if (dir not in visitingDirs):
                    dirs.append(dir)
                visitingDirs.add(dir)

            # increment the count of mods found
            if (fixedMod):
                self.modsFixed += 1

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
            noErrors = bool(not self.skippedMods and not self.skippedBlendsByMods)

            if (noErrors):
                self.logger.space()
                self.logger.log("ENJOY")

            self.logger.split()

            if (noErrors):
                self.addTips()

        self.createLog()


def main():
    args = argParser.parse_args()
    readAllInis = args.all
    defaultType = args.defaultType

    bossFixService = BossFixService(path = args.src, keepBackups = not args.deleteBackup, fixOnly = args.fixOnly, 
                                    undoOnly = args.revert, readAllInis = readAllInis, types = args.types, defaultType = defaultType,
                                    log = args.log, verbose = True, handleExceptions = True)
    bossFixService.fix()
    bossFixService.logger.waitExit()

# Main Driver Code
if __name__ == "__main__":
    main()