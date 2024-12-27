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
import re
import os
import configparser
from typing import List, Dict, Optional, Set, Callable, Any, Union, Tuple, Type
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import Pattern
from ...constants.FilePathConsts import FilePathConsts
from ...constants.FileEncodings import FileEncodings
from ...constants.IfPredPartType import IfPredPartType
from ...constants.IniConsts import IniKeywords, IniBoilerPlate
from ...constants.FileExt import FileExt
from ...exceptions.NoModType import NoModType
from .File import File
from ..iftemplate.IfTemplate import IfTemplate
from ..iftemplate.IfContentPart import IfContentPart
from ..iftemplate.IfPredPart import IfPredPart
from ..IniSectionGraph import IniSectionGraph
from ..iniresources.IniResourceModel import IniResourceModel
from ..iniresources.IniTexModel import IniTexModel
from ..strategies.ModType import ModType, GlobalIniRemoveBuilder
from ..strategies.iniParsers.BaseIniParser import BaseIniParser
from ..strategies.iniFixers.BaseIniFixer import BaseIniFixer
from ..strategies.iniRemovers.BaseIniRemover import BaseIniRemover
from ..strategies.texEditors.BaseTexEditor import BaseTexEditor
from ..iniparserdicts.KeepFirstDict import KeepFirstDict
from ..iniparserdicts.KeepAllDict import KeepAllDict
from ...tools.files.FilePath import FilePath
from ...tools.TextTools import TextTools
from ...tools.files.FileService import FileService
from ...view.Logger import Logger
##### EndLocalImports


##### Script
# IniFile: Class to handle .ini files
class IniFile(File):
    """
    This class inherits from :class:`File`

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

    modsToFix: Optional[Set[:class:`str`]]
        The names of the mods we want to fix to :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultModType: Optional[:class:`ModType`]
        The type of mod to use if the .ini file has an unidentified mod type :raw-html:`<br />` :raw-html:`<br />`
        If this value is ``None``, then will skip the .ini file with an unidentified mod type :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    version: Optional[:class:`float`]
        The game version we want the .ini file to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    version: Optional[:class:`float`]
        The game version we want the .ini file to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version.

    _parser: `ConfigParser`_
        Parser used to parse very basic cases in a .ini file

    modTypes: Set[:class:`ModType`]
        The types of mods that the .ini file should belong to

    modsToFix: Set[:class:`str`]
        The names of the mods that we want to fix to

    defaultModType: Optional[:class:`ModType`]
        The type of mod to use if the .ini file has an unidentified mod type

    _textureOverrideBlendRoot: Optional[:class:`str`]
        The name for the `section`_ containing the keywords: ``[.*TextureOverride.*Blend.*]``

    sectionIfTemplates: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ in the .ini file that can be parsed into an :class:`IfTemplate`

        For more info see :class:`IfTemplate`

        .. attention::
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

    remapBlendModels: Dict[:class:`str`, :class:`IniResourceModel`]
        The data for the ``[Resource.*RemapBlend.*]`` `sections`_ used in the fix

        The keys are the original names of the resource with the pattern ``[Resource.*Blend.*]``

    texEditModels: Dict[:class:`str`, Dict[:class:`str`, :class:`IniTexModel`]]
        The data for the ``[Resource.*]`` `sections`_ that belong to some texture file that got editted :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names for the type of texture files *eg. MyBrandNewLightMap*
        * The inner keys are the original names of the resource with the pattern ``[Resource.*]``

    texAddModels: Dict[:class:`str`, Dict[:class:`str`, :class:`IniTexModel`]]
        The data for the ``[Resource.*]`` `sections`_ that belong to some texture file that got added :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names for the type of texture files *eg. MyBrandNewLightMap*
        * The inner keys are the names of the mod object *eg. Head*
    """

    # -- regex strings ---

    _textureOverrideBlendPatternStr = r"^\s*\[\s*TextureOverride.*" + IniKeywords.Blend.value + r".*\s*\]"
    _fixedTextureOverrideBlendPatternStr = r"^\s*\[\s*TextureOverride.*" + IniKeywords.RemapBlend.value + r".*\s*\]"
    
    # --------------------
    # -- regex objects ---
    _sectionPattern = re.compile(r"^\s*\[.*\]")
    _textureOverrideBlendPattern = re.compile(_textureOverrideBlendPatternStr)
    _fixedTextureOverrideBlendPattern = re.compile(_fixedTextureOverrideBlendPatternStr)

    # -------------------

    _ifStructurePattern = re.compile(r"\s*(" + IfPredPartType.EndIf.value + "|" + IfPredPartType.Else.value +  "|" + IfPredPartType.If.value + "|" + IfPredPartType.Elif.value + ")")

    def __init__(self, file: Optional[str] = None, logger: Optional["Logger"] = None, txt: str = "", modTypes: Optional[Set[ModType]] = None, defaultModType: Optional[ModType] = None, 
                 version: Optional[float] = None, modsToFix: Optional[Set[str]] = None):
        super().__init__(logger = logger)

        self._filePath: Optional[FilePath] = None
        self.file = file
        self.version = version

        self._parserDictType = KeepAllDict
        self._parser = configparser.ConfigParser(dict_type = self._parserDictType, strict = False)
        self._parser.optionxform=str

        self._fileLines = []
        self._fileTxt = ""
        self._fileLinesRead = False
        self._ifTemplatesRead = False
        self._setupFileLines(fileTxt = txt)

        if (modTypes is None):
            modTypes = set()
        if (modsToFix is None):
            modsToFix = set()

        self.defaultModType = defaultModType
        self.modTypes = modTypes
        self.modsToFix = modsToFix
        self._heading = IniBoilerPlate.DefaultHeading.value.copy()

        self._isFixed = False
        self._setType(None)
        self._isModIni = False

        self._textureOverrideBlendRoot: Optional[str] = None
        self._textureOverrideBlendSectionName: Optional[str] = None
        self.sectionIfTemplates: Dict[str, IfTemplate] = {}
        self._resourceBlends: Dict[str, IfTemplate] = {}

        self.remapBlendModels: Dict[str, IniResourceModel] = {}
        self.texEditModels: Dict[str, Dict[str, IniTexModel]] = {}
        self.texAddModels: Dict[str, Dict[str, IniTexModel]] = {}

        self._iniParser: Optional[BaseIniParser] = None
        self._iniFixer: Optional[BaseIniFixer] = None
        self._iniRemover: Optional[BaseIniRemover] = None

    @property
    def filePath(self) -> Optional[FilePath]:
        """
        The path to the .ini file

        :getter: Returns the path to the file
        :type: Optional[:class:`FilePath`]
        """
        return self._filePath

    @property
    def file(self) -> Optional[str]:
        """
        The file path to the .ini file

        :getter: Returns the path to the file
        :setter: Sets the new path for the file
        :type: Optional[:class:`str`]
        """

        if (self._filePath is None):
            return None
        return self._filePath.path
    
    @file.setter
    def file(self, newFile: Optional[str]) -> str:
        if (newFile is not None and self._filePath is None):
            self._filePath = FilePath(newFile)
        elif (newFile is not None):
            self._filePath.path = newFile
        elif (self._filePath is not None):
            self._filePath = None

    @property
    def folder(self) -> str:
        """
        The folder where this .ini file resides :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will return the folder where this script is ran

        :getter: Retrieves the folder
        :type: :class:`str`
        """

        if (self._filePath is not None):
            return self._filePath.folder
        return FilePathConsts.CurrentDir

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
    
    def _setType(self, newType: Optional[ModType]):
        self._type = newType
        self._heading.title = None
    
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
        self._fileLines = TextTools.getTextLines(self._fileTxt)

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

        if (self._filePath is not None or eraseSourceTxt):
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
        self._heading = IniBoilerPlate.DefaultHeading.value.copy()
        self._setType(None)
        self._isModIni = False

        self._ifTemplatesRead = False
        self.sectionIfTemplates = {}
        self._resourceBlends = {}

        self._iniParser = None
        self._iniFixer = None

        self.remapBlendModels.clear()
        self.texEditModels.clear()
        self.texAddModels.clear()


    @property
    def availableType(self) -> Optional[ModType]:
        """
        Retrieves the type of mod identified for this .ini file

        .. note::
            This function is the same as :meth:`IniFile.type`, but will return :attr:`IniFile.defaultModType` if :meth:`IniFile.type` is ``None``

        Returns
        -------
        Optional[:class:`ModType`]
            The type of mod identified
        """

        if (self._type is not None):
            return self._type
        elif (self.defaultModType is not None):
            return self.defaultModType
        
        return None

    def read(self) -> str:
        """
        Reads the .ini file :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will read the existing value from :attr:`IniFile.fileTxt`

        Returns
        -------
        :class:`str`
            The text content of the .ini file
        """

        if (self._filePath is not None):
            self.fileTxt = FileService.read(self._filePath.path, "r", lambda filePtr: filePtr.read())
        return self._fileTxt
    
    def write(self, txt: Optional[str] = None) -> str:
        """
        Writes back into the .ini files based off the content in :attr:`IniFile._fileLines`

        Parameters
        ----------
        txt: Optional[:class:`str`]
            The text to write back into the .ini file :raw-html:`<br />` :raw-html:`<br />`

            If this argument is ``None``, then will use the :attr:`IniFile.fileTxt`

            **Default**: ``none``

        Returns
        -------
        :class:`str`
            The text that is written to the .ini file
        """

        if (self._filePath is None and txt is not None):
            self.fileTxt = txt

        if (self._filePath is None):
            return self._fileTxt
        
        if (txt is None):
            txt = self._fileTxt

        with open(self._filePath.path, "w", encoding = FileEncodings.UTF8.value) as f:
            f.write(txt)

        return txt

    def _setupFileLines(self, fileTxt: str = ""):
        if (self._filePath is None):
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

        if (self._filePath is not None):
            self.fileLines = FileService.read(self._filePath.path, "r", lambda filePtr: filePtr.readlines())
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
    
    def getTexEditModels(self) -> List[IniTexModel]:
        """
        Retrieves all the file path data needed for editing a texture .dds file
        (transforms :attr:`IniFile.texEditModels` to a list)

        Returns
        -------
        List[:class:`IniTexModel`]
            The data models needed for editting a texture .dds file
        """

        result = []

        for texName in self.texEditModels:
            texTypeModels = self.texEditModels[texName]
            for section in texTypeModels:
                result.append(texTypeModels[section])

        return result
    
    def getTexAddModels(self) -> List[IniTexModel]:
        """
        Retrieves all the file path data needed for creating new texture .dds file
        (transforms :attr:`IniFile.texAddModels` to a list)

        Returns
        -------
        List[:class:`IniTexModel`]
            The data models needed for editting a texture .dds file
        """

        result = []

        for texName in self.texAddModels:
            texTypeModels = self.texAddModels[texName]
            for modObj in texTypeModels:
                result.append(texTypeModels[modObj])

        return result
    
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

        if (self._textureOverrideBlendRoot is not None):
            return

        hasDefaultWithoutBlendSectinFound = bool(self.defaultModType is not None and self._textureOverrideBlendSectionName is None)
        blendPatternMatch = None

        if (hasDefaultWithoutBlendSectinFound):
            blendPatternMatch = self._textureOverrideBlendPattern.search(line)

            if (blendPatternMatch):
                self._isModIni = True
                self._textureOverrideBlendSectionName = self._getSectionName(line)
        
        if (not self.modTypes and ((blendPatternMatch is not None and blendPatternMatch) or (blendPatternMatch is None and self._textureOverrideBlendPattern.search(line)))):
            self._textureOverrideBlendRoot = self._getSectionName(line)
            self._isModIni = True
            return

        for modType in self.modTypes:
            if (modType.isType(line)):
                self._textureOverrideBlendRoot = self._getSectionName(line)
                self._setType(modType)
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

        # delete any previously saved sections
        try:
            self._parser[sectionName]
        except KeyError:
            pass
        else:
            del self._parser[sectionName]

        # eliminate all indented tabs/spaces
        srcTxt = re.sub(r"\n([( |\t)]+)", r"\n", srcTxt)

        # parse the section
        try:
            self._parser.read_string(srcTxt)
            result = self._parser[sectionName]
        except:
            return result

        if (self._parserDictType == KeepAllDict):
            sectionOpts = {}
            for varName in result:
                sectionOpts[varName] = self._parser.get(sectionName, varName, raw = True)

            result = sectionOpts
            for key in result:
                currentValues = result[key]
                result[key] = []

                for val in currentValues:
                    if (not val):
                        continue

                    currentValue = val.split("_", 1)
                    currentValue[0] = int(currentValue[0])
                    result[key].append(tuple(currentValue))
        else:
            result = dict(result)

        try:
            save[sectionName] = result
        except TypeError:
            pass
        
        #print(f"RESULLULULUT: {result}")
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
        partIndices = []

        for sectionName, ranges in rangesToRemove.items():
            for range in ranges:
                partIndices.append(range)

        self.fileLines = TextTools.removeLines(self._fileLines, partIndices)

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

            if (currentPart):
                ifTemplate.append(currentPart)

        # create the if template
        result = IfTemplate.build(ifTemplate, name = sectionName)
        return result
    

    def getIfTemplates(self, flush: bool = False) -> Dict[str, IfTemplate]:
        """
        Retrieves all the :class:`IfTemplate`s for the .ini file

        .. note::
            This is the same as :meth:`IniFile.readIfTemplates`, but uses caching

        Parameters
        ----------
        flush: :class:`bool`
            Whether to re-parse the :class:`IfTemplates`s instead of using the saved cached values

        Returns
        -------
        Dict[:class:`str`, :class:`IfTempalte`]
            The parsed :class:`IfTemplate`s :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the name of the :class:`IfTemplate`
            * The values are the corresponding :class:`IfTemplate`
        """

        if (not self._ifTemplatesRead or flush):
            self.readIfTemplates()
        return self.sectionIfTemplates

    def readIfTemplates(self) -> Dict[str, IfTemplate]:
        """
        Parses all the :class:`IfTemplate`s for the .ini file

        Returns
        -------
        Dict[:class:`str`, :class:`IfTempalte`]
            The parsed :class:`IfTemplate`s :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the name of the :class:`IfTemplate`
            * The values are the corresponding :class:`IfTemplate`
        """

        self.sectionIfTemplates = self.getSectionOptions(self._sectionPattern, postProcessor = self._processIfTemplate, handleDuplicateFunc = lambda duplicates: duplicates[0])
        self._ifTemplatesRead = True
        return self.sectionIfTemplates 
    
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
    
    @classmethod
    def compareResources(cls, resourceTuple1: Tuple[str, Optional[int]], resourceTuple2: Tuple[str, Optional[int]]) -> int:
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
    def disIni(self, makeCopy: bool = False):
        """
        Disables the .ini file

        .. note::
            For more info, see :meth:`FileService.disableFile` 

        Parameters
        ----------
        makeCopy: :class:`bool`
            Whether to make a copy of the disabled .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (self._filePath is None):
            return

        disabledPath = FileService.disableFile(self._filePath.path)
        if (makeCopy):
            FileService.copyFile(disabledPath, self._filePath.path)

    @classmethod
    def getFixedBlendFile(cls, blendFile: str, modName: str = "") -> str:
        """
        Retrieves the file path for the fixed RemapBlend.buf file

        Parameters
        ----------
        blendFile: :class:`str`
            The file path to the original Blend.buf file

        modName: :class:`str`
            The name of the mod to fix to

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapBlend.buf file
        """

        blendFolder = os.path.dirname(blendFile)
        blendBaseName = os.path.basename(blendFile)
        blendBaseName = blendBaseName.rsplit(".", 1)[0]
        
        return os.path.join(blendFolder, f"{cls.getRemapBlendName(blendBaseName, modName = modName)}{FileExt.Buf.value}")
    
    @classmethod
    def getFixedTexFile(cls, texFile: str, modName: str = "") -> str:
        """
        Retrieves the file path for the fixed RemapTex.dds file

        Parameters
        ----------
        texFile: :class:`str`
            The file path to the original .dds file

        modName: :class:`str`
            The name of the mod to fix to

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapTex.dds file
        """

        blendFolder = os.path.dirname(texFile)
        blendBaseName = os.path.basename(texFile)
        blendBaseName = blendBaseName.rsplit(".", 1)[0]

        return os.path.join(blendFolder, f"{cls.getRemapTexName(blendBaseName, modName = modName)}{FileExt.DDS.value}")
    
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

        return modTypeName

    def getHeadingName(self):
        """
        Retrieves the title for the header of the divider comment of the fix

        Returns
        -------
        :class:`str`
            The title for the header of the divider comment
        """

        result = self.getFixModTypeHeadingname()
        if (result):
            result += " "

        return f"{result}Remap"

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
            self._heading.title = self.getHeadingName()
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
            self._heading.title = self.getHeadingName()
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
        shortModTypeName = modTypeName

        if (modTypeName is None):
            modTypeName = "Mod"
            shortModTypeName = ""

        if (modTypeName):
            modTypeName += " "
        
        if (shortModTypeName):
            shortModTypeName += " "
        
        return IniBoilerPlate.Credit.value.replace(IniBoilerPlate.ModTypeNameReplaceStr.value, modTypeName).replace(IniBoilerPlate.ShortModTypeNameReplaceStr.value, shortModTypeName)
    
    def addFixBoilerPlate(self, fix: str = "") -> str:
        """
        Adds the boilerplate code to identify the .ini `sections`_ have been changed by this fix

        Parameters
        ----------
        fix: :class:`str`
            The content for the fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The fix with the boilerplate code included
        """

        result = self.getFixHeader()
        result += self.getFixCredit()
        result += fix
        result += self.getFixFooter()
        return result

    def fixBoilerPlate(func):
        """
        Decorator used to add the boilerplate code to identify a code section has been changed by this fix in the .ini file

        Examples
        --------
        .. code-block:: python
            :linenos:

            @fixBoilerPlate
            def helloWorld(self) -> str:
                return "Hello World"
        """

        def addFixBoilerPlateWrapper(self, *args, **kwargs):
            fix = func(self, *args, **kwargs)
            fix = self.addFixBoilerPlate(fix = fix)
            return fix
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

        if (not name.startswith(IniKeywords.Resource.value)):
            name = f"{IniKeywords.Resource.value}{name}"
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

        if (name.startswith(IniKeywords.Resource.value)):
            name = name[len(IniKeywords.Resource.value):]

        return name
    
    @classmethod
    def getRemapBlendName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapBlend' to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniFile.getRemapBlendName("EiTriesToUseBlenderAndFails", "Raiden")
        "EiTriesToUseRaidenRemapBlenderAndFails"


        >>> IniFile.getRemapBlendName("EiBlendsTheBlender", "Yae")
        "EiBlendsTheYaeRemapBlender"
    

        >>> IniFile.getRemapBlendName("ResourceCuteLittleEi", "Raiden")
        "ResourceCuteLittleEiRaidenRemapBlend"


        >>> IniFile.getRemapBlendName("ResourceCuteLittleEiRemapBlend", "Raiden")
        "ResourceCuteLittleEiRemapRaidenRemapBlend"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapBlend' keyword
        """

        nameParts = name.rsplit(IniKeywords.Blend.value, 1)
        namePartsLen = len(nameParts)

        remapName = f"{modName}{IniKeywords.RemapBlend.value}"
        if (namePartsLen > 1):
            name = remapName.join(nameParts)
        else:
            name += remapName

        return name
    
    @classmethod
    def getModSuffixedName(cls, name: str, suffix: str = "", modName: str = ""):
        """
        Changes a `section`_ name to have the suffix of 'modName' followed by 'suffix'

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        suffix: :class:`str`
            The name of the suffix to put at the end of the `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added suffix keyword
        """

        remapName = f"{modName}{suffix}"
        if (name.endswith(remapName)):
            return name
        elif(name.endswith(suffix)):
            return name[:len(suffix)] + remapName

        return name + remapName
    
    @classmethod
    def getRemapFixName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the suffix `RemapFix` to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniFile.getRemapFixName("EiIsDoneWithRemapFix", "Raiden")
        "EiIsDoneWithRaidenRemapFix"

        >>> IniFile.getRemapFixName("EiIsHappy", "Raiden")
        "EiIsHappyRaidenRemapFix"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapFix' keyword
        """

        return cls.getModSuffixedName(name, suffix = IniKeywords.RemapFix.value, modName = modName)
    
    @classmethod
    def getRemapTexName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to have the suffix `RemapFix` to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniFile.getRemapTexName("EiIsDoneWithRemapTex", "Raiden")
        "EiIsDoneWithRaidenRemapTex"

        >>> IniFile.getRemapTexName("EiIsHappy", "Raiden")
        "EiIsHappyRaidenRemapTex"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapFix' keyword
        """

        return cls.getModSuffixedName(name, suffix = IniKeywords.RemapTex.value, modName = modName)

    @classmethod
    def getRemapFixResourceName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to be a new non-blend resource created by this fix

        .. note::
            See :meth:`IniFile.getResourceName` and :meth:`IniFile.getRemapFix` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the suffix 'RemapFix' added
        """

        name = cls.getRemapFixName(name, modName = modName)
        name = cls.getResourceName(name)
        return name
    
    @classmethod
    def getRemapTexResourceName(cls, name: str, modName: str = ""):
        """
        Changes a `section`_ name to be a texture resource created by this fix

        .. note::
            See :meth:`IniFile.getResourceName` and :meth:`IniFile.getRemapTexName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the suffix 'RemapFix' added
        """

        name = cls.getRemapTexName(name, modName = modName)
        name = cls.getResourceName(name)
        return name

    @classmethod
    def getRemapBlendResourceName(cls, name: str, modName: str = "") -> str:
        """
        Changes the name of a section to be a new resource that this fix will create

        .. note::
            See :meth:`IniFile.getResourceName` and :meth:`IniFile.getRemapBlendName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the keyword 'Remap' added
        """

        name = cls.getRemapBlendName(name, modName = modName)
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

        return IniKeywords.Vb1.value in ifTemplatePart
    
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


        return IniKeywords.Draw.value in ifTemplatePart
    
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

        return ifTemplatePart[IniKeywords.Vb1.value]
    
    # fills the if..else template in the .ini for each section
    def fillIfTemplate(self, modName: str, sectionName: str, ifTemplate: IfTemplate, fillFunc: Callable[[str, str, Union[str, Dict[str, Any]], int, int, str], str], origSectionName: Optional[str] = None) -> str:
        """
        Creates a new :class:`IfTemplate` for an existing `section`_ in the .ini file

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name of the `section`_

        ifTemplate: :class:`IfTemplate`
            The :class:`IfTemplate` of the orginal `section`_

        fillFunc: Callable[[:class:`str`, :class:`str`, Union[:class:`str`, Dict[:class:`str`, Any], :class:`int`, :class:`str`, :class:`str`], :class:`str`]]
            The function to create a new **content part** for the new :class:`IfTemplate`
            :raw-html:`<br />` :raw-html:`<br />`

            .. tip::
                For more info about an 'IfTemplate', see :class:`IfTemplate`

            :raw-html:`<br />`
            The parameter order for the function is:

            #. The name for the type of mod to fix to
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
            addFix += fillFunc(modName, sectionName, part, partIndex, linePrefix, origSectionName)

            partIndex += 1
            
        return addFix
    
    

    # _getCommonMods(): Retrieves the common mods that need to be fixed between all target graphs
    #   that are used for the fix
    def _getCommonMods(self) -> Set[str]:
        if (self._type is None):
            return set()
        
        result = set()
        hashes = self._type.hashes
        indices = self._type.indices

        graphs = [self._blendCommandsGraph, self._nonBlendHashIndexCommandsGraph, self._resourceCommandsGraph]
        for graph in graphs:
            commonMods = graph.getCommonMods(hashes, indices, version = self.version)
            if (not result):
                result = commonMods
            else:
                result = result.intersection(commonMods)

        return result
    

    def _setToFix(self) -> Set[str]:
        """
        Sets the names for the types of mods that will used in the fix

        Returns
        -------
        Set[:class:`str`]
            The names of the mods that will be used in the fix        
        """

        commonMods = self._getCommonMods()
        toFix = commonMods.intersection(self.modsToFix)
        type = self.availableType

        if (not toFix and type is not None):
            self._toFix = type.getModsToFix()
        elif (not toFix):
            self._toFix = commonMods
        else:
            self._toFix = toFix

        return self._toFix

    # _makeRemapNames(): Makes the required names used for the fix
    def _makeRemapNames(self):
        self._blendCommandsGraph.getRemapBlendNames(self._toFix)
        self._nonBlendHashIndexCommandsGraph.getRemapBlendNames(self._toFix)
        self._resourceCommandsGraph.getRemapBlendNames(self._toFix)

    # _getCommonMods(): Retrieves the common mods that need to be fixed between all target graphs
    #   that are used for the fix
    def _getCommonMods(self) -> Set[str]:
        if (self._type is None):
            return set()
        
        result = set()
        hashes = self._type.hashes
        indices = self._type.indices

        graphs = [self._blendCommandsGraph, self._nonBlendHashIndexCommandsGraph, self._resourceCommandsGraph]
        for graph in graphs:
            commonMods = graph.getCommonMods(hashes, indices, version = self.version)
            if (not result):
                result = commonMods
            else:
                result = result.intersection(commonMods)

        return result
    

    def _setToFix(self) -> Set[str]:
        """
        Sets the names for the types of mods that will used in the fix

        Returns
        -------
        Set[:class:`str`]
            The names of the mods that will be used in the fix        
        """

        commonMods = self._getCommonMods()
        toFix = commonMods.intersection(self.modsToFix)
        type = self.availableType

        if (not toFix and type is not None):
            self._toFix = type.getModsToFix()
        elif (not toFix):
            self._toFix = commonMods
        else:
            self._toFix = toFix

        return self._toFix

    
    # _makeRemapNames(): Makes the required names used for the fix
    def _makeRemapNames(self):
        self._blendCommandsGraph.getRemapBlendNames(self._toFix)
        self._nonBlendHashIndexCommandsGraph.getRemapBlendNames(self._toFix)
        self._resourceCommandsGraph.getRemapBlendNames(self._toFix)


    # _getRemapName(sectionName, modName, sectionGraph, remapNameFunc): Retrieves the required remap name for the fix
    def _getRemapName(self, sectionName: str, modName: str, sectionGraph: Optional[IniSectionGraph] = None, remapNameFunc: Optional[Callable[[str, str], str]] = None) -> str:
        error = False
        if (sectionGraph is None):
            error = True

        if (not error):
            try:
                return sectionGraph.remapNames[sectionName][modName]
            except KeyError:
                error = True

        if (sectionName not in self.sectionIfTemplates):
            return sectionName

        if (remapNameFunc is None):
            remapNameFunc = self.getRemapBlendName

        return remapNameFunc(sectionName, modName)
    

    def _getFixer(self):
        """
        Retrieves the fixer for fixing the .ini file

        Returns
        -------
        Optional[:class:`BaseIniFixer`]
            The resultant fixer
        """

        availableType = self.availableType
        if (availableType is not None and self._iniParser is not None and self._iniFixer is None):
            self._iniFixer = availableType.iniFixBuilder.build(self._iniParser)
        
        return self._iniFixer
    
    # _getFixStr(fix, withBoilerPlate): Internal functino to get the needed lines to fix the .ini file
    def _getFixStr(self, fix: str = "", withBoilerPlate: bool = True) -> str:
        fixer = self._getFixer()
        availableType = self.availableType

        if (fixer is None and availableType is not None):
            return fix
        elif (availableType is None):
            raise NoModType()

        result = fixer.getFix(fixStr = fix)

        if (withBoilerPlate):
            return self.addFixBoilerPlate(fix = result)
        return result

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

        return self._getFixStr(fix = fix)

    @_readLines
    def injectAddition(self, addition: str, beforeOriginal: bool = True, keepBackup: bool = True, fixOnly: bool = False, update: bool = False) -> str:
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

        update: :class:`bool`
            Whether to update the source text within this class to reflect the new addition :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The content of the .ini file with the new text added
        """

        original = self._fileTxt
        if (keepBackup and fixOnly and self._filePath is not None):
            self.print("log", "Cleaning up and disabling the OLD STINKY ini")
            self.disIni()

        result = ""
        if (beforeOriginal):
            result = f"{addition}\n\n{original}"
        else:
            result = f"{original}\n{addition}"

        # writing the fixed file
        if (self._filePath is not None):
            with open(self._filePath.path, "w", encoding = FileEncodings.UTF8.value) as f:
                f.write(result)

        # update the source text
        if (update):
            self._fileTxt = result
            self._fileLines = TextTools.getTextLines(result)

        self._isFixed = True
        return result

    def _removeFix(self, parse: bool = False) -> str:
        """
        Removes any previous changes that were probably made by this script :raw-html:`<br />` :raw-html:`<br />`

        For the .ini file will remove:

        #. All code surrounded by the *'---...--- .* Fix ---...----'* header/footer
        #. All `sections`_ containing the keywords ``RemapBlend``

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file with the changes removed
        """

        self._getRemover()
        return self._iniRemover.remove(parse = parse)
    
    def _getRemover(self) -> BaseIniRemover:
        """
        Retrieves the remover for removing fixes from the .ini file

        Returns
        -------
        :class:`BaseIniRemover`
            The resultant parser
        """

        availableType = self.availableType

        if (availableType is not None and self._iniRemover is None):
            self._iniRemover = availableType.iniRemoveBuilder.build(self)
            self._iniRemover.iniFile = self
        elif (self._iniRemover is None):
            self._iniRemover = GlobalIniRemoveBuilder.build(self)
        
        return self._iniRemover

    @_readLines
    def removeFix(self, keepBackups: bool = True, fixOnly: bool = False, parse: bool = False) -> str:
        """
        Removes any previous changes that were probably made by this script and creates backup copies of the .ini file

        .. tip::
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

        parse: :class:`bool`
            Whether to also parse for the .*RemapBlend.buf files that need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file with the changes removed
        """

        if (keepBackups and not fixOnly and self._filePath is not None):
            self.print("log", f"Creating Backup for {self._filePath.base}")
            self.disIni(makeCopy = True)

        if (fixOnly):
            return self._fileTxt

        if (self._filePath is not None):
            self.print("log", f"Removing any previous changes from this script in {self._filePath.base}")

        result = self._removeFix(parse = parse)
        return result
    
    def makeResourceModel(self, ifTemplate: IfTemplate, toFix: Set[str], getFixedFile: Optional[Callable[[str, str], str]] = None,
                          iniResourceModelCls: Type[IniResourceModel] = IniResourceModel, 
                          iniResModelArgs: Optional[List[Any]] = None, iniResModelKwargs: Optional[Dict[str, Any]] = None) -> IniResourceModel:
        """
        Creates the data needed for fixing a particular ``[Resource.*]`` `section`_ in the .ini file

        Parameters
        ----------
        ifTemplate: :class:`IfTemplate`
            The particular `section`_ to extract data

        toFix: Set[:class:`str`]
            The names of the mods to fix 

        getFixedFile: Optional[Callable[[:class:`str`, :class:`str`], :class:`str`]]
            The function for transforming the file path of a found resource file into a new file path for the fixed resources file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :meth:`IniFile.getFixedBlendFile` :raw-html:`<br />` :raw-html:`<br />`

            The parameters for the function are:

                # The path to the original file
                # The type of mod to fix to

            **Default**: ``None``

        iniResourceModelCls: Type[:class:`IniResourceModel`]
            A subclass of :class:`IniResourceModel` for constructing the required data

            .. attention::
                The constructor of this subclass must at least have the same arguments and keyword arguments
                as the constructor for :class:`IniResourceModels`

             **Default**: :class:`IniResourceModel`

        iniResModelArgs: Optional[List[Any]]
            Any arguments to add onto the contructor for creating the subclass of a :class:`IniResourceModel` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        iniResModelKwargs: Optional[Dict[:class:`str`, Any]]
            Any keyword arguments to add onto the constructor for creating the subclass of a :class:`IniResourceModel` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`IniResourceModel`
            The data for fixing the particular resource
        """

        folderPath = self.folder
        if (getFixedFile is None):
            getFixedFile = self.getFixedBlendFile

        origResPaths = {}
        fixedResPaths = {}
        partIndex = 0

        for part in ifTemplate:
            if (isinstance(part, IfPredPart)):
                partIndex += 1
                continue

            currentOrigResPaths = []
            try:
                currentOrigResPaths = part[IniKeywords.Filename.value]
            except KeyError:
                partIndex += 1
                continue
            
            currentOrigResPaths = list(map(lambda pathData: FileService.parseOSPath(pathData[1]), currentOrigResPaths))
            origResPaths[partIndex] = currentOrigResPaths

            for modName in toFix:
                currentFixedResPaths = list(map(lambda origBlendFile: getFixedFile(origBlendFile, modName = modName), currentOrigResPaths))

                try:
                    fixedResPaths[partIndex]
                except KeyError:
                    fixedResPaths[partIndex] = {}

                fixedResPaths[partIndex][modName] = currentFixedResPaths

            partIndex += 1

        if (iniResourceModelCls == IniResourceModel): 
            return IniResourceModel(folderPath, fixedResPaths, origPaths = origResPaths)

        if (iniResModelKwargs is None):
            iniResModelKwargs = {}

        if (iniResModelArgs is None):
            iniResModelArgs = []

        return iniResourceModelCls(folderPath, fixedResPaths, *iniResModelArgs, origPaths = origResPaths, **iniResModelKwargs)
    
    def makeTexModel(self, ifTemplate: IfTemplate, toFix: Set[str], texEditors: Union[BaseTexEditor, Dict[int, Dict[str, List[BaseTexEditor]]]], 
                     getFixedFile: Optional[Callable[[str, str], str]] = None) -> IniTexModel:
        """
        Creates the data needed for fixing a particular ``[Resource.*]`` `section`_ for some .dds texture file in the .ini file

        Parameters
        ----------
        ifTemplate: :class:`IfTemplate`
            The particular `section`_ to extract data

        toFix: Set[:class:`str`]
            The names of the mods to fix 

        texEditors: Union[:class:`BaseTexEditor`, Dict[:class:`int`, Dict[:class:`str`, List[:class:`BaseTexEditor`]]]]
            The texture editors for editting the found .dds files :raw-html:`<br />` :raw-html:`<br />`

            * If this argument is of type :class:`BaseTexEditor`, then all .dds files encountered within the parsed `section`_ will use the same texture editor
            * If this argument is a dictionary, then the structure of the dictionary is follows the same structure as :attr:`IniTexModel.texEdits`

        getFixedFile: Optional[Callable[[:class:`str`, :class:`str`], :class:`str`]]
            The function for transforming the file path of a found .dds file into a new file path to the fixed .dds file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :meth:`IniFile.getFixedBlendFile` :raw-html:`<br />` :raw-html:`<br />`

            The parameters for the function are:

                # The path to the original file
                # The type of mod to fix to

            **Default**: ``None``

        Returns
        -------
        :class:`IniTexModel`
            The data for fixing the particular texture
        """

        texEdits = {}
        if (isinstance(texEditors, dict)):
            texEdits = texEditors

        elif (isinstance(texEditors, BaseTexEditor)):
            partIndex = 0
            for part in ifTemplate:
                if (isinstance(part, IfPredPart)):
                    partIndex += 1
                    continue

                currentOrigResPaths = []
                try:
                    currentOrigResPaths = part[IniKeywords.Filename.value]
                except KeyError:
                    partIndex += 1
                    continue

                for modName in toFix:
                    currentEditors = list(map(lambda origBlendFile: texEditors, currentOrigResPaths))

                    try:
                        texEdits[partIndex]
                    except KeyError:
                        texEdits[partIndex] = {}

                    texEdits[partIndex][modName] = currentEditors

        return self.makeResourceModel(ifTemplate, toFix, getFixedFile, iniResourceModelCls = IniTexModel, iniResModelArgs = [texEdits])

    def _getSubCommands(self, ifTemplate: IfTemplate, currentSubCommands: Set[str], subCommands: Set[str], subCommandLst: List[str]):
        for partIndex in ifTemplate.calledSubCommands:
            partSubCommands = ifTemplate.calledSubCommands[partIndex]

            for subCommandData in partSubCommands:
                subCommand = subCommandData[1]
                if (subCommand not in subCommands):
                    currentSubCommands.add(subCommand)
                    subCommands.add(subCommand)
                    subCommandLst.append(subCommand)

    def _getCommandIfTemplate(self, sectionName: str, raiseException: bool = True) -> Optional[IfTemplate]:
        """
        Retrieves the :class:`IfTemplate` for a certain `section`_ from `IniFile._sectionIfTemplate`

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
            ifTemplate = self.sectionIfTemplates[sectionName]
        except Exception as e:
            if (raiseException):
                raise KeyError(f"The section by the name '{sectionName}' does not exist") from e
            else:
                return None
        else:
            return ifTemplate

    @classmethod
    def getResources(cls, commandsGraph: IniSectionGraph, isIfTemplateResource: Callable[[IfContentPart], Any],
                     getIfTemplateResource: Callable[[IfContentPart], str], addResource: Callable[[Any, IfContentPart], Any]):
        """
        Retrieves all the referenced resources that were called by `sections`_ related to the ``[TextureOverride.*Blend.*]`` `sections`_

        Parameters
        ----------
        resources: Set[:class:`str`]
            The result for all the resource `sections`_ that were referenced

        commandsGraph: :class:`IniSectionGraph`
            The subgraph for all the `sections`_ related to the resource

        isIfTemplateResource: Callable[[:class:`IfContentPart`], :class:`bool`]
            Checks whether a part in the :class:`IfTemplate` of a `section`_ contains the key that reference the target resource

        getIfTemplateResource: Callable[[:class:`IfContentPart`], Any]
            Function to retrieve the target resource from a part in the :class:`IfTemplate` of a `section`_

        addResource: Callable[[Any, :class:`IfContentPart`], Any]
            Function to add in the result of the found resource `section`_

            :raw-html:`<br />`
            The parameter order for the function is:

            #. the retrieved resource `section`_
            #. the part in the :class:`IfTemplate` where the resource is found
        """

        sections = commandsGraph.sections
        for sectionName in sections:
            ifTemplate = sections[sectionName]

            for part in ifTemplate:
                if (isinstance(part, IfPredPart)):
                    continue

                if (isIfTemplateResource(part)):
                    resource = getIfTemplateResource(part)
                    addResource(resource, part)

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


    # getTargetHashAndIndexSections(blendCommandNames): Retrieves the sections with target hashes and indices
    def getTargetHashAndIndexSections(self, blendCommandNames: Set[str]) -> Set[IfTemplate]:
        if (self._type is None and self.defaultModType is None):
            return set()
        
        type = self._type
        if (self._type is None):
            type = self.defaultModType

        result = {}
        hashes = set(type.hashes.fromAssets)
        indices = set(type.indices.fromAssets)
        
        # get the sections with the hashes/indices
        for sectionName in self.sectionIfTemplates:
            ifTemplate = self.sectionIfTemplates[sectionName]
            if (sectionName in blendCommandNames):
                continue

            if (hashes.intersection(ifTemplate.hashes) or indices.intersection(ifTemplate.indices)):
                result[sectionName] = ifTemplate

        return result
    
    def _getParser(self) -> Optional[BaseIniParser]:
        """
        Retrieves the parser for parsing the .ini file

        Returns
        -------
        Optional[:class:`BaseIniParser`]
            The resultant parser
        """

        availableType = self.availableType
        if (availableType is not None and self._iniParser is None):
            self._iniParser = availableType.iniParseBuilder.build(self)
        
        return self._iniParser


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
        self.remapBlendModels.clear()
        self.texAddModels.clear()
        self.texEditModels.clear()

        self.getIfTemplates(flush = True)
        if (self.defaultModType is not None and self._textureOverrideBlendSectionName is not None and self._textureOverrideBlendRoot is None):
            self._textureOverrideBlendRoot = self._textureOverrideBlendSectionName

        parser = self._getParser()
        try:
            self.sectionIfTemplates[self._textureOverrideBlendRoot]
        except KeyError:
            if (self._iniParser is not None):
                self._iniParser.clear()
            return

        if (parser is not None):
            parser.clear()
        else:
            return

        parser.parse()

    # _fix(keepBackup, fixOnly, update, withBoilerPlate, withSrc): Internal function to fix the .ini file
    def _fix(self, keepBackup: bool = True, fixOnly: bool = False, update: bool = False, withBoilerPlate: bool = True, withSrc: bool = True) -> str:
        fix = ""
        fix += self._getFixStr(fix = fix, withBoilerPlate = withBoilerPlate)

        if (withBoilerPlate):
            fix = f"\n\n{fix}"
        
        if (not withSrc):
            self._isFixed = True
            return fix

        fix = self.injectAddition(fix, beforeOriginal = False, keepBackup = keepBackup, fixOnly = fixOnly, update = update)
        self._isFixed = True
        return fix

    def fix(self, keepBackup: bool = True, fixOnly: bool = False, update: bool = False) -> Union[str, List[str]]:
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

        update: :class:`bool`
            Whether to also update the source text of this classs with the fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Union[:class:`str`, List[:class:`str`]]
            The new content of the .ini file which includes the fix and the new content of any other newly created .ini files related to fixing the particular .ini file
        """

        fixer = self._getFixer()
        availableType = self.availableType

        if (availableType is None):
            raise NoModType()
        elif (fixer is None):
            return

        return fixer.fix(keepBackup = keepBackup, fixOnly = fixOnly, update = update)
##### EndScript