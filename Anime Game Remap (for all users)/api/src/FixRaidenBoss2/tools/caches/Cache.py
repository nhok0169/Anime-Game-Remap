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
from typing import Generic, Hashable, Optional, Any
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import T
##### EndLocalImports


##### Script
DefaultCacheSize = 128

class Cache(Generic[T]):
    """
    Class for a generic cache

    .. container:: operations

        **Supported Operations:**

        .. describe:: len(x)

            Retrieves the size of the :class:`Cache`, ``x``

        .. describe:: x[key]

            Retrieves the value from the :class:`Cache`, ``x``, from the key ``key``

        .. describe:: x[key] = newValue

            Sets the key ``key`` of the :class:`Cache`, ``x``, to have the value of ``newValue``

    :raw-html:`<br />`

    Parameters
    ----------
    capacity: :class:`int`
        The maximum capacity of the cache :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 128

    cacheStorage: Optional[Any]
        The type of `KVP`_ (Key-value pair) data structure to use for the cache. If this parameter is ``None``, then will use a dictionary :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    capacity: :class:`int`
        The maximum capacity of the cache

    cacheStorage: Any
        The type of `KVP`_ (Key-value pair) data structure to use for the cache.
    """

    def __init__(self, capacity: int = DefaultCacheSize, cacheStorage: Optional[Any] = None):
        self.capacity = capacity

        if (cacheStorage is None):
            self._cache = {}
        else:
            self._cache = cacheStorage

    def __getitem__(self, key: Hashable) -> Optional[T]:
        return self._cache[key]

    def __setitem__(self, key: Hashable, value: T) -> None:
        self._cache[key] = value

    def __len__(self) -> int:
        return len(self._cache)

    def clear(self) -> None:
        """
        Clears the cache
        """
        self._cache.clear()
##### EndScript