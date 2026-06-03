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
from typing import Union, List, Dict, Optional, Callable, Any
##### EndExtImports

##### LocalImports
from ...tools.files.FileService import FileService
from ...exceptions.BufFileNotRecognized import BufFileNotRecognized
from ...exceptions.BadBufData import BadBufData
from .BinaryFile import BinaryFile
from ..buffers.BufElementType import BufElementType
##### EndLocalImports


##### Script
class BufFile(BinaryFile):
    """
    This class inherits from :class:`BinaryFile`

    A class to handle .buf files

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the .buf file

    elements: List[:class:`BufElementType`]
        The sequence of elements within the .buf file

    fileType: :class:`str`
        The name for the type of .buf file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``Buffer``

    Attributes
    ----------
    fileType: :class:`str`
        The name for the type of .buf file

    bytesPerLine: :class:`int`
        The number of bytes per line in the .buf file
    """

    def __init__(self, src: Union[str, bytes], elements: List[BufElementType], fileType: str = "Buffer"):
        super().__init__(src)
        self._elementsInd: Dict[str, int] = {}
        self._elementsDict: Dict[str, int] = {}
        self._bytesPerLine = 0
        self.elements = elements
        self.fileType = fileType

        self.read()

    @property
    def elements(self) -> List[BufElementType]:
        """
        The sequence of elements within the .buf file

        :getter: Retrieves the elements
        :setter: Sets the elements for the .buf file
        :type: List[:class:`BufElementType`]
        """

        return self._elements
    
    @elements.setter
    def elements(self, newElements: List[BufElementType]):
        self._elements = newElements
        self._elementsInd.clear()
        self._elementsDict.clear()
        self._bytesPerLine = 0

        elementsLen = len(self._elements)
        for i in range(elementsLen):
            element = self._elements[i]
            elementName = element.name
            elementInd = ""

            if (elementName not in self._elementsInd):
                self._elementsInd[elementName] = 0
            else:
                self._elementsInd[elementName] += 1
                elementInd = f"{self._elementsInd[elementName]}"

            self._elementsDict[f"{elementName}{elementInd}"] = i
            self._bytesPerLine += element.size

    @property
    def bytesPerLine(self):
        """
        The number of bytes per line (per vertex)

        :getter: Retrieves the number of bytes per line
        :type: :class:`int`
        """

        return self._bytesPerLine

    def isValid(self) -> bool:
        """
        Whether the size of the data is divisible by the # of bytes per line

        Returns
        -------
        :class:`bool`
            Whether the provided data for the .buf file is valid
        """

        if (len(self._data) % self.bytesPerLine != 0):
            return False
        return True

    def read(self) -> bytes:
        """
        Reads the bytes in the .buf file

        Returns
        -------
        :class:`bytes`
            The read bytes
        """

        """
        Reads the bytes in the blend.buf file

        Returns
        -------
        :class:`bytes`
            The read bytes
        """

        self._data = FileService.readBinary(self.src)
        isValid = self.isValid()

        if (not isValid and isinstance(self.src, str)):
            raise BufFileNotRecognized(self.src, fileType = self.fileType)
        elif (not isValid):
            raise BadBufData(fileType = self.fileType)

        return self._data
    
    def decodeLine(self, src: bytes) -> Dict[str, List[Any]]:
        """
        Decodes a line (a vertex) within the .buf file

        Parameters
        ----------
        src: :class:`bytes`
            The source bytes to decode

        Returns 
        -------
        Dict[:class:`str`, List[Any]]
            The decoded values for the line :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names to the elements and the values are what is decoded
        """

        result = {}
        startInd = 0
        endInd = 0

        for elementName in self._elementsDict:
            elementInd = self._elementsDict[elementName]
            element = self._elements[elementInd]

            elementSize = element.size
            endInd += elementSize

            result[elementName] = element.decode(src[startInd: endInd])
            startInd = endInd

        return result
    
    def encodeLine(self, src: Dict[str, List[Any]]) -> bytes:
        """
        Encodes the data about a vertex to their corresponding bytes for the line

        Parameters
        ----------
        src: Dict[:class:`str`, List[Any]]
            The corresponding data for the vertex :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names for the elements and the values are the data for the elements

        Returns 
        -------
        :class:`bytes`
            The encoded bytes for the line
        """

        result = b""

        for elementName in self._elementsDict:
            elementInd = self._elementsDict[elementName]
            element = self._elements[elementInd]

            currentSrc = src[elementName]
            result += element.encode(currentSrc)

        return result

    def fix(self, fixedFile: Optional[str] = None, filters: Optional[List[Callable[[Dict[str, List[Any]], int, int, int], Dict[str, List[Any]]]]] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes the .buf file

        Parameters
        ----------
        fixedFile: Optional[:class:`str`]
            The file path for the fixed .buf file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
            The filters to process each element :raw-html:`<br />` :raw-html:`<br />`

            The filters take in the following arguments:

            #. The data for a particular line
            #. The starting byte index of the line that is read
            #. The line index being processed
            #. The size of each line :raw-html:`<br />` :raw-html:`<br />`

            The output of the filters is the resultant data that consists where the keys are the names of the elements within a line
            in the .buf file and the values are the resultant data for each element in the line

        Raises
        ------
        :class:`BufFileNotRecognized`
            If the original .buf file provided by the :attr:`src` attribute cannot be read

        :class:`BadBufData`
            If the bytes passed into the :attr:`src` attribute do not correspond to the format defined for the .buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedFile`` is ``None``, then will return an array of bytes for the fixed .buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed .buf file if the provided .buf file got corrected
        """

        result = bytearray()
        dataLen = len(self._data)
        for i in range(0, dataLen, self._bytesPerLine):
            decodedValues = self.decodeLine(self._data[i: i + self._bytesPerLine])
            lineInd = i / self._bytesPerLine

            for filter in filters:
                decodedValues = filter(decodedValues, i, lineInd, self._bytesPerLine)

            result += self.encodeLine(decodedValues)

        if (fixedFile is not None):
            FileService.writeBinary(fixedFile, result)
            return fixedFile

        return result
##### EndScript