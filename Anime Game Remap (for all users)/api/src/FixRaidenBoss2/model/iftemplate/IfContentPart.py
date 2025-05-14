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
from collections import defaultdict
from typing import List, Dict, Tuple, Union, Set, Union, Callable, Any, Optional
##### EndExtImports

##### LocalImports
from ...tools.Algo import Algo
from ...tools.DictTools import DictTools
from ...tools.ListTools import ListTools
from ...tools.Algo import Algo
from .IfTemplatePart import IfTemplatePart
##### EndLocalImports


##### Script
class IfContentPart(IfTemplatePart):
    """
    This class inherits from :class:`IfTemplatePart`

    Class for defining the content part of an :class:`IfTemplate`

    .. note::
        see :class:`IfTemplate` for more details

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: key in x

            Determines if 'key' exists in the content part of the :class:`IfContentPart`

        .. describe:: x[key]

            Retrieves the corresponding data value from the :class:`IfContentPart` based off 'key' :raw-html:`<br />` :raw-html:`<br />`

            * If 'key' is an :class:`int`, then will retrieve a tuple containing:

                #. The corresponding key for the `KVP`_ found
                #. The corresponding value to the found `KVP`_
                #. The occurence index for the key of the `KVP`_

            * Otherwise, will retrieve the corresponding value from :meth:`IfContentPart.src`

        .. describe:: for key, val, keyInd, orderInd in x

            Iterates over all the key/value initializations and updates within the :class:`IfContentPart`, ``x`` :raw-html:`<br />` :raw-html:`<br />`

            The tuples to iterate over are as follows:

            #. key: (:class:`str`) A particular key in the :class:`IfContentPart`
            #. val: (:class:`str`) The corresponding value to the key
            #. keyInd: (:class:`int`) The occurence index of the same key within the :class:`IfContentPart`
            #. orderInd: (:class:``int) The order index the `KVP`_ appears in the overall :class:`IfContentPart`

    Parameters
    ----------
    src: Dict[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]
        The source for the part in the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the name of the keys in the part
        * The values are the coresponding values for the keys for all instances where the particular key got instantiated/updated. Each element in the list contains:

            #. The order index the `KVP`_ was called within the part
            #. The value of the `KVP`_

    depth: :class:`int`
        The depth the part is within the :class:`IfTemplate`

    Attributes
    ----------
    src: Dict[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]
        The source for the part in the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the name of the keys in the part
        * The values are the coresponding values for the keys for all instances where the particular key got instantiated/updated. Each element in the list contains:
            #. The order index the `KVP`_ was called within the part
            #. The value of the `KVP`_

    depth: :class:`int`
        The depth the part is within the :class:`IfTemplate`

    _order: List[Tuple[:class:`str`, :class:`int`]]
        The order the `KVP`_s appear in the part. The elements contain:
            #. The name of the key for the `KVP`_
            #. The occurence index of the key within the part
    """

    def __init__(self, src: Dict[str, List[Tuple[int, str]]], depth: int):
        self._order: List[Tuple[str, int]] = []
        self.src = src
        self.depth = depth

    def __iter__(self):
        for key, keyInd in self._order:
            valTuple = self.src[key][keyInd]
            orderInd = valTuple[0]
            val = valTuple[1]
            result = (key, val, keyInd, orderInd)
            yield result

    def __contains__(self, key: str):
        return key in self.src

    def __getitem__(self, key: Union[str, int]) -> Union[List[Tuple[int, str]], str]:
        if (isinstance(key, int)):
            kvpRef = self._order[key]
            val = self.src[kvpRef[0]][kvpRef[1]]
            return (kvpRef[0], val, kvpRef[1])

        return self.src[key]
    
    def get(self, key: Union[str, int], default: Optional[Any] = None) -> Union[List[Tuple[int, str]], str, Any]:
        """
        Retrieves the corresponding data value from the :class:`IfContentPart` based off 'key' :raw-html:`<br />` :raw-html:`<br />`

            * If 'key' is an :class:`int`, then will retrieve a tuple containing:

                #. The corresponding key for the `KVP`_ found
                #. The corresponding value to the found `KVP`_
                #. The occurence index for the key of the `KVP`_

            * Otherwise, will retrieve the corresponding value from :meth:`IfContentPart.src` :raw-html:`<br />` :raw-html:`<br />`

        If the 'key' is not found, then will return the value from 'default'

        .. note::
            This is the same as the `getitem operator`_ specified for this class, but will return a default value
            if the key is not found

        Paramters
        ---------
        key: Union[:class:`str`, :class:`int`]
            The key to search for in this class

        default: Optional[Any]
            The default value to return if the key is not found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        Union[List[Tuple[:class:`int`, :class:`str`]], :class:`str`, Any]
            Either the found value or the default value
        """

        try:
            return self.__getitem__(key)
        except KeyError:
            return default

    @property
    def src(self):
        """
        The raw content of the part :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the names of the keys in the content part of the :class:`IfTemplate`. Note that the same key can appear multiple times in a particular content part.
        * The values consists of:
            #. The order index the `KVP`_ appeared in the :class:`IfContentPart`
            #. The corresponding value for the key

        :getter: Retrieves the raw content of the part
        :setter: Sets the raw content for the part
        :type: Dict[:class:`str`, List[:class:`int`, :class:`str`]]
        """

        return self._src
    
    @src.setter
    def src(self, newSrc: Dict[str, List[Tuple[int, str]]]):
        self._src = newSrc
        self._setupOrder()

    def _setupOrder(self):
        self._order = []
        for key in self.src:
            values = self.src[key]
            valuesLen = len(values)
            for i in range(valuesLen):
                orderInd, _ = values[i]
                keyRef = (key, i, orderInd)
                Algo.binaryInsert(self._order, keyRef, lambda keyRef1, keyRef2: keyRef1[2] - keyRef2[2])

        self._order = list(map(lambda orderData: orderData[:-1], self._order))

    def __contains__(self, key):
        return key in self.src

    def toStr(self, linePrefix: str = "") -> str:
        """
        Retrieves the part as a string

        Parameters
        ----------
        linePrefix: :class:`str`
            The string that will prefix every line :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The string representation of the part        
        """

        result = ""
        orderLen = len(self._order)
        i = 0
        for key, val, keyInd, orderInd in self:
            result += f"{linePrefix}{key} = {val}"
            if (i < orderLen - 1):
                result += "\n"
            i += 1

        return result
    
    def getVals(self, key: str) -> List[str]:
        """
        Retrieves the corresponding values based off 'key'
        
        Parameters
        ----------
        key: :class:`str`
            The key to the values belong to

        Returns
        -------
        List[:class:`str`]
            The corresponding values found for the key
        """

        result = []

        values = None
        try:
            values = self._src[key]
        except KeyError:
            return result
        
        result = list(map(lambda valData: valData[1], values))
        return result
    
    def removeKey(self, key: Union[str, Tuple[str, Callable[[Tuple[int, str]], bool]]]):
        """
        Removes a key from the part.

        Parameters
        ----------
        key: :class:`str`
            The key to remove. :raw-html:`<br />` :raw-html:`<br />`

            * If given only a string, will delete all instances of the key.
            * If given a tuple containing a string and a predicate, will delete all the keys that satisfy the predicate.
              The predicate takes in a tuple that contains:

                #. The order index where the corresponding `KVP`_ appeared
                #. The corresponding value for the `KVP`_
        """

        orderIndsToRemove = set()
        values = None
        pred = lambda val: True
        targetKey = key

        if (isinstance(key, tuple) and len(key) >= 2):
            pred = key[1]
            targetKey = key[0]

        try:
            values = self.src[targetKey]
        except KeyError:
            return
        
        currentRemoved = set()
        for value in values:
            if (pred(value)):
                orderIndsToRemove.add(value[0])
                currentRemoved.add(value)

        if (len(currentRemoved) == len(values)):
            del self.src[targetKey]

        self._order = ListTools.removeByInds(self._order, orderIndsToRemove)

        # update the order indices
        orderLen = len(self._order)
        for i in range(orderLen):
            orderData = self._order[i]
            self.src[orderData[0]][orderData[1]][0] = i

    def removeKeys(self, keys: Set[Union[str, Tuple[str, Callable[[Tuple[int, str]], bool]]]]):
        """
        Removes multiple keys from the part

        Parameters
        ----------
        keys: Set[Union[:class:`str`, Callable[[Tuple[:class:`int`, :class:`str`]], :class:`bool`]]]
            The keys to remove. :raw-html:`<br />` :raw-html:`<br />`

            * If given only a string, will delete all instances of the key.
            * If given a tuple containing a string and a predicate, will delete all the keys that satisfy the predicate.
              The predicate takes in a tuple that contains:

              #. The order index where the corresponding `KVP`_ appeared
              #. The corresponding value for the `KVP`_
        """

        orderIndsToRemove = set()

        for key in keys:
            pred = lambda val: True
            targetKey = key

            if (isinstance(key, tuple) and len(key) >= 2):
                pred = key[1]
                targetKey = key[0]

            values = None
            try:
                values = self.src[targetKey]
            except KeyError:
                continue
            
            currentRemoved = set()
            for value in values:
                if (pred(value)):
                    orderIndsToRemove.add(value[0])
                    currentRemoved.add(value)
            
            if (len(currentRemoved) == len(values)):
                del self.src[targetKey]

        if (not orderIndsToRemove):
            return
        
        self._order = ListTools.removeByInds(self._order, orderIndsToRemove)

        # update the order indices
        orderLen = len(self._order)
        for i in range(orderLen):
            orderData = self._order[i]
            valData = self.src[orderData[0]][orderData[1]]
            self.src[orderData[0]][orderData[1]] = (i, valData[1])

    def addKVPToFront(self, key: str, value: str):
        """
        Adds a new `KVP`_ into the part
        
        .. warning::
            This operation will take `O(n)` time, where `n` is the # of `KVP`_s within the part

        Parameters
        ----------
        key: :class:`str`
            The name of the key

        value: :class:`str`
            The corresponding value to the key
        """

        try:
            self.src[key]
        except KeyError:
            self.src[key] = []

        valData = (-1, value)
        self.src[key].insert(0, valData)
        self._order.insert(0, (key, -1))

        # update the indices of the other KVPs
        for keyName in self.src:
            kvps = self.src[keyName]
            kvpsLen = len(kvps)

            for i in range(kvpsLen):
                valData = kvps[i]
                kvps[i] = (valData[0] + 1, valData[1])

            if (keyName != key):
                continue
            
            kvpsLen = len(kvps)
            for i in range(kvpsLen):
                valData = kvps[i]
                orderInd = valData[0]
                self._order[orderInd] = (key, i)
        

    def addKVP(self, key: str, value: str, toFront: bool = False):
        """
        Adds a new `KVP`_ into the part

        Parameters
        ----------
        key: :class:`str`
            The name of the key

        value: :class:`str`
            The corresponding value to the key

        toFront: :class:`bool`
            Whether to add the new `KVP`_ to the front of the part

            .. warning::
                Please see the warning at :meth:`addKVPToFront`
        """

        if (toFront):
            self.addKVPToFront(key, value)
            return

        try:
            self.src[key]
        except KeyError:
            self.src[key] = []
        
        valData = (len(self._order), value)
        self.src[key].append(valData)
        self._order.append((key, len(self.src[key]) - 1))

    def replaceVals(self, newVals: Dict[str, Union[str, List[str], Tuple[str, Callable[[str], bool]]]], addNewKVPs: bool = True):
        """
        Replaces the values in the `KVP`_s of the parts or adds in new `KVP`_s if the original key did not exist

        Parameters
        ----------
        newVals: Dict[:class:`str`, Union[:class:`str`, List[:class:`str`], Tuple[:class:`str`, Callable[[:class:`str`], :class:`bool`]]]]
            The new values for the `KVP`_s in the parts :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the corresponding keys for the `KVP`_s
            * The values can either contain:
                
                * A string, which represents the new value for all instances of the key OR
                * A list of strings, representing the individual new values for each instance of the key OR
                * A tuple containing a string and a predicate, representing the new value for certain instances of the key that satisfy the predicate.
                  The predicate takes in the old value of the `KVP`_ as an argument

        addNewKVPs: :class:`bool`
            Whether to add new KVPs if the corresponding key in 'newVals' does not exist :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        for key in newVals:
            vals = newVals[key]

            valsIsStr = isinstance(vals, str)
            valsIsCond = isinstance(vals, tuple) and len(vals) >= 2

            currentVals = None
            try:
                currentVals = self.src[key]
            except KeyError:
                if (not addNewKVPs):
                    continue

                if (valsIsStr):
                    self.addKVP(key, vals)
                elif (valsIsCond):
                    self.addKVP(key, vals[0])
                else:
                    for val in vals:
                        self.addKVP(key, val)

                continue

            if (valsIsStr):
                self.src[key] = list(map(lambda valData: (valData[0], vals), currentVals))
                continue

            elif (valsIsCond):
                currentValsLen = len(currentVals)
                pred = vals[1]

                for i in range(currentValsLen):
                    valData = currentVals[i]
                    if (pred(valData[1])):
                        self.src[key][i] = (valData[0], vals[0])

                continue

            smallerValLen = min(len(currentVals), len(vals))
            for i in range(smallerValLen):
                self.src[key][i][1] = vals[i]

    def remapKeys(self, keyRemap: Dict[str, List[str]]):
        """
        Remaps the keys in the `KVP`_s of the parts

        Parameters
        ----------
        keyRemap: Dict[:class:`str`, List[:class:`str`]]
            The remap for the keys :raw-html:`<br />` :raw-html:`<br />`

            The keys are the old names of the keys to be remapped and the values are the new names of the keys to be remapped to

            .. warning::
                Recommeded that the new names in each list to be unique. Otherwise, this function will make each list to have unique values.
        """

        occurences = defaultdict(lambda: 0)
        i = 0
        orderLen = len(self._order)
        keysToRemove = set()
        keysToAdd = set()
        remappedSrc = defaultdict(lambda: [])

        while (i < orderLen):
            keyData = self._order[i]
            key = keyData[0]
            keyOccurence = keyData[1]
            currentMaxOccurence = occurences[key]
            
            # update the occurence of the key
            if (keyOccurence < currentMaxOccurence):
                self._order[i] = (key, currentMaxOccurence)

            if (key not in keyRemap):
                i += 1
                occurences[key] += 1
                continue
            else:
                keysToRemove.add(key)

            newKeys = keyRemap[key]
            newKeysLen = len(newKeys)

            keyValData = self.src[key][keyOccurence]
            keyVal = keyValData[1]
            newKeyRefs = []

            # construct the remapped keys
            for j in range(newKeysLen):
                newKey = newKeys[j]

                newKeyRefs.append((newKey, occurences[newKey]))
                remappedSrc[newKey].append((i + j, keyVal))

                keysToAdd.add(newKey)
                occurences[newKey] += 1

            self._order = self._order[:i] + newKeyRefs + self._order[i + 1:]

            newRefsLen = len(newKeyRefs)
            i += newRefsLen
            orderLen += (newRefsLen - 1)
            
        # remove the keys that do not appear after the remap
        for key in keysToRemove:
            if (key not in keysToAdd):
                del self.src[key]

        # construct the new src
        srcValCompare = lambda srcVal, remappedVal: srcVal[0] - remappedVal[0]
        DictTools.update(self.src, remappedSrc, lambda key, srcVals, remappedVals: remappedVals if (key in keysToRemove) else Algo.merge([srcVals, remappedVals], srcValCompare))
##### EndScript