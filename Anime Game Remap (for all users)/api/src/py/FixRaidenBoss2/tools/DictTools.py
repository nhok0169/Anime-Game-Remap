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
from typing import Dict, Any, Hashable, Optional, Callable, List, Tuple, Union
##### EndExtImports

##### CyLocalImports
from ..CyDictTools import CyDictTools
##### EndCyLocalImports

##### LocalImports
from ..constants.GenericTypes import PdDataFrame
from ..constants.GlobalPackageManager import GlobalPackageManager
from ..constants.Packages import PackageModules
##### EndLocalImports


##### Script
class UnHashableNone():
    """
    A symbol to indicate a null value in some hash data structure
    """

    pass


class DictTools():
    """
    Tools for handling with Dictionaries
    """

    _CyTools = CyDictTools()

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

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.update`

        .. note::
            ``srcDict``/``newDict`` may be :class:`dict` subclasses (e.g. `DefaultDict`_) -- the
            returned dictionary is ``srcDict`` itself, so it preserves whatever subclass
            ``srcDict`` actually is

        Parameters
        ----------
        srcDict: Dict[Hashable, Any]
            The dictionary to be updated

        newDict: Dict[Hashable, Any]
            The dictionary to help with updating ``srcDict``

        combineDuplicate: Optional[Callable[[Hashable, Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter is the key that is in both dictionary
            * The second parameter is the value that comes from ``srcDict``
            * The third parameter is the value that comes from ``newDict``

            If this value is set to ``None``, then will use the key from ``newDict`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`TypeError`
            If ``srcDict`` or ``newDict`` is not a :class:`dict` (or a subclass of it)

        Returns
        -------
        Dict[Hashable, Any]
            Reference to the updated dictionary
        """

        return cls._CyTools.update(srcDict, newDict, combineDuplicate)

    @classmethod
    def updateMany(cls, srcDict: Dict[Hashable, Any], dictList: List[Dict[Hashable, Any]], combineDuplicate: Optional[Callable[[Hashable, Dict[int, Any]], Any]] = None) -> Dict[Hashable, Any]:
        """
        Updates ``srcDict`` based off the new values from a list of dictionaries

        This is the same as :meth:`update`, generalized to more than one 'newDict' at once

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.updateMany`

        .. note::
            ``srcDict``/entries of ``dictList`` may be :class:`dict` subclasses (e.g.
            `DefaultDict`_) -- the returned dictionary is ``srcDict`` itself, so it preserves
            whatever subclass ``srcDict`` actually is

        Parameters
        ----------
        srcDict: Dict[Hashable, Any]
            The dictionary to be updated

        dictList: List[Dict[Hashable, Any]]
            The dictionaries to help with updating ``srcDict``, applied in order

        combineDuplicate: Optional[Callable[[Hashable, Dict[:class:`int`, Any]], Any]]
            Function for handling cases where a key is shared by 2 or more of the dictionaries
            being merged together (``srcDict`` and every dictionary in ``dictList``) :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter is the shared key
            * The second parameter is a :class:`dict` mapping the *index* of a dictionary that
              has this key to the corresponding value at this key :raw-html:`<br />` :raw-html:`<br />`

              The indices treat ``srcDict`` and ``dictList`` as one combined, 0-indexed sequence
              (``srcDict`` is index ``0``; ``dictList[i]`` is index ``i + 1``) -- only indices
              belonging to dictionaries that actually have the shared key are included

            If this value is set to ``None``, then the dictionaries in ``dictList`` are applied
            to ``srcDict`` in order via plain :meth:`dict.update`, i.e. the last dictionary to
            have a given key wins :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`TypeError`
            If ``srcDict`` or any entry of ``dictList`` is not a :class:`dict` (or a subclass of it)

        Returns
        -------
        Dict[Hashable, Any]
            Reference to the updated ``srcDict``
        """

        return cls._CyTools.updateMany(srcDict, dictList, combineDuplicate)

    @classmethod
    def combine(cls, dict1: Dict[Hashable, Any], dict2: Dict[Hashable, Any], combineDuplicate: Optional[Callable[[Hashable, Any, Any], Any]] = None, makeNewCopy: bool = True) -> Dict[Hashable, Any]:
        """
        Creates a dictionary from combining 2 dictionaries

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.combine`

        .. note::
            ``dict1``/``dict2`` may be :class:`dict` subclasses (e.g. `DefaultDict`_) -- if
            ``makeNewCopy`` is ``False``, the returned dictionary is ``dict1`` itself, so it
            preserves whatever subclass ``dict1`` actually is

        Parameters
        ----------
        dict1: Dict[Hashable, Any]
            The destination of where we want the combined dictionaries to be stored

        dict2: Dict[Hashable, Any]
            The dictionary we want to combine with

        combineDuplicate: Optional[Callable[[Hashable, Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter is the key that is in both dictionary
            * The second parameter is the value that comes from ``dict1``
            * The third parameter is the value that comes from ``dict2``

            If this value is set to ``None``, then will use the value from ``dict2`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        makeNewCopy: :class:`bool`
            Whether we want the resultant dictionary to be newly created or to be updated into ``dict1`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Raises
        ------
        :class:`TypeError`
            If ``dict1`` or ``dict2`` is not a :class:`dict` (or a subclass of it)

        Returns
        -------
        Dict[Hashable, Any]
            The combined dictionary :raw-html:`<br />` :raw-html:`<br />`

            If ``makeNewCopy`` is ``True``, this is a newly created dictionary. Otherwise, this is
            ``dict1`` itself, updated in place
        """

        return cls._CyTools.combine(dict1, dict2, combineDuplicate, makeNewCopy = makeNewCopy)

    @classmethod
    def combineMany(cls, dict1: Dict[Hashable, Any], dictList: List[Dict[Hashable, Any]], combineDuplicate: Optional[Callable[[Hashable, Dict[int, Any]], Any]] = None, makeNewCopy: bool = True) -> Dict[Hashable, Any]:
        """
        Creates a dictionary from combining ``dict1`` with a list of dictionaries

        This is the same as :meth:`combine`, generalized to more than one 'dict2' at once

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.combineMany`

        .. note::
            ``dict1``/entries of ``dictList`` may be :class:`dict` subclasses (e.g.
            `DefaultDict`_) -- if ``makeNewCopy`` is ``False``, the returned dictionary is
            ``dict1`` itself, so it preserves whatever subclass ``dict1`` actually is

        Parameters
        ----------
        dict1: Dict[Hashable, Any]
            The destination of where we want the combined dictionaries to be stored

        dictList: List[Dict[Hashable, Any]]
            The dictionaries we want to combine with ``dict1``, applied in order

        combineDuplicate: Optional[Callable[[Hashable, Dict[:class:`int`, Any]], Any]]
            Function for handling cases where a key is shared by 2 or more of the dictionaries
            being combined (``dict1`` and every dictionary in ``dictList``) :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter is the shared key
            * The second parameter is a :class:`dict` mapping the *index* of a dictionary that
              has this key to the corresponding value at this key :raw-html:`<br />` :raw-html:`<br />`

              The indices treat ``dict1`` and ``dictList`` as one combined, 0-indexed sequence
              (``dict1`` is index ``0``; ``dictList[i]`` is index ``i + 1``) -- only indices
              belonging to dictionaries that actually have the shared key are included

            If this value is set to ``None``, then the dictionaries in ``dictList`` are applied
            in order via plain :meth:`dict.update`, i.e. the last dictionary to have a given key
            wins :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        makeNewCopy: :class:`bool`
            Whether we want the resultant dictionary to be newly created or to be updated into
            ``dict1`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Raises
        ------
        :class:`TypeError`
            If ``dict1`` or any entry of ``dictList`` is not a :class:`dict` (or a subclass of it)

        Returns
        -------
        Dict[Hashable, Any]
            The combined dictionary :raw-html:`<br />` :raw-html:`<br />`

            If ``makeNewCopy`` is ``True``, this is a newly created dictionary. Otherwise, this
            is ``dict1`` itself, updated in place
        """

        return cls._CyTools.combineMany(dict1, dictList, combineDuplicate, makeNewCopy = makeNewCopy)

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
    def forDict(cls, nestedDict: Dict[Hashable, Any], keyNames: List[str], func: Callable[[Dict[str, Hashable], Dict[str, Any]], Any], ordered: bool = True):
        """
        Iterates over a nested dictionary

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.forDict`

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

        ordered: :class:`bool`
            Whether to visit leaves in the same order the keys/values were inserted into ``nestedDict`` :raw-html:`<br />` :raw-html:`<br />`
 
            Setting this to ``False`` skips the extra bookkeeping needed to preserve order, which is slightly
            faster but the traversal order becomes unspecified :raw-html:`<br />` :raw-html:`<br />`
 
            **Default**: ``True``
        """

        cls._CyTools.forDict(nestedDict, keyNames, func, ordered)

    @classmethod
    def iterDict(cls, nestedDict: Dict[Hashable, Any], keyNames: List[str], leafOnly: bool = False, ordered: bool = True):
        """
        Iterates over a nested dictionary, yielding at each leaf node
 
        This is the generator equivalent of :meth:`forDict`, so no callback function is needed

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.iterDict`
 
        Parameters
        ----------
        nestedDict: Dict[Hashable, Any]
            The nested dictionary to iterate over
 
        keyNames: List[:class:`str`]
            The variable names of the keys in the nested dictionary
 
        leafOnly: :class:`bool`
            Whether to only yield the leaf value at each iteration, instead of the keys/values encountered
            at each dictionary layer :raw-html:`<br />` :raw-html:`<br />`
 
            **Default**: ``False``

        ordered: :class:`bool`
            Whether to visit leaves in the same order the keys/values were inserted into ``nestedDict`` :raw-html:`<br />` :raw-html:`<br />`
 
            Setting this to ``False`` skips the extra bookkeeping needed to preserve order, which is slightly
            faster but the traversal order becomes unspecified :raw-html:`<br />` :raw-html:`<br />`
 
            **Default**: ``True``
 
        Yields
        ------
        Union[Tuple[Dict[:class:`str`, Hashable], Dict[:class:`str`, Any]], Any]
            If ``leafOnly`` is ``False``:
 
            #. The dictionary keys encountered in the current iteration
            #. The corresponding values encountered at each dictionary layer in the current iteration
 
            If ``leafOnly`` is ``True``, then only the leaf value at the current iteration
        """
 
        yield from cls._CyTools.iterDict(nestedDict, keyNames, leafOnly, ordered)

    @classmethod
    def nestedDictToDataFrame(cls, nestedDict: Dict[Hashable, Any], colNames: List[str]) -> PdDataFrame:
        """
        Transforms a nested dictionary into a `pandas DataFrame`_

        Parameters
        ----------
        nestedDict: Dict[Hashable, Any]
            The nested dictionary to convert

        colNames: List[:class:`str`]
            The names for the columns in the nested dictionary

            .. warning::
                The list must have at least 2 values

        Returns
        -------
        `pandas.DataFrame`_
            The converted data
        """

        result = cls._CyTools.nestedDictToNdArray(nestedDict, colNames)
        pd = GlobalPackageManager.get(PackageModules.Pandas.value)
        return pd.DataFrame(result, columns = colNames)

    @classmethod
    def getKeys(cls, dictList: List[Dict[Hashable, Any]], ordered: bool = True) -> List[str]:
        """
        Gets the unique keys found across a list of dictionaries

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.getKeys`

        Parameters
        ----------
        dictList: List[Dict[Hashable, Any]]
            The list of dictionaries to gather keys from

        ordered: :class:`bool`
            Whether to return the keys in the order they were first encountered while
            iterating over ``dictList`` :raw-html:`<br />` :raw-html:`<br />`

            Setting this to ``False`` skips the extra bookkeeping needed to preserve order,
            which is slightly faster but the order of the returned keys becomes unspecified
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        List[Hashable]
            A list of the unique keys found across all dictionaries in ``dictList``
        """

        return cls._CyTools.getKeys(dictList, ordered = ordered)

    @classmethod
    def getVal(cls, dct: Dict[Hashable, Any], keys: Union[List[Hashable], Tuple[Hashable, ...]], errorOnNotFound: bool = False, default: Any = None) -> Any:
        """
        Retrieves the corresponding value from a nested dictionary

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.getVal`

        .. note::
            ``dct`` may be a :class:`dict` subclass (e.g. `DefaultDict`_) at any level, including
            ``dct`` itself. Indexing here always behaves like a plain :class:`dict` lookup, so a
            missing key along the path is correctly reported as not found rather than triggering
            a `DefaultDict`_'s ``default_factory`` (which would otherwise silently create and
            return an empty value instead of reporting "not found")

        Parameters
        ----------
        dct: Dict[Hashable, Any]
            The nested dictionary to query

        keys: Union[List[Hashable], Tuple[Hashable, ...]]
            The keys used to query the dictionary :raw-html:`<br />` :raw-html:`<br />`

            If the amount of keys provided is less than the amount of layers in ``dct``, then the corresponding
            :class:`dict` at that layer will be returned. Otherwise, the corresponding leaf value will be returned

        errorOnNotFound: :class:`bool`
            Whether to raise an exception if the value is not found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if the value is not found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            If the corresponding value based on 'keys' is not found and 'errorOnNotFound' is set to ``True`` --
            this includes the case where ``dct`` itself is not a :class:`dict` (or a subclass of it)

        Returns
        -------
        Any
            Either:

            * The found value OR
            * The value specified from 'default' if 'errorOnNotFound' is set to ``False``
        """

        return cls._CyTools.getVal(dct, keys, errorOnNotFound = errorOnNotFound, default = default)

    @classmethod
    def contains(cls, dct: Dict[Hashable, Any], keys: Union[List[Hashable], Tuple[Hashable, ...]]) -> bool:
        """
        Determines whether a path of keys exists within a nested dictionary

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.contains`

        .. note::
            ``dct`` may be a :class:`dict` subclass (e.g. `DefaultDict`_) at any level, including
            ``dct`` itself -- this only ever checks for key membership, so a missing key along
            the path is correctly reported as not found rather than triggering a `DefaultDict`_'s
            ``default_factory``

        Parameters
        ----------
        dct: Dict[Hashable, Any]
            The nested dictionary to query

        keys: Union[List[Hashable], Tuple[Hashable, ...]]
            The keys used to query the dictionary, representing the path to check for within ``dct`` :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                The amount of keys provided does not need to match the amount of layers in ``dct``

        Returns
        -------
        :class:`bool`
            Whether the path specified by 'keys' exists within ``dct`` :raw-html:`<br />` :raw-html:`<br />`

            If ``dct`` itself is not a :class:`dict` (or a subclass of it), ``False`` is returned
        """

        return cls._CyTools.contains(dct, keys)

    @classmethod
    def setVal(cls, dct: Dict[Hashable, Any], keys: Union[List[Hashable], Tuple[Hashable, ...]], value: Any) -> None:
        """
        Sets the value at a key-path within a nested dictionary, creating any missing
        intermediate :class:`dict` layers along the way (a "deep set")

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.setVal`

        .. note::
            If some key along the path already maps to a value that is not a :class:`dict`,
            that value is overwritten with a new, empty :class:`dict` so the path can continue

        .. note::
            ``dct`` (and any nested :class:`dict` created/traversed along the path) may be a
            :class:`dict` subclass (e.g. `DefaultDict`_)

        Parameters
        ----------
        dct: Dict[Hashable, Any]
            The nested dictionary to modify in place

        keys: Union[List[Hashable], Tuple[Hashable, ...]]
            The keys representing the path within ``dct`` to set ``value`` at :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If ``keys`` is empty, this has no effect and ``dct`` is left unchanged

        value: Any
            The new value to set at the corresponding path within ``dct``

        Raises
        ------
        :class:`TypeError`
            If ``dct`` is not a :class:`dict` (or a subclass of it)
        """

        cls._CyTools.setVal(dct, keys, value)

    @classmethod
    def getCommonKeys(cls, dictList: List[Dict[Hashable, Any]], ordered: bool = True) -> List[Hashable]:
        """
        Retrieves the intersection of the keys found across a list of dictionaries

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.getCommonKeys`

        Parameters
        ----------
        dictList: List[Dict[Hashable, Any]]
            The list of dictionaries to gather keys from

        ordered: :class:`bool`
            Whether to retrieve the keys in the order they were first inserted into
            ``dictList``'s first (dict-valued) entry, i.e. standard Python 3.7+ :class:`dict`
            insertion order :raw-html:`<br />` :raw-html:`<br />`

            Setting this to ``False`` skips the extra bookkeeping needed to preserve order,
            which is slightly faster but the order of the returned keys becomes unspecified
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        List[Hashable]
            A list of the keys common to every dictionary in ``dictList`` :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If ``dictList`` has no :class:`dict` entries, an empty list is returned
        """

        return cls._CyTools.getCommonKeys(dictList, ordered = ordered)

    @classmethod
    def getCommonPaths(cls, dictList: List[Dict[Hashable, Any]], ordered: bool = True) -> List[List[Hashable]]:
        """
        Retrieves the maximal key-paths common across a list of nested dictionaries

        A "path" here is the same notion used by :meth:`contains`: a sequence of keys that can
        be followed, one nested :class:`dict` layer at a time, starting from the root of a
        dictionary. A path is only included in the result if it cannot be extended by any
        further key while remaining common to every (dict-valued) entry in ``dictList`` --
        e.g. if the only path shared between ``dictA`` and ``dictB`` is ``["1", "2", "3"]``,
        the result contains ``["1", "2", "3"]`` alone, not its prefixes ``["1"]``/
        ``["1", "2"]`` as well

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.getCommonPaths`

        .. note::
            Any entry in ``dictList`` that is not a :class:`dict` is ignored, the same as in
            :meth:`getCommonKeys`

        Parameters
        ----------
        dictList: List[Dict[Hashable, Any]]
            The list of nested dictionaries to gather the common paths from

        ordered: :class:`bool`
            Whether to traverse/retrieve the common paths (and any branching keys within a
            path) in the order the keys were first inserted into ``dictList``'s first (dict-valued)
            entry, i.e. standard Python 3.7+ :class:`dict` insertion order :raw-html:`<br />` :raw-html:`<br />`

            Setting this to ``False`` skips the extra bookkeeping needed to preserve order,
            which is slightly faster (falls back to :meth:`getCommonKeys`'s :class:`set`-based
            intersection) but the order of the returned paths becomes unspecified :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        List[List[Hashable]]
            The maximal key-paths common to every (dict-valued) entry in ``dictList`` :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If ``dictList`` has no :class:`dict` entries, an empty list is returned
        """

        return cls._CyTools.getCommonPaths(dictList, ordered = ordered)

    @classmethod
    def iterPaths(cls, dct: Dict[Hashable, Any]):
        """
        Iterates over a nested dictionary, yielding at each leaf path

        This is the generator equivalent of applying :meth:`getCommonPaths` to a single
        dictionary: a "path" is the same notion used by :meth:`contains`/:meth:`getVal`/
        :meth:`setVal`/:meth:`getCommonPaths`, and only "leaf"/maximal paths are yielded -- a
        path that ends at either a non-:class:`dict` value, or an empty :class:`dict` (nothing
        further to descend into)

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.iterPaths`

        .. note::
            ``dct`` (and any nested :class:`dict` along the way) may be a :class:`dict` subclass
            (e.g. `DefaultDict`_)

        .. note::
            Paths are yielded in the same order the keys were inserted into ``dct`` (and its
            nested dictionaries), depth-first

        Parameters
        ----------
        dct: Dict[Hashable, Any]
            The nested dictionary to iterate over

        Yields
        ------
        List[Hashable]
            The next leaf path found within ``dct`` :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If ``dct`` itself is empty (or not a :class:`dict`, or a subclass of it),
                nothing is yielded
        """

        yield from cls._CyTools.iterPaths(dct)

    @classmethod
    def getPaths(cls, dct: Dict[Hashable, Any], ordered: bool = True) -> List[List[Hashable]]:
        """
        Retrieves all the maximal key-paths within a nested dictionary

        This is the eager, :class:`list`-returning equivalent of :meth:`iterPaths` -- a "path"
        is the same notion used by :meth:`contains`/:meth:`getVal`/:meth:`setVal`/
        :meth:`getCommonPaths`, and only "leaf"/maximal paths are retrieved -- a path that ends
        at either a non-:class:`dict` value, or an empty :class:`dict` (nothing further to
        descend into)

        .. note::
            This function is a convenience for calling :meth:`CyDictTools.getPaths`

        .. note::
            ``dct`` (and any nested :class:`dict` along the way) may be a :class:`dict` subclass
            (e.g. `DefaultDict`_)

        Parameters
        ----------
        dct: Dict[Hashable, Any]
            The nested dictionary to retrieve the paths from

        ordered: :class:`bool`
            Whether to retrieve the paths in the same order the keys were inserted into ``dct``
            (and its nested dictionaries), depth-first :raw-html:`<br />` :raw-html:`<br />`

            Setting this to ``False`` skips the extra bookkeeping needed to preserve order,
            which is slightly faster but the order of the returned paths becomes unspecified
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        List[List[Hashable]]
            The maximal key-paths found within ``dct`` :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If ``dct`` itself is empty (or not a :class:`dict`, or a subclass of it), an
                empty list is returned
        """

        return cls._CyTools.getPaths(dct, ordered = ordered)
##### EndScript