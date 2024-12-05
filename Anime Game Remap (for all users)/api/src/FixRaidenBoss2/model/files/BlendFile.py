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
import struct
from typing import Union, Optional, Tuple, Dict, List
##### EndExtImports

##### LocalImports
from .File import File
from ..VGRemap import VGRemap
from ...exceptions.BlendFileNotRecognized import BlendFileNotRecognized
from ...exceptions.BadBlendData import BadBlendData
from ...tools.files.FileService import FileService
##### EndLocalImports


##### Script
class BlendFile(File):
    """
    This Class inherits from :class:`File`

    Used for handling blend.buf files

    .. note::
        We observe that a Blend.buf file is a binary file defined as:

        * each line contains 32 bytes (256 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
        * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
        * the blend weights are floating points while the blend indices are unsigned integers

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the blend file

    Attributes
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the blend file

    _data: :class:`bytes`
        The bytes read from the source
    """

    BytesPerLine = 32

    def __init__(self, src: Union[str, bytes]):
        self.src = src
        self._data = self.read()

    def read(self) -> bytes:
        """
        Reads the bytes in the blend.buf file

        Returns
        -------
        :class:`bytes`
            The read bytes
        """

        return self.readFile(self.src)

    @classmethod
    def readFile(cls, blendSrc: Union[str, bytes]):
        result = FileService.readBinary(blendSrc)
        isValid = cls._isValid(result)

        if (not isValid and isinstance(blendSrc, str)):
            raise BlendFileNotRecognized(blendSrc)
        elif (not isValid):
            raise BadBlendData()
        
        return result

    @classmethod
    def _getLineWeight(cls, data: bytes, lineInd: int) -> Tuple[float, float, float, float]:
        return [struct.unpack("<f", data[lineInd + 4 * j : lineInd + 4 * (j+1)])[0] for j in range(4)]
    
    @classmethod
    def _getLineIndices(cls, data: bytes, lineInd: int) -> Tuple[int, int, int, int]:
        return [struct.unpack("<I", data[lineInd + 16 + 4 * j : lineInd + 16 + 4 * (j+1)])[0] for j in range(4)]

    @classmethod
    def _isValid(cls, data: bytes):
        if (len(data) % cls.BytesPerLine != 0):
            return False
        return True

    def correct(self, vgRemap: VGRemap, fixedBlendFile: Optional[str] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Blend.buf file

        Parameters
        ----------
        vgRemap: :class:`VGRemap`
            The vertex group remap for correcting the Blend.buf file

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
        blendFile = self.src
        blendIsFile = isinstance(blendFile, str)
        if (not vgRemap.remap and blendIsFile):
            return None
        elif (not vgRemap.remap):
            return bytearray(blendFile)

        result = bytearray()
        dataLen = len(self._data)
        for i in range(0,dataLen,32):
            blendweights = self._getLineWeight(self._data, i)
            blendindices = self._getLineIndices(self._data, i)
            outputweights = bytearray()
            outputindices = bytearray()

            # replaces the blend index in the original mod with the corresponding blend index
            #   for the boss
            for weight, index in zip(blendweights, blendindices):
                if weight != 0 and index <= vgRemap.maxIndex:
                    try:
                        index = int(vgRemap.remap[index])
                    except KeyError:
                        pass

                outputweights += struct.pack("<f", weight)
                outputindices += struct.pack("<I", index)
            result += outputweights
            result += outputindices

        if (fixedBlendFile is not None):
            FileService.writeBinary(fixedBlendFile, result)
            return fixedBlendFile

        return result

    @classmethod
    def _addRemap(cls, hasRemap: bool, remap: Dict[bytes, Union[bytes, List[bytes]]], key: bytes, value: bytes) -> bool:
        currentIsRemap = True
        try:
            remap[key]
        except KeyError:
            remap[key] = value
        else:
            remapValue = remap[key]

            if (remapValue != value):
                currentIsRemap = False

                if (not isinstance(remapValue, list)):
                    remap[key] = [remapValue]

                remap[key].append(value)

        return (hasRemap and currentIsRemap)
##### EndScript