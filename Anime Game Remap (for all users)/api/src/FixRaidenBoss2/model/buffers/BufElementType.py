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
from typing import List, Any
##### EndExtImports

##### LocalImports
from .BufType import BufType
from .BufDataType import BufDataType
##### EndLocalImports


##### Script
class BufElementType(BufType):
    """
    This class inherits from :class:`BufType`

    The type definition for an element within a .buf file

    Parameters
    ----------
    name: :class:`str`
        The name of the element

    formatName: :class:`str`
        The name of the type format according to 3dmigoto

    dataTypes: List[:class:`BufDataType`]
        The data types composed within the element

    Attributes
    ----------
    formatName: :class:`str`
        The name of the type format according to 3dmigoto
    """

    def __init__(self, name: str, formatName: str, dataTypes: List[BufDataType]):
        super().__init__(name)
        self.formatName = formatName
        self.dataTypes = dataTypes

    @property
    def dataTypes(self) -> List[BufDataType]:
        """
        The data types composed within the element

        :getter: Retrieves the data types within the element
        :setter: Sets the new data types for the element
        :type: List[:class:`BufDataType`]
        """

        return self._dataTypes
    
    @dataTypes.setter
    def dataTypes(self, newDataTypes: List[BufDataType]):
        self._dataTypes = newDataTypes
        self._size = 0

        for dataType in self._dataTypes:
            self._size += dataType.size

    @property
    def size(self):
        """
        The byte size for the element

        :getter: The size of the element
        :type: :class:`int`
        """

        return self._size

    def decode(self, src: bytes) -> List[Any]:
        result = []
        byteStart = 0
        byteEnd = 0

        for dataType in self.dataTypes:
            byteEnd += dataType.size
            result.append(dataType.decode(src[byteStart: byteEnd]))
            byteStart = byteEnd

        return result
    
    def encode(self, src: List[Any]) -> bytes:
        result = b""
        minLen = min(len(self.dataTypes), len(src))

        for i in range(minLen):
            dataType = self.dataTypes[i]
            result += dataType.encode(src[i])

        return result
##### EndScript