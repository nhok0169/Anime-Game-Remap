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
from typing import List, Any, Tuple, Callable, Union, Set, Optional, Hashable, Dict
from collections import OrderedDict
##### EndExtImports

##### CppLocalImports
from ..core import CppListTools
##### EndCppLocalImports

##### CyLocalImports
from ..CyListTools import CyListTools
##### EndCyLocalImports

##### LocalImports
from ..constants.GenericTypes import T
##### EndLocalImports


##### Script
class ListTools():
    """
    Tools for handling with Lists
    """

    _CyTools = CyListTools()


    @classmethod
    def getDistinct(cls, lst: List[Any], keepOrder: bool = False) -> List[Any]:
        """
        Makes all the elements in the list unique

        Parameters
        ----------
        lst: List[Any]
            The list we are working with

        keepOrder: bool
            Whehter to keep the order of the elements in the list :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        List[Any]
            The new list with only unique values
        """

        if (keepOrder):
            return list(OrderedDict.fromkeys(lst))
        return list(set(lst))
    

    @classmethod
    def removeParts(cls, lst: List[T], partIndices: List[Tuple[int, int]]) -> List[T]:
        """
        Removes many sub-lists from a list

        .. note::
            This function is a convenience for calling :meth:`CppListTools.removeParts`

        Parameters
        ----------
        lst: List[T]
            The desired list to have its parts removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]:
            The indices relating to the parts to be removed from the lists :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain:

                #. The starting index of the part
                #. The ending index of the part (excluded from the actual list)s

        Returns
        -------
        List[T]
            The new list with its parts removed
        """

        return CppListTools.removeParts(lst, partIndices)
    
    @classmethod
    def removeByInds(cls, lst: List[T], inds: Set[int]) -> List[T]:
        """
        Removes many indices from a list

        .. note::
            This function is a convenience for calling :meth:`CppListTools.removeByInds`

        Parameters
        ----------
        lst: List[T]
            The desired list to have its parts removed

        inds: Set[:class:`int`]
            The indices to the elements in the list that needs to be removed :raw-html:`<br />` :raw-html:`<br />`

        Returns
        -------
        List[T]
            The new list with elements specified by indices removed
        """

        return CppListTools.removeByInds(lst, inds)
    
    @classmethod
    def addLstsByInds(cls, lst: List[T], subLsts: Dict[int, List[T]]) -> List[T]:
        """
        Inserts multiple sublists into the main list by index

        .. note::
            This function is a convenience for calling :meth:`CppListTools.addLstsByInds`

        Parameters
        ----------
        lst: List[T]
            The main list to work with

        subLsts: Dict[:class:`int`, List[T]]
            The sublists to insert into the main list :raw-html:`<br />` :raw-html:`<br />`

            The keys are the indices to insert the sublists and the values are the sublists

        Returns
        -------
        List[T]
            The resultant combined list
        """

        return CppListTools.addLstsByInds(lst, subLsts)
    
    @classmethod
    def interleave(cls, lst1: List[T], lst2: List[T]) -> List[T]:
        """
        Interleaves 2 lists toghether

        .. note::
            This function is a convenience for calling :meth:`CyListTools.interleave`

        Parameters
        ----------
        lst1: List[T]
            The first list to interleave. Items from this list will be used first in the alternating sequence

        lst2: List[T]
            The second list to interleave. Items from this list will be used second in the alternating sequence

        Returns
        -------
        List[T]
            A new list with the elements from both lists interleaved toghether
        """

        return cls._CyTools.interleave(lst1, lst2)
    
    @classmethod
    def splitLstByInds(cls, lst: List[T], indices: Union[List[int], Set[int]], sortIndices: bool = False) -> List[List[T]]:
        """
        Splits a list into sub-lists based on a list of indices.
        
        Parameters
        ----------
        lst: List[T]
            The list to split

        indices: Union[List[:class:`int`], Set[:class:`int`]]
            The indices of where to split the list

            .. warning::
                If the 'sortIndices' argument is set to ``False``, then will assume that this argument is sorted

        sortIndices: :class:`bool`
            Whether to sort the indices provided :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (sortIndices):
            indices = list(indices)
            indices.sort()

        full_indices = [0] + indices + [None]
        return [lst[start:end] for start, end in zip(full_indices[:-1], full_indices[1:])]

    @classmethod
    def getIndsAfterRemove(cls, removedInds: List[int], lstLen: int) -> List[int]:
        """
        Retrieve the index shifts in some data structure,
        after the list got elements removed by indices

        .. note::
            This function is a convenience for calling :meth:`CppListTools.getIndsAfterRemove`

        Parameters
        ----------
        removedInds: List[:class:`int`] 
            The indices to elements that got removed from the list :raw-html:`<br />` :raw-html:`<br />`

            Assume that the list in sorted order

        lstLen: :class:`int`
            The length of the original list, before its elements got removed

        Returns
        -------
        List[:class:`int`]
            A list containing how much each index is shifted
        """

        return CppListTools.getIndsAfterRemove(removedInds, lstLen)

    @classmethod
    def toDict(cls, lst: List[T], getId: Optional[Callable[[int, T], Hashable]] = None) -> Dict[Hashable, T]:
        """
        Converts a list into a dictionary

        Parameters
        ----------
        lst: List[T]
            The list to convert

        getId: Optional[Callable[[:class:`int`, T], `Hashable`_]]
            The function to generate the id for a particular list item.
            If this argument is ``None``, will use the index of the item as the id for the item :raw-html:`<br />` :raw-html:`<br />`

            The function takes in the index of the item and the value of the item as parameters :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[`Hashable`_, T]
            The converted dictionary
        """

        result = {}
        if (getId is None):
            getId = lambda ind, val: ind

        lstLen = len(lst)
        for i in range(lstLen):
            item = lst[i]
            id = getId(i, item)
            result[id] = item

        return result
##### EndScript