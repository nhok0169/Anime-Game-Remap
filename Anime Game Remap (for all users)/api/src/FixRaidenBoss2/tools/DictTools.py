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
from typing import Dict, Any, Hashable, Optional, Callable, List
##### EndExtImports


##### Script
class DictTools():
    """
    Tools for handling with Dictionaries
    """

    @classmethod
    def getFirstKey(cls, dict: Dict[Any, Any]) -> Any:
        """
        Retrieves the first key in a dictionary

        Parameters
        ----------
        dict: Dict[Any, Any]
            The dictionary we are working with

            .. note::
                The dictionary must not be empty

        Returns
        -------
        Any
            The first key of the dictionary
        """

        return next(iter(dict))

    @classmethod
    def getFirstValue(cls, dict: Dict[Any, Any]) -> Any:
        """
        Retrieves the first value in a dictionary

        Parameters
        ----------
        dict: Dict[Any, Any]
            The dictionary we are working with

        Returns
        -------
        Any
            The first value of the dictionary
        """

        return dict[cls.getFirstKey(dict)]
    
    @classmethod
    def update(cls, srcDict: Dict[Hashable, Any], newDict: Dict[Hashable, Any], combineDuplicate: Optional[Callable[[Hashable, Any, Any], Any]] = None) -> Dict[Hashable, Any]:
        """
        Updates ``srcDict`` based off the new values from ``newDict``

        Parameters
        ----------
        srcDict: Dict[Hashable, Any]
            The dictionary to be updated

        newDict: Dict[Hashable, Any]
            The dictionary to help with updating ``srcDict``

        combineDuplicate: Optional[Callable[[`Hashable`_, Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter is the key that is in both dictionary
            * The second parameter is the value that comes from ``srcDict``
            * The third parameter is the value that comes from ``newDict``

            If this value is set to ``None``, then will use the key from ``newDict`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[Hashable, Any]
            Reference to the updated dictionary
        """

        if (combineDuplicate is None):
            srcDict.update(newDict)
            return srcDict
        
        combinedValues = {}
        srcDictLen = len(srcDict)
        newDictLen = len(newDict)
        
        shortDict = srcDict
        longDict = newDict
        if (srcDictLen > newDictLen):
            shortDict = newDict
            longDict = srcDict

        for key in shortDict:
            if (key in longDict):
                combinedValues[key] = combineDuplicate(key, srcDict[key], newDict[key])

        srcDict.update(newDict)
        srcDict.update(combinedValues)
        return srcDict


    @classmethod
    def combine(cls, dict1: Dict[Hashable, Any], dict2: Dict[Hashable, Any], combineDuplicate: Optional[Callable[[Hashable, Any, Any], Any]] = None) -> Dict[Hashable, Any]:
        """
        Creates a new dictionary from combining 2 dictionaries

        Parameters
        ----------
        dict1: Dict[Hashable, Any]
            The destination of where we want the combined dictionaries to be stored

        dict2: Dict[Hashable, Any]
            The dictionary we want to combine with

        combineDuplicate: Optional[Callable[[`Hashable`_, Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter is the key that is in both dictionary
            * The second parameter is the value that comes from ``srcDict``
            * The third parameter is the value that comes from ``newDict``

            If this value is set to ``None``, then will use the key from 'dict2' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        makeNewCopy: :class:`bool`
            Whether we want the resultant dictionary to be newly created or to be updated into ``dict1``

        Returns
        -------
        Dict[Hashable, Any]
            The new combined dictionary
        """

        new_dict = {**dict1, **dict2}

        if (combineDuplicate is None):
            return new_dict

        for key in new_dict:
            if key in dict1 and key in dict2:
                new_dict[key] = combineDuplicate(key, new_dict[key], dict1[key])

        return new_dict
    
    @classmethod
    def invert(cls, dict: Dict[Hashable, Hashable]) -> Dict[Hashable, Hashable]:
        """
        Inverts a dictionary by making the keys the values and the values the keys

        Parameters
        ----------
        dict: Dict[Hashable, Hashable]
            The dictionary to invert

        Returns
        -------
        Dict[Hashable, Hashable]
            The inverted dictionary
        """

        return {v: k for k, v in dict.items()}
    
    @classmethod
    def filter(cls, dict: Dict[Hashable, Any], predicate: Callable[[Hashable, Any], bool]) -> Dict[Hashable, Any]:
        """
        Filters a dictionary

        Parameters
        ----------
        dict: Dict[Hashable, Hashable]
            The dictionary to filter

        predicate: Callable[[Hashable, Any], :class:`bool`]
            The predicate used for the filter :raw-html:`<br />` :raw-html:`<br />`

            The predicate has the following parameters

            #. The key of the dictionary
            #. The value of the dictionary

        Returns
        -------
        Dict[Hashable, Any]
            The filtered dictionary
        """

        return {key: value for key, value in dict.items() if predicate(key, value)}
    
    @classmethod
    def _forDict(cls, nestedDict: Dict[Hashable, Any], keyNames: List[str], func: Callable[[Dict[str, str], Dict[str, Any]], Any], currentKeyInd: int, keys: Dict[str, Hashable], values: Dict[str, Hashable]) -> Any:
        keyNamesLen = len(keyNames)
        if (currentKeyInd >= keyNamesLen):
            func(keys, values)
            return

        keyName = keyNames[currentKeyInd]
        for key in nestedDict:
            currentVal = nestedDict[key]
            keys[keyName] = key
            values[keyName] = currentVal
            cls._forDict(currentVal, keyNames, func, currentKeyInd + 1, keys, values)

    @classmethod
    def forDict(cls, nestedDict: Dict[Hashable, Any], keyNames: List[str], func: Callable[[Dict[str, Hashable], Dict[str, Any]], Any]):
        """
        Iterates over a nested dictionary

        Parameters
        ----------
        nestedDict: Dict[Hashable, Any]
            The nested dictionary to iterate over

        keyNames: List[:class:`str`]
            The variable names of the keys in the nested dictionary

        func: Callable[Dict[:class:`str`, Hashable], Dict[:class:`str`, Any], Any]
            callback function that will be called at the leaf node of the nested dictionary :raw-html:`<br />` :raw-html:`<br />`

            The function contains the following arguments:
            #. The dictionary keys encountered in the current iteration
            #. The corresponding values encountered at each dictionary layer in the current iteration
        """

        keys = {}
        values = {}
        cls._forDict(nestedDict, keyNames, func, 0, keys, values)
        
##### EndScript