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
import itertools as IT
from collections import defaultdict
from typing import List, Dict, Tuple, Union, Set, Union, Callable, Any, Optional
##### EndExtImports

##### LocalImports
from ...tools.Algo import Algo
from ...tools.ListTools import ListTools
from ...tools.Algo import Algo
from .IfTemplatePart import IfTemplatePart
##### EndLocalImports


##### Script
class RemappedKeyData():
    """
    Class to store data about a remapped register within a .ini `section`_

    Parameters
    ----------
    key: :class:`str`
        The new register name to remap the old register to

    check: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
        Predicate to check whether to remap to the new register :raw-html:`<br />` :raw-html:`<br />`

        The predicate takes in:
         
        #. the old register name 
        #. the old register value

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    toInd: Optional[:class:`int`]
        Whether to shift the remapped register to a particular index within the :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    key: :class:`str`
        The new register name to remap the old register to

    check: Callable[[:class:`str`, :class:`str`], :class:`bool`]
        Predicate to check whether to remap to the new register

    toInd: Optional[:class:`int`]
        Whether to shift all the remapped register to a particular index within the :class:`IfContentPart`
    """

    def __init__(self, key: str, check: Optional[Callable[[str, str], bool]] = None, toInd: Optional[int] = None):
        self.key = key
        self.check = check
        self.toInd = toInd

    @classmethod
    def build(cls, data: Union[str, Tuple[str, Callable[[str, str], bool]], "RemappedKeyData"]) -> "RemappedKeyData":
        """
        Builds the object based off the raw 'data' provided

        Parameters
        ----------
        data: Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`, :class:`str`], :class:`bool`]]]
            The data to provide into the :class:`RemappedKeyData` class :raw-html:`<br />` :raw-html:`<br />`

            The provided data either contains:
            
            * The new name of the key to remap to OR
            * A tuple containing a new name for the key to remap to and a predicate that takes in the old key and old value of whether to remap the key. OR
            * The object that contains all the necessary information for remapping to the new key

        Returns
        -------
        :class:`RemappedKeyData`
            The constructed object
        """

        if (isinstance(data, cls)):
            return data
        elif (isinstance(data, str)):
            return cls(data)
        return cls(data[0], check = data[1])
    

class KeyRemapData():
    """
    Class to store data about a remapping a particular register

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x[ind]

            Retrieves the corresponding :class:`RemappedKeyData` based off the index, 'ind'

        .. describe:: len(x)

            Retrieves the number of keys to remap to

        .. describe:: for remapped key in x

            Iterates through all the data of the keys to remap to

    Parameters
    ----------
    remappedKeys: List[:class:`RemappedKeyData`]
        The new registers to remap the old register to

    keepKeyWithoutRemap: :class:`bool`
        Whether retain the old register, if the old register does not get remapped :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    remappedKeys: List[:class:`RemappedKeyData`]
        The new registers to remap the old register to

    keepKeyWithoutRemap: :class:`bool`
        Whether retain the old register, if the old register does not get remapped
    """

    def __init__(self, remappedKeys: List[RemappedKeyData], keepKeyWithoutRemap: bool = False):
        self.keepKeyWithoutRemap = keepKeyWithoutRemap
        self.remappedKeys = remappedKeys

    def __getitem__(self, key: int) -> RemappedKeyData:
        return self.remappedKeys[key]
    
    def __len__(self) -> int:
        return len(self.remappedKeys)
    
    def __iter__(self):
        for remappedKey in self.remappedKeys:
            yield remappedKey

    @classmethod
    def build(cls, remappedKeys: Union["KeyRemapData", List[Union[str, Tuple[str, Callable[[str, str], bool]], RemappedKeyData]]], keepKeyWithoutRemap: bool = False) -> "KeyRemapData":
        """
        Build the object based off the raw 'remappedKeys' provided

        Parameters
        ----------
        remappedKeys: Union[:class:`KeyRemapData`, List[Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`, :class:`str`], :class:`bool`]], :class:`RemappedKeyData`]]]
            raw data to provide into the object that contains either: :raw-html:`<br />` :raw-html:`<br />`

            * The data for remapping a particular key OR
            * A list containing:

                * The new names of the keys to remap to OR
                * A tuple containing a new name for the key to remap to and a predicate that takes in the old key and value of whether to remap the key. OR
                * A class that contains all the necessary information for remapping to the new key

        keepKeyWithoutRemap: :class:`bool`
            Whether retain the old register, if the old register does not get remapped :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (isinstance(remappedKeys, cls)):
            return remappedKeys

        remappedKeys = list(map(lambda remappedKey: RemappedKeyData.build(remappedKey), remappedKeys))
        return cls(remappedKeys, keepKeyWithoutRemap)


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

    def __getitem__(self, key: Union[str, int]) -> Union[List[Tuple[int, str]], Tuple[str, str, int]]:
        if (isinstance(key, int)):
            kvpRef = self._order[key]
            val = self.src[kvpRef[0]][kvpRef[1]][1]
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
        Union[List[Tuple[:class:`int`, :class:`str`]], Tuple[:class:`str`, :class:`str`, :class:`int`], Any]
            Either the found value or the default value
        """

        try:
            return self.__getitem__(key)
        except KeyError:
            return default
        except IndexError:
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
        self._src = copy.copy(newSrc)
        for key in self._src:
            self._src[key] = sorted(self._src[key], key = lambda data: data[0])

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
    
    def _updateOrderOccurrencesAfterRemoval(self, key: str, ind: int, change: int):
        newInd = ind + change
        valData = self._src[key][newInd]
        orderInd = valData[0]

        keyData = self._order[orderInd]
        self._order[orderInd] = (keyData[0], newInd)
    
    def removeKey(self, key: Union[str, Tuple[str, Callable[[Tuple[int, str], "IfContentPart"], bool]]]):
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
        pred = lambda val, part: True
        targetKey = key

        if (isinstance(key, tuple) and len(key) >= 2):
            pred = key[1]
            targetKey = key[0]

        try:
            values = self.src[targetKey]
        except KeyError:
            return
        
        currentValRemovedInds = set()
        valuesLen = len(values)

        for i in range(valuesLen):
            value = values[i]
            if (pred(value, self)):
                orderIndsToRemove.add(value[0])
                currentValRemovedInds.add(i)

        if (len(currentValRemovedInds) == len(values)):
            del self.src[targetKey]
        else:
            keyValsLen = len(self.src[targetKey])
            self.src[targetKey] = ListTools.removeByInds(values, currentValRemovedInds)

            currentValRemovedInds = list(currentValRemovedInds)
            currentValRemovedInds.sort()

            ListTools.updateIndsAfterRemove(currentValRemovedInds, keyValsLen, lambda ind, change: self._updateOrderOccurrencesAfterRemoval(targetKey, ind, change))

        self._order = ListTools.removeByInds(self._order, orderIndsToRemove)

        # update the order indices
        orderLen = len(self._order)
        for i in range(orderLen):
            orderData = self._order[i]
            valData = self.src[orderData[0]][orderData[1]]
            self.src[orderData[0]][orderData[1]] = (i, valData[1])

    def removeKeys(self, keys: Set[Union[str, Tuple[str, Callable[[Tuple[int, str], "IfContentPart"], bool]]]]):
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
            pred = lambda val, part: True
            targetKey = key

            if (isinstance(key, tuple) and len(key) >= 2):
                pred = key[1]
                targetKey = key[0]

            values = None
            try:
                values = self.src[targetKey]
            except KeyError:
                continue
            
            currentValRemovedInds = set()
            valuesLen = len(values)

            for i in range(valuesLen):
                value = values[i]
                if (pred(value, self)):
                    orderIndsToRemove.add(value[0])
                    currentValRemovedInds.add(i)

            if (len(currentValRemovedInds) == len(values)):
                del self.src[targetKey]
            else:
                keyValsLen = len(self.src[targetKey])
                self.src[targetKey] = ListTools.removeByInds(values, currentValRemovedInds)

                currentValRemovedInds = list(currentValRemovedInds)
                currentValRemovedInds.sort()

                ListTools.updateIndsAfterRemove(currentValRemovedInds, keyValsLen, lambda ind, change: self._updateOrderOccurrencesAfterRemoval(targetKey, ind, change))

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
                valData = self.src[key][i]
                self.src[key][i] = (valData[0], vals[i])

    def reorder(self, orderMap: Dict[int, int]):
        """
        Reorders the `KVPs`_

        Parameters
        ----------
        orderMap: Dict[:class:`int`, :class:`int`]
            The mapping of how to reorder the `KVPs`_ :raw-html:`<br />` :raw-html:`<br />`

            The keys are the original indices of the `KVPs`_ and the values are the new indices for the `KVPs`_
        """

        newOrder = []
        orderLen = len(self._order)

        for i in range(orderLen + 1):
            currentOrderPart = deque()
            if (i < orderLen and i not in orderMap):
                currentOrderPart.append(self._order[i])

            newOrder.append(currentOrderPart)

        # move the kvps
        for fromInd in orderMap:
            toInd = orderMap[fromInd]
            keyData = self._order[fromInd]

            if (toInd > orderLen):
                toInd = orderLen
            elif (-orderLen - 1 < toInd < 0):
                toInd = orderLen + 1 + toInd
            elif (toInd <= -orderLen - 1):
                toInd = 0

            newOrder[toInd].append(keyData)

        # update the src
        newOrder = list(IT.chain(*newOrder))
        orderLen = len(newOrder)

        for i in range(orderLen):
            key, occurence = newOrder[i]
            _, val = self._src[key][occurence]
            self._src[key][occurence] = (i, val)
        
        # update the ordering
        for key in self._src:
            keyValData = self._src[key]
            keyValData.sort(key = lambda valData: valData[0])
            keyValDataLen = len(keyValData)

            for i in range(keyValDataLen):
                orderInd, val = keyValData[i]
                newOrder[orderInd] = (key, i)

        self._order = newOrder

    def remapKeys(self, keyRemap: Dict[str, Union[KeyRemapData, List[Union[str, Tuple[str, Callable[[str, str], bool]], RemappedKeyData]]]]):
        """
        Remaps the keys in the `KVP`_s of the parts

        Parameters
        ----------
        keyRemap: Dict[:class:`str`, Union[:class:`KeyRemapData`, List[Union[:class:`str`, Tuple[:class:`str`, Callable[[:class:`str`, :class:`str`], :class:`bool`]], :class:`RemappedKeyData`]]]]
            The remap for the keys, where: :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the old names of the keys to be remapped
            * the values are either:

                * The data for remapping a particular key OR
                * A list containing either:

                    * The new names of the keys to remap to OR
                    * A tuple containing a new name for the key to remap to and a predicate that takes in the old key and value of whether to remap the key. OR
                    * A class that contains all the necessary information for remapping to the new key
        """

        i = 0
        orderLen = len(self._order)
        keysToRemove = set()
        keysToAdd = set()
        convertedKeyRemap = {}

        newOrder = []
        for i in range((orderLen + 1) * 2):
            newOrder.append([])

        newSrc = defaultdict(lambda: [])
        for key in self._src:
            newSrc[key]

        for i in range(orderLen):
            keyData = self._order[i]
            key = keyData[0]
            fromKeyOccurence = keyData[1]
            newFromKeyOccurence = len(newSrc[key])

            keyValData = self.src[key][fromKeyOccurence]
            keyVal = keyValData[1]

            inKeyRemap = key in keyRemap
            if (not inKeyRemap):
                newSrc[key].append((-1, keyVal))
                newOrder[i * 2].append((key, newFromKeyOccurence))
                continue

            newKeys = keyRemap[key]
            newKeysLen = len(newKeys)

            if (key not in convertedKeyRemap):
                convertedKeyRemap[key] = KeyRemapData.build(newKeys)

            newKeys = convertedKeyRemap[key]
            keepKeyWithoutRemap = newKeys.keepKeyWithoutRemap
            keyRemapped = False

            # construct the remapped keys
            for j in range(newKeysLen):
                newKeyData = newKeys[j]
                newKey = newKeyData.key
                check = newKeyData.check
                toInd = newKeyData.toInd

                if (check is not None and not check(key, keyVal)):
                    continue

                toKeyOccurence = len(newSrc[newKey])

                if (not keyRemapped):
                    keyRemapped = True

                hasToInd = toInd is not None
                if (not hasToInd):
                    toInd = i

                if (toInd >= orderLen):
                    toInd = orderLen
                elif (-orderLen - 1 < toInd < 0):
                    toInd = orderLen + 1 + toInd
                elif (toInd <= -orderLen - 1):
                    toInd = 0

                toInd *= 2
                if (hasToInd):
                    toInd += 1

                newOrder[toInd].append((newKey, toKeyOccurence))
                newSrc[newKey].append((-1, keyVal))

                keysToAdd.add(newKey)

            if (not keyRemapped and keepKeyWithoutRemap):
                newSrc[key].append((-1, keyVal))
                newOrder[i * 2].append((key, newFromKeyOccurence))

            if (not keepKeyWithoutRemap or (keyRemapped and keepKeyWithoutRemap)):
                keysToRemove.add(key)
            
        # remove the keys that do not appear after the remap
        for key in keysToRemove:
            if (key not in keysToAdd):
                del newSrc[key]

        self._order = list(IT.chain(*newOrder))
        self._src = dict(newSrc)
        newSrc = []
        newOrder = []

        # update the src
        orderLen = len(self._order)

        for i in range(orderLen):
            key, occurence = self._order[i]
            _, val = self._src[key][occurence]
            self._src[key][occurence] = (i, val)
        
        # update the ordering
        for key in self._src:
            keyValData = self._src[key]
            keyValData.sort(key = lambda valData: valData[0])
            keyValDataLen = len(keyValData)

            for i in range(keyValDataLen):
                orderInd, val = keyValData[i]
                self._order[orderInd] = (key, i)
##### EndScript