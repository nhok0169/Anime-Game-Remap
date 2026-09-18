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
import ntpath
from typing import Optional, List, Callable, Dict, Union, Any
##### EndExtImports

##### LocalImports
from ...constants.FilePrefixes import FilePrefixes
from ...constants.FilePathConsts import FilePathConsts
from ...constants.FileTypes import FileTypes
from ...constants.FileExt import FileExt
from ...constants.FileEncodings import ReadEncodings
from ...constants.GenericTypes import TextIoWrapper
from .FilePath import FilePath
from ...exceptions.MissingFileException import MissingFileException
from ...exceptions.DuplicateFileException import DuplicateFileException
##### EndLocalImports


##### Script
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
            filters[FileTypes.Default.value] = lambda itemPath: True
        
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
    def disableFile(cls, file: str, filePrefix: str = FilePrefixes.BackupFilePrefix.value) -> str:
        """
        Marks a file as disabled and changes the file to a .txt file

        Parameters
        ----------
        file: :class:`str`
            The file path to the file we are working with

        filePrefix: :class:`str`
            Prefix name we want to add in front of the file name :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``"RemapBKUP"``

        Returns
        -------
        :class:`str`
            The new name of the file
        """

        baseName = os.path.basename(file)
        baseName = FileService.changeExt(baseName, FileExt.Txt.value)

        backupFile = os.path.join(os.path.dirname(file), filePrefix + baseName)
        FileService.rename(file, backupFile)
        return backupFile

    @classmethod
    def copyFile(cls, src: str, dest: str):
        """
        Copies a file from ``src`` to ``dest``

        Parameters
        ----------
        src: :class:`str`
            The file path to the file to be copied

        dest: :class:`str`
            The new file path for the copied file
        """

        shutil.copy2(src, dest)

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
    def readBinary(cls, src: Union[str, bytes]) -> bytes:
        """
        Reads a binary file

        Parameters
        ----------
        src: Union[:class:`str`, :class:`bytes`]
            The source to read from

        Returns
        -------
        :class:`bytes`
            The read bytes
        """

        result = None
        if (isinstance(src, str)):
            with open(src, "rb") as f:
                result = f.read()
        else:
            result = src

        return result
    
    @classmethod
    def writeBinary(cls, file: str, data: bytes):
        """
        Writes data into a binary file

        Parameters
        ----------
        file: :class:`str`
            The file to write into

        data: :class:`bytes`
            The data to write
        """

        with open(file, "wb") as f:
            f.write(data)

    @classmethod
    def getPath(cls, path: Optional[str]) -> str:
        return FilePathConsts.getPath(path)
##### EndScript
    