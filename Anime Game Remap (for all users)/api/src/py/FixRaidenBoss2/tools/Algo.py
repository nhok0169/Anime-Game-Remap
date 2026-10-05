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
from typing import List, Union, Callable
##### EndExtImports

##### CppLocalImports
from ..core import CppAlgo
##### EndCppLocalImports

##### CyLocalImports
from ..CyAlgo import CyAlgo
##### EndCyLocalImports

##### LocalImports
from ..constants.GenericTypes import T
##### EndLocalImports


##### Script
class Algo():
    """
    Tools for some basic algorithms
    """

    _CyTools = CyAlgo()

    @classmethod
    def merge(cls, sortedLsts: List[List[T]], compare: Callable[[T, T], int]) -> List[T]:
        """
        Merges k sorted lists toghether

        .. note::
            This function is a convenience for calling :meth:`CppAlgo.merge`

        Parameters
        ----------
        sortedLsts: List[List[T]]
            The sorted lists to merge

        compare: Callable[[T, T], :class:`int`]
            The `compare function`_ for comparing elements in the lists

        Returns
        -------
        List[T]
            A new list with all elements from the given lists merged toghether, preserving ordering
        """

        return CppAlgo.merge(sortedLsts, compare)

    @classmethod
    def binarySearch(cls, lst: List[T], target: T, compare: Callable[[T, T], int]) -> List[Union[int, bool]]:
        """
        Performs `binary search`_ to search for 'target' in 'lst'

        .. note::
            This function is a convenience for calling :meth:`CyAlgo.binarySearch`

        Parameters
        ----------
        lst: List[T]
            The sorted list we are searching from

        target: T
            The target element to search for in the list

        compare: Callable[[T, T], :class:`int`]
            The `compare function`_ for comparing elements in the list with the target element

        Returns
        -------
        [:class:`bool`, :class:`int`]
            * The first element is whether the target element is found in the list
            * The second element is the found index or the index that we expect the target element to be in the list
        """

        return cls._CyTools.binarySearch(lst, target, compare)
    
    @classmethod
    def binaryInsert(cls, lst: List[T], target: T, compare: Callable[[T, T], int], optionalInsert: bool = False) -> bool:
        """
        Insert's 'target' into 'lst' using `binary search`_

        .. note::
            This function is a convenience for calling :meth:`CyAlgo.binaryInsert`

        Parameters
        ----------
        lst: List[T]
            The sorted list we want to insert the target element

        target: T
            The target element to insert

        compare: Callable[[T, T], :class:`int`]
            The `compare function`_ for comparing elements in the list with the target element

        optionalInsert: :class:`bool`
            Whether to still insert the target element into the list if the element target element is found in the list :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`bool`
            Whether the target element has been inserted into the list
        """

        return cls._CyTools.binaryInsert(lst, target, compare, optionalInsert = optionalInsert)
##### EndScript
