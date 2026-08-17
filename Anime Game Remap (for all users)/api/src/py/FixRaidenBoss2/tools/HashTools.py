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
from typing import Hashable, Union
##### EndExtImports

##### CppLocalImports
from ..core import CppHashTools, Hash128
##### EndCppLocalImports

##### CyLocalImports
from ..CyHashTools import CyHashTools
##### EndCyLocalImports


##### Script
class HashTools(CppHashTools):
    """
    This class inherits from :class:`CppHashTools`

    Tools for custom hashing
    """

    _CyTools = CyHashTools()

    @classmethod
    def hashLibSerialize(cls, obj: Hashable) -> bytes:
        """
        Converts some hashable into deterministic bytes, suitable for the ``data``/``str``
        parameters of this class's inherited hashing methods (eg. :meth:`CppHashTools.getDeterministicHash`)

        .. note::
            This function is a convenience for calling :meth:`CyHashTools.hashLibSerialize`

        Parameters
        ----------
        obj: Hashable
            The object to convert

        Returns
        -------
        :class:`bytes`
            The resultant bytes converted from the object
        """

        return cls._CyTools.hashLibSerialize(obj)

    @classmethod
    def _toBytes(cls, data: Union[str, bytes, Hashable]) -> Union[str, bytes]:
        # 'data' that's already a str/bytes is passed straight through to the inherited C++
        # methods as-is; anything else is first run through hashLibSerialize() so those methods
        # (which only understand str/bytes) can still hash it deterministically
        if isinstance(data, (str, bytes)):
            return data
        return cls.hashLibSerialize(data)

    @classmethod
    def getDeterministicHash(cls, data: Union[str, bytes, Hashable]) -> Hash128:
        """
        Deterministically hashes 'data'

        .. note::
            Unlike :meth:`CppHashTools.getDeterministicHash`, 'data' doesn't need to be a
            :class:`str`/:class:`bytes` -- any other hashable object is first canonically
            serialized via :meth:`hashLibSerialize`

        Parameters
        ----------
        data: Union[:class:`str`, :class:`bytes`, Hashable]
            The data to hash

        Returns
        -------
        :class:`Hash128`
            The resultant deterministic hash
        """

        return CppHashTools.getDeterministicHash(cls._toBytes(data))

    @classmethod
    def getDeterministicHashStr(cls, data: Union[str, bytes, Hashable]) -> str:
        """
        Deterministically hashes 'data' into a base64 string

        .. note::
            Unlike :meth:`CppHashTools.getDeterministicHashStr`, 'data' doesn't need to be a
            :class:`str`/:class:`bytes` -- any other hashable object is first canonically
            serialized via :meth:`hashLibSerialize`

        Parameters
        ----------
        data: Union[:class:`str`, :class:`bytes`, Hashable]
            The data to hash

        Returns
        -------
        :class:`str`
            The resultant deterministic hash, as a base64 string
        """

        return CppHashTools.getDeterministicHashStr(cls._toBytes(data))

    @classmethod
    def getShortDeterministicHashStr(cls, data: Union[str, bytes, Hashable]) -> str:
        """
        Deterministically hashes 'data' into a short, compact, possibly-colliding base64 string

        .. note::
            Unlike :meth:`CppHashTools.getShortDeterministicHashStr`, 'data' doesn't need to be a
            :class:`str`/:class:`bytes` -- any other hashable object is first canonically
            serialized via :meth:`hashLibSerialize`

        Parameters
        ----------
        data: Union[:class:`str`, :class:`bytes`, Hashable]
            The data to hash

        Returns
        -------
        :class:`str`
            The resultant short, possibly-colliding hash, as a base64 string
        """

        return CppHashTools.getShortDeterministicHashStr(cls._toBytes(data))
##### EndScript
