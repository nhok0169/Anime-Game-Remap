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
from collections import OrderedDict
from typing import Hashable, Optional
##### EndExtImports

##### LocalImports
from .Cache import Cache, DefaultCacheSize
from ...constants.GenericTypes import T
##### EndLocalImports


##### Script
class LruCache(Cache):
    """
    This class inherits from :class:`Cache`

    Class for an `LRU cache`_

    .. container:: operations

        **Supported Operations:**

        .. describe:: len(x)

            Retrieves the size of the :class:`LruCache`, ``x``

        .. describe:: x[key]

            Retrieves the value from the :class:`LruCache`, ``x``, from the key ``key``

        .. describe:: x[key] = newValue

            Sets the key ``key`` of the :class:`LruCache`, ``x``, to have the value of ``newValue``

    :raw-html:`<br />`

    Parameters
    ----------
    capacity: :class:`int`
        The maximum capacity of the cache :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 128
    """

    def __init__(self, capacity: int = DefaultCacheSize):
        super().__init__(capacity, OrderedDict())

    def __getitem__(self, key: Hashable) -> Optional[T]:
        if key not in self._cache:
            raise KeyError(f"{key}")

        self._cache.move_to_end(key)
        return self._cache[key]

    def __setitem__(self, key: Hashable, value: T) -> None:
        if len(self._cache) == self.capacity:
            self._cache.popitem(last=False)

        self._cache[key] = value
        self._cache.move_to_end(key)
##### EndScript
