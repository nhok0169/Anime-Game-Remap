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
import heapq
from typing import List, Union, Callable
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T
from .HeapNode import HeapNode
##### EndLocalImports


##### Script
class Algo():
    """
    Tools for some basic algorithms
    """

    @classmethod
    def merge(cls, sortedLsts: List[List[T]], compare: Callable[[T, T], int]) -> List[T]:
        """
        Merges k sorted lists toghether

        .. note::
            Implemented using the `standard heap solution`_ (See `k-way merge problem`_ for more details)

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

        minHeap = []
        heapCompare = lambda nodeData1, nodeData2: compare(nodeData1[0], nodeData2[0])

        numOfSortedLsts = len(sortedLsts)
        for i in range(numOfSortedLsts):
            lst = sortedLsts[i]
            lstLen = len(lst)

            if (lst):
                heapq.heappush(minHeap, HeapNode((lst[0], i, lstLen, 0), heapCompare))

        result = []
        while (minHeap):
            smallestData = heapq.heappop(minHeap).val
            result.append(smallestData[0])
            lstId, lstLen, lstInd = smallestData[1:]

            if (lstInd < lstLen - 1):
                lst = sortedLsts[lstId]
                lstInd += 1
                heapq.heappush(minHeap, HeapNode((lst[lstInd], lstId, lstLen, lstInd), heapCompare))

        return result

    @classmethod
    def _getMid(cls, left, right) -> int:
        return int(left + (right - left) / 2)

    @classmethod
    def binarySearch(cls, lst: List[T], target: T, compare: Callable[[T, T], int]) -> List[Union[int, bool]]:
        """
        Performs `binary search`_ to search for 'target' in 'lst'

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
    def binaryInsert(cls, lst: List[T], target: T, compare: Callable[[T, T], int], optionalInsert: bool = False) -> bool:
        """
        Insert's 'target' into 'lst' using `binary search`_

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

        found = False
        inserted = False

        found, insertInd = cls.binarySearch(lst, target, compare)
        if (not optionalInsert or not found):
            lst.insert(insertInd, target)
            inserted = True

        return inserted
##### EndScript
