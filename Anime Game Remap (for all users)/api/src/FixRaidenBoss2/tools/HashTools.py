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
from typing import Hashable, Callable, Optional, Dict
import hashlib
import json
##### EndExtImports

##### LocalImports
from .IntTools import IntTools
##### EndLocalImports


##### Script
class HashTools():
    """
    Tools for custom hashing
    """

    Base64SmallHashMap = {}
    Base64SmallDeterministicHashMap = {}
    Base64SmallHashMaxVal = 2 ** 16

    @classmethod
    def clear(cls):
        cls.Base64SmallHashMap.clear()
        cls.Base64SmallDeterministicHashMap.clear()

    @classmethod
    def base64Hash(cls, obj: Hashable, hashFunc: Optional[Callable[[Hashable], int]] =None) -> str:
        """
        Converts the hash to base 64

        Parameters
        ----------
        obj: Hashable
            The object to hash

        hashFunc: Optional[Callable[[Hashable], :class:`int`]]
            The base hash function to use. :raw-html:`<br />` :raw-html:`<br />`

            if this value is ``None``, then the hash function will be the `builtin hash`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The base 64 hash
        """

        if (hashFunc is None):
            hashFunc = hash

        hashVal = hashFunc(obj)
        return IntTools.toBase64(hashVal)

    @classmethod
    def _genericBase64ShortUniqueHash(cls, obj: Hashable, hashFunc: Callable[[Hashable], int], hashMap: Dict[str, Dict[Hashable, str]]) -> str:
        hashFuncWrapper = lambda objToHash: hashFunc(objToHash) % cls.Base64SmallHashMaxVal
        hashVal = cls.base64Hash(obj, hashFunc = hashFuncWrapper)

        if (hashVal not in hashMap):
            hashMap[hashVal] = {}

        hashVals = hashMap[hashVal]
        hashValsLen = len(hashVals)

        if (obj not in hashVals):
            hashInd = cls.base64Hash(hashValsLen, hashFunc = hashFuncWrapper)
            hashVals[obj] = hashInd
            hashValsLen += 1
        else:
            hashInd = hashVals[obj]

        # collisions
        if (hashValsLen > 1):
            return f"{hashVal},{hashInd}"
        return f"{hashVal}"
    
    @classmethod
    def base64ShortUniqueHash(cls, obj: Hashable) -> str:
        """
        Converts the hash from `builtin hash`_ function to a unique and unique and short base 64 hash

        Parameters
        ----------
        obj: Hashable
            The object to hash

        Returns
        -------
        :class:`str`
            The unique base 64 hash
        """

        return cls._genericBase64ShortUniqueHash(obj, hash, cls.Base64SmallHashMap)
    
    @classmethod
    def _hashLibSerialize(cls, obj: Hashable):
        if isinstance(obj, (int, float, str, bool, type(None))):
            return obj
        if isinstance(obj, (list, tuple)):
            return [cls._hashLibSerialize(item) for item in obj]
        if isinstance(obj, dict):
            return {k: cls._hashLibSerialize(v) for k, v in sorted(obj.items())}
        
        # Handle custom objects
        return {k: cls._hashLibSerialize(v) for k, v in sorted(obj.__dict__.items()) if not k.startswith('_')}
    
    @classmethod
    def hashLibSerialize(cls, obj: Hashable) -> bytes:
        """
        Convert some hashable into bytes used for the `hashlib` library

        Parameters
        ----------
        obj: Hashable
            The object to convert

        Returns
        -------
        :class:`bytes`
            The resultant bytes converted from the object
        """

        serializedData = cls._hashLibSerialize(obj)
        return json.dumps(serializedData, sort_keys=True).encode('utf-8')
    
    @classmethod
    def _base64DeterministicShortUniqueHashFunc(cls, obj: Hashable):
        if (not isinstance(obj, bytes)):
            obj = cls.hashLibSerialize(obj)

        md5Hash = hashlib.md5(obj)
        digestBytes = md5Hash.digest()
        result = int.from_bytes(digestBytes, byteorder='big')
        return result

    @classmethod
    def base64DeterministicShortUniqueHash(cls, obj: Hashable) -> str:
        """
        Converts the hash from a naive hash function that acts as incrementor to a unique and short base 64 hash

        Parameters
        ----------
        obj: Hashable
            The object to hash

        Returns
        -------
        :class:`str`
            The unique base 64 hash
        """

        return cls._genericBase64ShortUniqueHash(obj, cls._base64DeterministicShortUniqueHashFunc, cls.Base64SmallDeterministicHashMap)
##### EndScript