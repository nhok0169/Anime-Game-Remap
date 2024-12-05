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
from typing import List, Any, Tuple, Callable, Union, Set
from collections import OrderedDict
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import T, N
##### EndLocalImports


##### Script
class ListTools():
    """
    Tools for handling with Lists
    """

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
    def removeParts(cls, lst: List[T], partIndices: List[Tuple[int, int]], nullifyRemoval: Callable[[], N], isNull: Callable[[Union[T, N]], bool]) -> List[T]:
        """
        Removes many sub-lists from a list

        Parameters
        ----------
        lst: List[T]
            The desired list to have its parts removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]:
            The indices relating to the parts to be removed from the lists :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain:

                #. The starting index of the part
                #. The ending index of the part (excluded from the actual list)

        nullifyRemoval: Callable[[], N]:
            Function for creating a null element used to replace the removed part

        isNull: Callable[[Union[T, N]], :class:`bool`]
            Function for identifying whether an element in the list is the null element

        Returns
        -------
        List[T]
            The new list with its parts removed
        """

        null = nullifyRemoval()
        for indices in partIndices:
            startInd = indices[0]
            endInd = indices[1]
            lst[startInd:endInd] =  [null] * (endInd - startInd)

        lst = list(filter(lambda element: not isNull(element), lst))
        return lst
    
    @classmethod
    def removeByInds(cls, lst: List[T], inds: Set[int]) -> List[T]:
        """
        Removes many indices from a list

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

        return [element for ind, element in enumerate(lst) if ind not in inds]
##### EndScript