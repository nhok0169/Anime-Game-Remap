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
from typing import Callable
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T
##### EndLocalImports


##### Script
class HeapNode():
    """
    Class for a node in a `heap`_

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x < y

            Whether the value in the node x is smaller than the value in the node y

        .. describe:: x <= y
            Whether the value in the node x is smaller or equal to the value in the node y

        .. describe:: x > y

            Whether the value in the node x is bigger than the value in the node y

        .. describe:: x >= y

            Whether the value in the node x is bigger or equal to the value in the node y

        .. describe:: x == y

            Whether the value in the node x is equal to the value in node y

        .. describe:: x != y

            Whether the value in the node x is not equal to the value in node y

    Parameters
    ----------
    val: T
        The value to be stored in the node

    compare: Callable[[T, T], :class:`int`]
        The `compare function`_ for comparing elements in the heap

    Attributes
    ----------
    val: T
        The value within the node

    compare: Callable[[T, T], :class:`int`]
        The `compare function`_ for comparing elements in the heap
    """

    def __init__(self, val: T, compare: Callable[[T, T], int]):
        self.val = val
        self.compare = compare

    def __lt__(self, other: "HeapNode"):
        return self.compare(self.val, other.val) < 0
    
    def __le__(self, other: "HeapNode"):
        return self.compare(self.val, other.val) <= 0
    
    def __gt__(self, other: "HeapNode"):
        return self.compare(self.val, other.val) > 0
    
    def __ge__(self, other: "HeapNode"):
        return self.compare(self.val, other.val) >= 0
    
    def __eq__(self, other: "HeapNode"):
        return self.compare(self.val, other.val) == 0
    
    def __ne__(self, other: "HeapNode"):
        return self.compare(self.val, other.val) != 0
##### EndScript