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
from typing import List, Union, Callable
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T
##### EndLocalImports


##### Script
class Algo():
    """
    Tools for some basic algorithms
    """

    @classmethod
    def _getMid(cls, left, right) -> int:
        return int(left + (right - left) / 2)

    @classmethod
    def binarySearch(cls, lst: List[T], target: T, compare: Callable[[T, T], bool]) -> List[Union[int, bool]]:
        """
        Performs `binary search`_ to search for 'target' in 'lst'

        Parameters
        ----------
        lst: List[T]
            The sorted list we are searching from

        target: T
            The target element to search for in the list

        compare: Callable[[T, T], :class:`bool`]
            The `compare function`_ for comparing elements in the list with the target element

        Returns
        -------
        [:class:`int`, :class:`bool`]
            * The first element is whether the target element is found in the list
            * The second element is the found index or the index that we expect the target element to be in the list
        """

        left = 0
        right = len(lst) - 1
        mid = cls._getMid(left, right)

        while (left <= right):
            midItem = lst[mid]
            compResult = compare(midItem, target)

            if (compResult == 0):
                return [True, mid]
            elif (compResult > 0):
                right = mid - 1
            else:
                left = mid + 1

            mid = cls._getMid(left, right)

        return [False, left]
    
    @classmethod
    def binaryInsert(cls, lst: List[T], target: T, compare: Callable[[T, T], bool], optionalInsert: bool = False) -> bool:
        """
        Insert's 'target' into 'lst' using `binary search`_

        Parameters
        ----------
        lst: List[T]
            The sorted list we want to insert the target element

        target: T
            The target element to insert

        compare: Callable[[T, T], :class:`bool`]
            The `compare function`_ for comparing elements in the list with the target element

        optionalInsert: :class:`bool`
            Whether to still insert the target element into the list if the element target element is found in the list :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`bool`
            Whether the target element has been inserted into the list
        """

        found = False
        inserted = False

        found, insertInd = cls.binarySearch(lst, target, compare)
        if (not optionalInsert or not found):
            lst.insert(insertInd, target)
            inserted = True

        return inserted
##### EndScript
