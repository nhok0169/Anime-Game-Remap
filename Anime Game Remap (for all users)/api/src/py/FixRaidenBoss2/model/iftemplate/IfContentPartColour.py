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
import copy
from collections import UserDict
from typing import Optional, Union, List, Tuple, Dict, Set, Callable
##### EndExtImports

##### CppLocalImports
from ...core import Ranges
##### EndCppLocalImports

##### LocalImports
from ...tools.DictTools import DictTools
from .IfContentPart import IfContentPart
##### EndLocalImports


##### Script
class IfContentPartColourChange():
    """
    Class to store the change in states of a particular key for a :class:`IfContentPart`

    Parameters
    ----------
    old: Optional[Union[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]]
        The old value of a particular key

    Attributes
    ----------
    old: Optional[Union[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]]
        The old value of a particular key
    """

    def __init__(self, old: Optional[Union[str, List[Tuple[int, str]]]] = None):
        self.old = old

    def restore(self, colouring: "IfContentPartColouring", key: str):
        """
        Restores the old value for a particular key
        """

        if (key not in colouring):
            return
        
        if (self.old is None):
            del colouring[key]
            return
        
        colouring[key] = self.old


class IfContentPartColouring(UserDict):
    """
    Class that keeps track of the current state of the `KVPs`_ within an :class:`IfContentPart`

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: hash(x)

            Retrieves the hash for the colouring

    :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the names of the register keys
    * The values are either:

        * A string value, indicating the value of the `KVP`_ comes from some previous :class:`IfContentPart` OR
        * A list of type ``List[Tuple[int, str]]``. The list indicates that the values of the corresponding key comes from the current :class:`IfContentPart`
          Each tuple contains the new state value for the corresponding key and its index of that `KVP`_ within the current part 
    """

    def __hash__(self):
        return id(self)

    def updateColouring(self, ifContentPart: IfContentPart, targetKeys: Optional[Set[str]] = None, updatePreviousKVPs: bool = True) -> Dict[str, "IfContentPartColourChange"]:
        """
        Updates the current state of the `KVPs`_ based on the current :class:`IfContentPart`

        Parameters
        ----------
        ifContentPart: :class:`IfContentPart`
            The part to update the new `KVPs`_

        targetKeys: Optional[Set[:class:`str`]]
            The target keys to keep track of :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will keep track of all the keys :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        updatePreviousKVPs: :class:`bool`
            Whether to also update the `KVP`_ values from previous :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        Dict[:class:`str`, :class:`IniContentPartColourChange`]
            The change in the state :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the keys and the values are the state change for the keys
        """

        change = {}
        keysChanged = set()

        # update the values from previous parts
        if (updatePreviousKVPs):
            for key in self:
                val = self[key]
                if (not isinstance(val, list)):
                    continue

                change[key] = IfContentPartColourChange(old = val)
                self[key] = val[-1][1]
                keysChanged.add(key)

        ifContentSrc = ifContentPart.src
        ifContentSrcLen = len(ifContentSrc)
        targetKeysLen = ifContentSrcLen if (targetKeys is None) else len(targetKeys)
        shortIterKeys = targetKeys if (targetKeysLen < ifContentSrcLen) else ifContentSrc

        # update the values for the current part
        for key in shortIterKeys:
            if (key not in ifContentSrc):
                continue

            if (targetKeys is not None and key not in targetKeys):
                continue

            newVal = ifContentSrc[key]
            if (not newVal):
                continue
            
            if (key not in keysChanged):
                oldVal = self.get(key)
                change[key] = IfContentPartColourChange(old = oldVal)

            self[key] = newVal

        return change
    
    def restore(self, colourChange: Dict[str, "IfContentPartColourChange"]):
        """
        Restores to a previous state

        Parameters
        ----------
        colourChange: Dict[:class:`str`, :class:`IniContentPartColourChange`]
            The change in the state :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the keys and the values are the state change for the keys
        """

        for key in colourChange:
            colourChange[key].restore(self, key)

    def getIndVals(self, key: str, filter: Optional[Callable[[Optional[int], str], bool]] = None) -> List[Tuple[Optional[int], str]]:
        """
        Retrieves both the corresponding values and the index of where the value occurs

        Parameters
        ----------
        key: :class:`str`
            The key to search for

        Returns
        -------
        List[Tuple[Optional[:class:`str`], :class:`str`]]
            Both the values and their index within the current :class:`IfContentPart`. If an index is ``None``, then the value appeared before the current part. 

        filter: Optional[Callable[[Optional[:class:`int`], :class:`str`], :class:`bool`]]
            A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`

            The predicate takes in the following parameters:

            #. The index the value appears in the current :class:`IfContentPart`. If this argument is ``None``, then the part was specified for the current :class:`IfContentPart`
            #. The corresponding value

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        result = self.get(key, [])
        if (not isinstance(result, list)):
            return [(None, result)]
        
        if (filter is None):
            return result

        return [data for data in result if (filter(data[0], data[1]))]

    def getVals(self, key: str, unique: bool = True, filter: Optional[Callable[[Optional[int], str], bool]] = None) -> Union[List[str], Set[str]]:
        """
        Retrieves the values for a given key

        Parameters
        ----------
        key: :class:`str`
            The key to search for

        unique: :class:`bool`
            Whether the values returned should be unique :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        filter: Optional[Callable[[Optional[:class:`int`], :class:`str`], :class:`bool`]]
            A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`

            The predicate takes in the following parameters:

            #. The index the value appears in the current :class:`IfContentPart`. If this argument is ``None``, then the part was specified for the current :class:`IfContentPart`
            #. The corresponding value

            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Union[List[:class:`str`], Set[:class:`str`]]
            The resultant values
        """

        vals = self.get(key, [])
        if (not isinstance(vals, list)):
            vals = [[None, vals]]

        if (filter is None):
            filter = lambda ind, val: True

        if (unique):
            result = set()
            for ind, val in vals:
                if (filter(ind, val)):
                    result.add(val)

        else:
            result = []
            for ind, val in vals:
                if (filter(ind, val)):
                    result.append(val)

        return result
    
    def getRanges(self, keysExists: Optional[Dict[str, bool]] = None, keyFilters: Optional[Dict[str, Callable[[Optional[int], str], bool]]] = None, 
                  existsRequireAll: bool = True, filtersRequireAll: bool = True, globalRequireAll: bool = True, includeKeyDefs: bool = True) -> Ranges:
        """
        Retrieves the ranges of indices within the current part that satisfy specified conditions for each key

        Parameters
        ----------
        keysExists: Optional[Dict[:class:`str`, :class:`bool`]]
            Checks whether a key exists or does not exist :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the registers and the values are whether to check for the existance/non-existance of the register :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        keyFilters: Optional[Dict[:class:`str`, Callable[[Optional[:class:`int`], :class:`str`], :class:`bool`]]]
            The conditions to satisfy for each key :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the registers and the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

            The predicates take the following parameters:

            #. The index the value appears in the current :class:`IfContentPart`. If this argument is ``None``, then the part was specified for the current :class:`IfContentPart`
            #. The corresponding value :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        existsRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy all existance checks at 'keyExists' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        filtersRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy all the predicates specified at 'keyFilters' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        globalRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy checks in both 'keyExists' and 'keyFilters' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        includeKeyDefs: :class:`bool`
            Whether to include indices where the values for the keys specified at 'keyExists' or 'keyFilters' are being (re)defined :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`Ranges`
            The valid ranges that satisfiy the specified conditions
        """

        result = Ranges.createFull()
        filterRanges = {}
        existRanges = {}
        keyDefInds = set()
        
        if (keyFilters is None):
            keyFilters = {}

        if (keysExists is None):
            keysExists = {}

        for reg in keyFilters:
            regVals = self.get(reg)
            if (regVals is None):
                continue

            keyFilter = keyFilters[reg]

            if (not isinstance(regVals, list)):
                filterRanges[reg] = Ranges.createFull() if (keyFilter(None, regVals)) else Ranges.createEmpty()
                continue

            currentRanges = []
            regValsLen = len(regVals)

            for i in range(regValsLen):
                ind, val = regVals[i]
                if (not includeKeyDefs):
                    keyDefInds.add(ind)

                if (not keyFilter(ind, val)):
                    continue

                endInd, _ = regVals[i + 1] if (i < regValsLen - 1) else (None, None)
                currentRanges.append([ind, endInd])

            currentRanges = Ranges(currentRanges)
            filterRanges[reg] = currentRanges

        for reg in keysExists:
            regVals = self.get(reg)
            keyExists = keysExists[reg]

            if (regVals is None):
                existRanges[reg] = Ranges.createEmpty() if (keyExists) else Ranges.createFull()
                continue

            if (not isinstance(regVals, list)):
                existRanges[reg] = Ranges.createFull() if (keyExists) else Ranges.createEmpty()
                continue

            currentExistRanges = [] if (regValsLen == 0) else [[regVals[0][0], None]]
            currentExistRanges = Ranges(currentExistRanges)
            existRanges[reg] = currentExistRanges if (keyExists) else ~currentExistRanges

            if (not includeKeyDefs):
                for ind, val in regVals:
                    keyDefInds.add(ind)

        existResult = result.getOverlaps(list(existRanges.values()), requireAll = existsRequireAll)
        filterResult = result.getOverlaps(list(filterRanges.values()), requireAll = filtersRequireAll)
        result = existResult & filterResult if (globalRequireAll) else existResult + filterResult

        if (includeKeyDefs):
            return result

        keyDefRanges = Ranges.createFromSet(keyDefInds)
        return result - keyDefRanges
##### EndScript