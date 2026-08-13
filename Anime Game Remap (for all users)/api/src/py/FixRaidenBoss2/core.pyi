"""
C++ internal core of AGRemap
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['BaseDFA', 'BiMap', 'CppAhoCorasickDFA', 'CppAlgo', 'CppIfContentPart', 'CppIfTemplatePart', 'CppIntTools', 'CppListTools', 'CppTrie', 'DFA', 'IOrderedMultiMap', 'IfContentPartColourChange', 'IfContentPartColouring', 'KeyRemapData', 'OrderedMultiMap', 'OrderedMultiMapIterator', 'OrderedMultiMapSqrt', 'OrderedMultiMapSqrtIterator', 'Ranges', 'RangesInt', 'RemappedKeyData', 'ReplaceIf', 'ReplaceList', 'appendAllToOrderedMultiMap']
class BaseDFA:
    def acceptLen(self) -> int:
        """
        Retrieves the number of accepting states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of accepting states in the `DFA`_
        """
    def addState(self, id: typing.Any, isAccept: bool | None = None, isStart: bool = False) -> bool:
        """
        Add a new state to the `DFA`
        
        Parameters
        ----------
        id: Hashable
            The id for the state
        
        isAccept: Optional[:class:`bool`]
            Whether the state is an accepting state :raw-html:`<br />` :raw-html:`<br />`
        
            * If this value is ``None`` and the state already exists, then will not change whether the existing state is accepting or not.
            * Otherwise, if this value is ``None`` and the state does not already exists, then will not set the state as accepting. :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        isStart: :class:`bool`
            Whether to set the state as the new starting state
        
            .. warning::
                A `DFA`_ can only have 1 start state
        
            .. warning::
                If the `DFA`_ is empty and you add a new state, will set this state as the start state
        
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the state was newly added
        """
    def clear(self) -> None:
        """
        Clears the `DFA`_
        """
    def isAccept(self, id: typing.Any) -> bool:
        """
        Determines whether some state is an accepting state
        
        Paramters
        ---------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the corresponding state is an accepting state
        """
    def isStart(self, id: typing.Any) -> bool:
        """
        Determines whether some state is a starting state
        
        Paramters
        ---------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the corresponding state is a starting state
        """
    def reset(self) -> None:
        """
        Resets the `DFA`_ to return back to its starting state
        """
    def stateExists(self, id: typing.Any) -> bool:
        """
        Determines whether some state exists in the `DFA`_
        
        Paramters
        ---------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the id corresponds to a state in the `DFA`_
        """
    def stateLen(self) -> int:
        """
        Retrieves the number of states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of states in the `DFA`_
        """
class BiMap:
    """
    
            A one-to-one dictionary
            
    """
    def __init__(self) -> None:
        ...
    def __len__(self) -> int:
        ...
    def add(self, key: typing.Any, val: typing.Any) -> bool:
        ...
    def clear(self) -> None:
        ...
    def empty(self) -> bool:
        ...
    def findKey(self, val: typing.Any) -> typing.Any | None:
        ...
    def findValue(self, key: typing.Any) -> typing.Any | None:
        ...
    def getKey(self, val: typing.Any) -> typing.Any:
        ...
    def getValue(self, key: typing.Any) -> typing.Any:
        ...
    def insert(self, key: typing.Any, val: typing.Any) -> None:
        ...
class CppAhoCorasickDFA:
    """
    
    A class for an `Aho-Corasick`_ `DFA`_ implemented in C++
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: txt in x
    
            Determines if a keyword is found within 'txt'
    
        .. describe:: x[keyword]
    
            Retrieves the corresponding value to 'keyword'
    
        .. describe:: x[keyword] = val
    
            Sets the new `KVP`_
    
        .. describe:: len(x)
    
            Retrieves the number of elements
    
    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    handleDuplicate: Optional[Callable[[:class:`str`, T, T], T]]
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
    
        The function takes in the following parameters:
    
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
    
        If this value is ``None``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __contains__(self, txt: str) -> bool:
        """
        Determines if a keyword is found within 'txt'
        """
    def __getitem__(self, keyword: str) -> typing.Any:
        """
        Retrieves the corresponding value to 'keyword'
        """
    def __init__(self, data: collections.abc.Mapping[str, typing.Any] | None = None, handleDuplicate: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any] | None = None) -> None:
        ...
    def __len__(self) -> int:
        """
        Retrieves the number of elements
        """
    def __setitem__(self, keyword: str, val: typing.Any) -> bool:
        """
        Sets the new `KVP`_
        """
    def add(self, keyword: str, value: typing.Any) -> bool:
        """
        Adds a new keyword
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add
        
        value: T
            The value associated with the keyword
        
        Returns
        -------
        :class:`bool`
            Whether the keyword has already been inserted
        """
    def build(self, data: collections.abc.Mapping[str, typing.Any] | None = None) -> None:
        """
        Rebuilds the `DFA`_
        
        Parameters
        ----------
        data: Optional[Dict[:class:`str`, T]]
            Any initial data to put into the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears the data
        """
    def contains(self, txt: str) -> bool:
        """
        Determines if 'txt' contains a corresponding keyword from the `DFA`_
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search
        
        Returns
        -------
        :class:`bool`
            Whether text contains a corresponding keyword
        """
    def find(self, txt: str) -> tuple[str | None, int]:
        """
        Finds the first keyword within 'txt'
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword
        
        Returns
        -------
        Tuple[Optional[:class:`str`], :class:`int`]
            Data of the found keyword containing:
        
            #. The keyword found
            #. The starting index of where the keyword was found. The index is only valid if the keyword is found.
        """
    def findAll(self, txt: str) -> dict[str, list[tuple[int, int]]]:
        """
        Finds all occurences of the keywords from the `DFA`_ in the given text
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords
        
        Returns
        -------
        Dict[:class:`str`, List[Tuple[:class:`int`, :class:`int`]]]
            The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`
        
            * The keys are the keywords found
            * The values are all instances of the keyword found
            * The tuple contains the starting index of the found instance and the ending index of the found instance
        """
    def findFirstAll(self, txt: str) -> dict[str, tuple[int, int]]:
        """
        Finds the first occurences of the keywords from the `DFA`_ in the given text
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords
        
        Returns
        -------
        Dict[:class:`str`, Tuple[:class:`int`, :class:`int`]]
            The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`
        
            * The keys are the keywords found
            * The tuple contains the starting index of the found instance and the ending index of the first found instance
        """
    def findMaximal(self, txt: str, count: typing.SupportsInt | typing.SupportsIndex = 1) -> tuple[str | None, int] | tuple[list[str], list[int]]:
        """
        Finds the first few largest keywords within 'txt'
        
        .. note::
            This function is a greedy version of :meth:`find` or `Maximal Munch`_ that consumes only a limited amount of tokens
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword
        
        count: :class:`int`
            The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``1``
        
        Returns
        -------
        Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[:class:`int`, List[:class:`int`]]]
            Data of the found keyword: :raw-html:`<br />` :raw-html:`<br />`
        
            * If the 'count' argument is less than or equal to 1, then the data will contain:
        
                #. The keyword found
                #. The starting index of where the keyword was found.
        
            * If the 'count' argument is greater than 1, then the data will contain:
        
                #. The list of keywords found
                #. The corresponding starting indices for where the keyword were found
        """
    @typing.overload
    def get(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None) -> tuple[str | None, typing.Any]:
        """
        Retrieves the corresponding value from the first keyword fround in 'txt'
        
        .. note::
            This function retrieves the corresponding value after running :meth:`find`
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found
        
        Raises
        ------
        :class:`KeyError`
            If no keywords are found
        
        Returns
        -------
        Tuple[Optional[:class:`str`], Union[T, Any]]
            Retrieves the following resultant data:
        
            #. The first keyword found
            #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
                'errorOnNotFound' is set to ``False``
        """
    @typing.overload
    def get(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None) -> tuple[str | None, typing.Any]:
        """
        Retrieves the corresponding value from the first keyword fround in 'txt'
        
        .. note::
            This function retrieves the corresponding value after running :meth:`find`
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If no keywords are found
        
        Returns
        -------
        Tuple[Optional[:class:`str`], Union[T, Any]]
            Retrieves the following resultant data:
        
            #. The first keyword found
            #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
                'errorOnNotFound' is set to ``False``
        """
    def getAll(self, txt: str) -> dict[str, typing.Any]:
        """
        Retrieves all the corresponding values to all the keywords found within 'txt'
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords
        
        Returns
        -------
        Dict[:class:`str`, T]
            The corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords found and the values are the values to the keywords
        """
    def getKeyVal(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding value of the key given in 'txt'
        
        Parameters
        ----------
        txt: :class:`str`
            The keyword to find the corresponding value
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception
        
        default: Any
            If 'errorsOnNotFound' is ``False``, then the default value to return if no keywords are found
        
        Raises
        ------
        :class:`KeyError`
            If the keyword is not found
        
        Returns
        -------
        Union[T, Any]
            Either the found value for the corresponding keyword or the value specified at 'default', if no keywords were found and
            'errorOnNotFound' is set to ``False``
        """
    def getMaximal(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None, count: typing.SupportsInt | typing.SupportsIndex = 1) -> tuple[str | None, typing.Any] | tuple[list[str], list[typing.Any]]:
        """
        Retrieves the corresponding value from the first largest keyword fround in 'txt'
        
        .. note::
            This function retrieves the corresponding value after running :meth:`findMaximal`
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        count: :class:`int`
            The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``1``
        
        Raises
        ------
        :class:`KeyError`
            If no keywords are found
        
        Returns
        -------
        Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[T, Any, List[T]]]
            Retrieves the following resultant data: :raw-html:`<br />` :raw-html:`<br />`
        
            * If the 'count' argument is less than or equal to 1, then the data contains:
        
                #. The first largest keyword found
                #. Either the found value for the first largest keyword found or the value specified at 'default', if no keywords were found and
                'errorOnNotFound' is set to ``False``
        
            * If the 'count' argument is greater than 1, then the data contains:
        
                #. The list of keywords found
                #. The corresponding found values to the keywords
        """
    def maximalStartsWith(self, txt: str) -> str | None:
        """
        Finds the largest keyword that is a prefix of the search text
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search keywords
        
        Returns 
        -------
        Optional[:class:`str`]
            The keyword that is found to be the prefix of the search text, if available
        """
    @property
    def handleDuplicate(self) -> collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]:
        """
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
        
        The function takes in the following parameters:
        
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
        
        :getter: Retrieves the function
        :setter: Sets the new function
        :type: Callable[[:class:`str`, T, T], T]
        """
    @handleDuplicate.setter
    def handleDuplicate(self, arg1: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]) -> None:
        ...
class CppAlgo:
    """
    C++ Tools for handling algorithm operations
    """
    @staticmethod
    def binarySearch(lst: collections.abc.Sequence[typing.Any], target: typing.Any, compare: collections.abc.Callable) -> tuple[bool, int]:
        """
        Performs a binary search for the target element in a sorted list.
        
        Parameters
        ----------
        lst: List[T]
            The sorted list to search.
        
        target: T
            The target element to search for.
        
        compare: Callable[[T, T], int]
            The compare function used to compare list elements with the target.
        
        Returns
        -------
        Tuple[bool, int]
            A tuple where the first value indicates whether the target was found,
            and the second value is the index of the found element or insertion point.
        """
    @staticmethod
    def merge(sorted_lsts: collections.abc.Sequence[collections.abc.Sequence[typing.Any]], compare: collections.abc.Callable) -> list:
        """
        Merges multiple sorted lists into one sorted list.
        
        Parameters
        ----------
        sorted_lsts: List[List[T]]
            The sorted lists to merge.
        
        compare: Callable[[T, T], int]
            The compare function used to order list elements.
        
        Returns
        -------
        List[T]
            The merged list of all input elements in sorted order.
        """
class CppIfContentPart(CppIfTemplatePart):
    """
    
    This class inherits from :class:`CppIfTemplatePart`
    
    The content part of an `IfTemplate` -- the C++ port of the deprecated Python
    ``IfContentPart``, holding the key-value pairs (e.g. a `.ini` section's registers) for one part
    of the template.
    
    Unlike the deprecated version, this class owns its data purely through a caller-supplied
    :class:`IOrderedMultiMap` implementation -- pick which concrete ordered-multimap backs a given
    :class:`CppIfContentPart` (:class:`CppOrderedMultiMap`/:class:`CppOrderedMultiMapSqrt` via their
    ``asInterface()`` method, or any custom :class:`IOrderedMultiMap` implementation of your own,
    including one implemented from Python), and every method on this class is a thin, renamed
    delegation straight to that implementation -- the semantics for every operation are exactly
    :class:`CppOrderedMultiMap`'s documented rules, not the deprecated Python class's old ones; only
    the *method names* below intentionally echo the old class's naming (e.g. ``insertAllAt`` ->
    ``addKVPsByInds``).
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines if 'key' exists in the content part
    
        .. describe:: len(x)
    
            Retrieves the number of KVPs in the content part
    
        .. describe:: x[index]
    
            Retrieves the ``(key, value)`` pair at the given true positional index, if ``index`` is
            an :class:`int`
    
        .. describe:: x[key]
    
            Retrieves every value currently stored under ``key``, in true positional order
            (equivalent to :meth:`getVals` with ``ordered=True``), if ``key`` is a :class:`str` --
            raises :class:`KeyError` if ``key`` doesn't exist
    
        .. describe:: iter(x)
    
            Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex,
            orderIndex)`` tuples
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        deep copy (equivalent to ``x.clone()``)
    
    Parameters
    ----------
    src: Optional[Dict[Any, List[Tuple[:class:`int`, Any]]]]
        Initial data to populate ``content`` with, as key -> list of ``(index, value)`` pairs, one
        entry per occurrence of that key :raw-html:`<br />` :raw-html:`<br />`
    
        ``index`` orders every occurrence relative to every other occurrence *across all keys*
        (gathered, stable-sorted by ``index`` ascending, then appended in that order); it is **not**
        a strict absolute position, so gaps and duplicate/out-of-order values are fine. If
        ``content`` already holds data (a pre-populated instance was passed in rather than left to
        default), ``src``'s entries are appended after it, not merged/interleaved with it.
        :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning no initial data is inserted
    
    depth: :class:`int`
        The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``0``
    
    content: Optional[:class:`IOrderedMultiMap`]
        The backing ordered-multimap implementation to use, taken by ownership -- see this class's
        top-level warning about what that means for 'content' afterward :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used
            
    """
    @staticmethod
    def buildFromOrder(src: typing.Any, depth: typing.SupportsInt | typing.SupportsIndex = 0, content: typing.Any = None) -> CppIfContentPart:
        """
        Creates a new part, populated from a flat, already-ordered list of key-value pairs -- a thin
        convenience over default-constructing then calling :meth:`addKVPs`
        
        Parameters
        ----------
        src: List[Tuple[Any, Any]]
            The key-value pairs to populate ``content`` with, appended in the order given (``src[0]``
            ends up first, right after whatever ``content`` already held, and so on) -- a key may repeat
            here directly, e.g. ``[("a", "1"), ("b", "2"), ("a", "3")]``
        
        depth: :class:`int`
            The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``0``
        
        content: Optional[:class:`IOrderedMultiMap`]
            The backing ordered-multimap implementation to use, taken by ownership -- see
            :class:`CppIfContentPart`'s top-level warning about what that means for 'content' afterward
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used
        
        Returns
        -------
        :class:`CppIfContentPart`
            The newly-created part
        """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> CppIfContentPart:
        """
        Creates a copy of this part (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> CppIfContentPart:
        """
        Creates a deep copy of this part (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    @typing.overload
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    @typing.overload
    def __getitem__(self, key: str) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key, in true positional order (equivalent to :meth:`getVals` with ``ordered=True``); raises :class:`KeyError` if the key doesn't exist
        """
    def __init__(self, src: typing.Any = None, depth: typing.SupportsInt | typing.SupportsIndex = 0, content: typing.Any = None) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of KVPs
        """
    def addKVP(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a KVP to the end
        """
    def addKVPAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a KVP so it ends up at position 'index' (0-based); see :meth:`CppOrderedMultiMap.insertAt` for the full index semantics
        """
    def addKVPToFront(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a KVP at the beginning
        """
    def addKVPs(self, kvps: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of KVPs to the end, in the order given
        """
    def addKVPsByInds(self, kvps: dict, sortIndices: bool = True, ranges: typing.Any = None) -> int:
        """
        Bulk indexed insert of KVPs; see :meth:`CppOrderedMultiMap.insertAllAt` for the full semantics
        """
    def addKVPsToFront(self, kvps: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of KVPs at the beginning, in the order given
        """
    def clone(self) -> CppIfContentPart:
        """
        Creates a deep copy of this part, at the same depth
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many KVPs share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the part has no KVPs
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value)`` pairs
        """
    @typing.overload
    def get(self, key: typing.SupportsInt | typing.SupportsIndex, errorOnNotFound: bool = False, default: typing.Any = None, ordered: bool = True, withInds: bool = False, ranges: typing.Any = None) -> typing.Any:
        """
        Retrieves the ``(key, value)`` pair at a true positional index (if ``key`` is an :class:`int`) or
        every value currently stored under a key (if ``key`` is a :class:`str`) -- like :meth:`__getitem__`,
        except not finding anything is configurable instead of always raising.
        
        Parameters
        ----------
        key: Union[:class:`int`, :class:`str`]
            The true positional index or key to look up
        
        errorOnNotFound: :class:`bool`
            If ``True`` and nothing is found, raises :class:`KeyError` (``key`` was a :class:`str`) or
            :class:`IndexError` (``key`` was an :class:`int`, out of range). If ``False`` (the default),
            returns ``default`` instead of raising.
        
        default: Any
            The value returned when nothing is found and ``errorOnNotFound`` is ``False`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        ordered: :class:`bool`
            Only takes effect when ``key`` is a :class:`str` -- same purpose as ``ordered`` from
            :meth:`CppOrderedMultiMap.getAll` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        withInds: :class:`bool`
            Only takes effect when ``key`` is a :class:`str` -- if ``True``, each returned value is
            paired with its true positional index (equivalent to :meth:`getValsWithInds`); if ``False``
            (the default), values are returned bare (equivalent to :meth:`getVals`)
        
        ranges: Optional[:class:`Ranges`]
            Only takes effect when ``key`` is a :class:`str` -- if provided, only occurrences whose true
            positional index (same convention as :meth:`getByInd`) falls within ``ranges`` are
            considered :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is considered
        
        Returns
        -------
        Any
            The result described above, or ``default`` if nothing was found and ``errorOnNotFound`` is
            ``False``
        """
    @typing.overload
    def get(self, key: str, errorOnNotFound: bool = False, default: typing.Any = None, ordered: bool = True, withInds: bool = False, ranges: typing.Any = None) -> typing.Any:
        """
        Same as the ``int``-keyed overload above, for a :class:`str` ``key`` -- see its docstring for
        the full parameter descriptions
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the KVP at a true positional index, paired with its occurrence index
        """
    def getKeys(self) -> set[typing.Any]:
        """
        Retrieves every distinct key currently in this part
        
        Returns
        -------
        Set[Any]
            Every distinct key, as a set (unordered)
        """
    def getVals(self, key: typing.Any, ordered: bool = True, ranges: typing.Any = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Any]
            The values for this key, in the requested order
        """
    def getValsWithInds(self, key: typing.Any, ordered: bool = True, ranges: typing.Any = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index
        (equivalent to :meth:`getVals`, except each value is paired with its true positional index)
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Tuple[:class:`int`, Any]]
            The ``(index, value)`` pairs for this key, in the requested order
        """
    def items(self) -> list[tuple[typing.Any, typing.Any, int, int]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of KVPs
        """
    def remapKeys(self, keyRemap: dict, ranges: typing.Any = None) -> None:
        """
        Bulk-renames keys; see :meth:`CppOrderedMultiMap.remapKeys` for the full semantics
        """
    def removeKVPAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: typing.Any = None) -> bool:
        """
        Removes the KVP currently at position 'pos'
        """
    def removeKey(self, key: typing.Any, ranges: typing.Any = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every KVP with this key, subject to two independent, optional filters -- both must hold
        (where provided) for a given occurrence to actually be removed. With neither filter provided,
        this is unconditional removal of every KVP with this key.
        
        Parameters
        ----------
        key: Any
            The key whose KVPs to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges'
        
        check: Optional[Callable[[int, Any], bool]]
            If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
            positional index and value
        
        Returns
        -------
        :class:`int`
            How many KVPs were actually removed
        """
    def removeKeys(self, keys: dict, ranges: typing.Any = None) -> int:
        """
        Removes multiple, independently-specified keys -- a thin loop over :meth:`removeKey`, sharing
        one 'ranges' filter across all of them.
        
        Parameters
        ----------
        keys: Dict[Any, Optional[Callable[[int, Any], bool]]]
            Each key to remove, mapped to its own optional check predicate -- ``check(index, value)``
            must return ``True`` for a given occurrence of that key to actually be removed, if provided;
            if omitted, every occurrence of that key is removed unconditionally (subject to 'ranges'
            below)
        
        ranges: Optional[:class:`Ranges`]
            If provided, an occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges' for it to be eligible for removal, for every key in 'keys'
        
        Returns
        -------
        :class:`int`
            How many KVPs were actually removed, summed across every key in 'keys'
        """
    def reorder(self, orderMap: dict, ranges: typing.Any = None) -> None:
        """
        Reorders existing KVPs in place; see :meth:`CppOrderedMultiMap.reorder` for the full semantics
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: typing.Any = None) -> None:
        """
        Bulk-updates values by key; see :meth:`CppOrderedMultiMap.replaceVals` for the full semantics
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the KVP at a true positional index, leaving its key untouched
        """
    def size(self) -> int:
        """
        Retrieves the number of KVPs
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[CppIfContentPart]:
        """
        Splits this part into several smaller parts at the given indices, each at the same depth as
        this part; see :meth:`CppOrderedMultiMap.splitByInds` for the full semantics
        
        Returns
        -------
        List[:class:`CppIfContentPart`]
            The resulting parts, left to right
        """
    def toStr(self, linePrefix: str = '') -> str:
        """
        Retrieves the part as a string, one ``key = value`` line per KVP in true positional order
        
        Parameters
        ----------
        linePrefix: :class:`str`
            The string that will prefix every line :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The string representation of the part
        """
    @property
    def content(self) -> IOrderedMultiMap:
        """
        :class:`IOrderedMultiMap`: The backing ordered-multimap implementation directly, for operations this class doesn't itself wrap
        """
    @property
    def depth(self) -> int:
        """
        :class:`int`: The depth this part is within the owning `IfTemplate`
        """
    @depth.setter
    def depth(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CppIfTemplatePart:
    """
    
    Base class for some part in an `IfTemplate`
            
    """
    def __init__(self) -> None:
        ...
class CppIntTools:
    """
    C++ Tools for handling integers
    """
    @staticmethod
    def toBase(num: typing.SupportsInt | typing.SupportsIndex, base: typing.SupportsInt | typing.SupportsIndex) -> tuple[list[int], bool]:
        """
                                Converts a base 10 number to an arbitrary base number
        
                                Parameters
                                ----------
                                num: :class:`int`
                                    The base 10 number to convert
        
                                base: :class:`int`
                                    The base to convert to
        
                                Raises
                                ------
                                :class:`TypeError`
                                    The base is smaller or equal to 1
        
                                Returns
                                -------
                                Tuple[List[:class:`int`], :class:`bool`]
                                    Retrieves the following data in the tuple:
        
                                    #. The digits in the converted number
                                    #. Whether the number is negative
        """
    @staticmethod
    def toBase64(num: typing.SupportsInt | typing.SupportsIndex, getDigit: collections.abc.Sequence[str] | None = None, negativeChar: str = '-') -> str:
        """
        Converts a base 10 number to a base 64 number
        
        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert
        
        getDigit: List[:class:`str`]
            how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`
        
            * If this argument is a list, each element is the string representation of the digit at the particular index of the string/list.
            * If this argument is ``None``, then will use the following string for each digit:
        
            ``ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_``
        
            This is the same digit representation as the `standard base 64`_ except that the 63rd digit (``/``) is replaced with the ``_`` character :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        negativeChar: :class:`str`
            The character representation for the negative symbol :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``"-"``
        
        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base 64 number
        """
    @staticmethod
    def toStrBase(num: typing.SupportsInt | typing.SupportsIndex, base: typing.SupportsInt | typing.SupportsIndex, getDigit: collections.abc.Sequence[str], negativeChar: str) -> str:
        """
        Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
        are all characters
        
        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert
        
        base: :class:`int`
            The base to convert to
        
        getDigit: List[:class:`str`]
            The string representations of each digit. Each element is the string representation
            of the digit at the particular index of the list.
        
        negativeChar: :class:`str`
            The character representation for the negative symbol
        
        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base number
        """
class CppListTools:
    """
    C++ Tools for handling with Lists
    """
    @staticmethod
    def addLstsByInds(lst: list, subLsts: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, list]) -> list:
        """
                                Inserts multiple sublists into the main list by index
        
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
    @staticmethod
    def getIndsAfterRemove(removedInds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], lstLen: typing.SupportsInt | typing.SupportsIndex) -> list[int]:
        """
                                Retrieve the index shifts in some data structure,
                                after the list got elements removed by indices
        
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
    @staticmethod
    def removeByInds(lst: list, inds: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> list:
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
    @staticmethod
    def removeParts(lst: list, part_indices: list) -> list:
        """
                                Removes many indices from a list
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The desired list to have its parts removed
        
                                inds: Set[:class:`int`]
                                    The indices to the elements in the list that needs to be removed
        
                                Returns
                                -------
                                List[T]
                                    The new list with elements specified by indices removed
        """
class CppTrie:
    """
    
    A class for a basic `trie`_ implemented in C++
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines if 'key' is found
    
        .. describe:: x[key]
    
            Retrieves the corresponding value to 'key'
    
        .. describe:: x[key] = val
    
            Sets the new `KVP`_
    
        .. describe:: len(x)
    
            Retrieves the number of elements
    
    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the keywords to put into the `trie`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    handleDuplicate: Optional[Callable[[:class:`str`, T, T], T]]
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
    
        The function takes in the following parameters:
    
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
    
        If this value is ``None``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __contains__(self, key: str) -> bool:
        """
        Determines if 'key' is found
        """
    def __getitem__(self, key: str) -> typing.Any:
        """
        Retrieves the corresponding value to 'key'
        """
    def __init__(self, data: collections.abc.Mapping[str, typing.Any] | None = None, handleDuplicate: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any] | None = None) -> None:
        ...
    def __len__(self) -> int:
        """
        Retrieves the number of elements
        """
    def __setitem__(self, key: str, val: typing.Any) -> bool:
        """
        Sets the new `KVP`_
        """
    def add(self, keyword: str, value: typing.Any) -> bool:
        """
        Adds a new keyword
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add
        
        value: T
            The value associated with the keyword
        
        Returns
        -------
        :class:`bool`
            Whether the keyword has already been inserted
        """
    def build(self, data: collections.abc.Mapping[str, typing.Any] | None = None) -> None:
        """
        Rebuilds the `trie`_
        
        Parameters
        ----------
        data: Optional[Dict[:class:`str`, T]]
            Any initial data to put into the `trie`_ :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords to put into the trie and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears the data
        """
    def get(self, keyword: str, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding value to 'keyword'
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to get the corresponding value for
        
        errorOnNotFound: :class:`bool`  
            If the keyword is not found, whether to raise an exception
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if 'keyword' is not found
        
        Raises
        ------
        :class:`KeyError`
            If 'keyword' is not found
        
        Returns
        -------
        Union[T, Any]
            Either the found value for the keyword or the value specified at 'default', if 'keyword' is not found and
            'errorOnNotFound' is set to ``False``
        """
    @property
    def handleDuplicate(self) -> collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]:
        """
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
        
        The function takes in the following parameters:
        
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
        
        :getter: Retrieves the function
        :setter: Sets the new function
        :type: Callable[[:class:`str`, T, T], T]
        """
    @handleDuplicate.setter
    def handleDuplicate(self, arg1: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]) -> None:
        ...
class DFA(BaseDFA):
    """
    
    Class for a `DFA (Deterministic Finite Automaton)`_
            
    """
    def __init__(self) -> None:
        ...
    def addFuncTransition(self, srcId: typing.Any, func: collections.abc.Callable, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_ such that the transition is based off a predicate function
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        func: Callable[[Hashable], :class:`bool`]
            The predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            The function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addKeywordTransition(self, srcId: typing.Any, keyword: typing.Any, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_ such that the transition is based off a keyword
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keyword: `Hashable`_
            The keyword or predicate function that will trigger a transition from the source state to the destination state
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addTransition(self, srcId: typing.Any, keyword: typing.Any, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keyword: Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]
            The keyword or predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            If keyword is a predicate function, the function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addTransitions(self, srcId: typing.Any, keywords: typing.Any, destId: typing.Any) -> None:
        """
        Adds a group of transitions from one state to another state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keywords: Union[List[Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]], `Hashable`_, Callable[[Hashable], :class:`bool`]]
            The keywords or predicate functions that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            For predicate functions, the function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def transition(self, keyword: typing.Any) -> tuple[typing.Any, bool, bool]:
        """
        Transitions to a new state
        
        Parameters
        ----------
        keyword: Hashable
            The keyword to trigger the transition to the new state
        
        Returns
        -------
        Tuple[Hashable, :class:`bool`, :class:`bool`]
            Resultant data regarding the new transitioned state, which includes:
        
            #. The id of the new state
            #. Whether the new state is an accepting state
            #. Whether a transition was taken 
        """
    @property
    def currentStateId(self) -> typing.Any | None:
        """
        The id of the state the `DFA`_ is currently at
        
        .. warning::
            The setter will not set the new id for the state if the newly current id does not correspond
            to any state within the `DFA`_
        
        :getter: Retrieves the id of the current state
        :setter: Sets the new id of the current state the `DFA`_ is on
        :type: Hashable
        """
    @currentStateId.setter
    def currentStateId(self, arg1: typing.Any) -> None:
        ...
    @property
    def startId(self) -> typing.Any | None:
        """
        The id to the start state
        
        .. warning::
            The setter will not set the new id for the state if the newly given start id does not correspond
            to any state within the `DFA`_
        
        :getter: Retrieves the start id
        :setter: Sets the new start id
        :type: Hashable
        """
    @startId.setter
    def startId(self, arg1: typing.Any) -> None:
        ...
class IOrderedMultiMap:
    """
    
    An abstract ordered-multimap interface: implement every method below (in a Python subclass) to
    plug an entirely custom backing structure into any C++ code that accepts this interface --
    :class:`OrderedMultiMap` and :class:`OrderedMultiMapSqrt` are two such implementations,
    each exposed as an :class:`IOrderedMultiMap` via their own ``asInterface()`` method.
    
    .. note::
        Unlike :class:`OrderedMultiMap`, the ``ranges`` parameter accepted throughout this
        interface takes either a bound :class:`Ranges` instance or a plain
        ``List[Tuple[Optional[int], Optional[int]]]`` of ``(start, end)`` bounds -- a Python
        subclass's own override of a ``ranges``-taking method always receives the latter, plain
        shape.
            
    """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> IOrderedMultiMap:
        """
        Creates a copy of this instance (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IOrderedMultiMap:
        """
        Creates a deep copy of this instance (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def __init__(self) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of entries
        """
    def clone(self) -> IOrderedMultiMap:
        """
        Creates a deep copy of this instance
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many entries share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the map is empty
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value)`` pairs
        """
    def getAll(self, key: typing.Any, ordered: bool = True, ranges: typing.Any = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key; see :meth:`CppOrderedMultiMap.getAll` for the
        full semantics
        """
    def getAllWithInds(self, key: typing.Any, ordered: bool = True, ranges: typing.Any = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index;
        see :meth:`CppOrderedMultiMap.getAllWithInds` for the full semantics
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the entry at a true positional index, paired with its occurrence index
        """
    def getKeys(self) -> list[typing.Any]:
        """
        Retrieves every distinct key currently in the map, as a :class:`list` rather than a real
        ``set`` -- unlike :meth:`CppOrderedMultiMap.getKeys`, this interface has no way to guarantee an
        arbitrary key type is hashable
        """
    def insert(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a key-value pair to the end
        """
    def insertAllAt(self, items: dict, sortIndices: bool = True, ranges: typing.Any = None) -> int:
        """
        Bulk indexed insert; see :meth:`OrderedMultiMap.insertAllAt` for the full semantics
        """
    def insertAllEnd(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of key-value pairs to the end, in the order given
        """
    def insertAllStart(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of key-value pairs at the beginning, in the order given
        """
    def insertAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair so it ends up at position 'index' (0-based); see :meth:`OrderedMultiMap.insertAt` for the full index semantics
        """
    def insertStart(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair at the beginning
        """
    def items(self) -> list[tuple[typing.Any, typing.Any, int, int]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of entries
        """
    def remapKeys(self, keyRemap: dict, ranges: typing.Any = None) -> None:
        """
        Bulk-renames keys; see :meth:`OrderedMultiMap.remapKeys` for the full semantics
        """
    def removeAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: typing.Any = None) -> bool:
        """
        Removes the entry currently at position 'pos'
        """
    def removeKey(self, key: typing.Any, ranges: typing.Any = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every entry with this key, subject to optional 'ranges'/'check' filters; see
        :meth:`OrderedMultiMap.removeKey` for the full semantics -- 'check', if provided, is
        ``check(index, value)``
        """
    def reorder(self, orderMap: dict, ranges: typing.Any = None) -> None:
        """
        Reorders existing entries in place; see :meth:`OrderedMultiMap.reorder` for the full semantics
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: typing.Any = None) -> None:
        """
        Bulk-updates values by key; see :meth:`OrderedMultiMap.replaceVals` for the full semantics
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the entry at a true positional index, leaving its key untouched
        """
    def size(self) -> int:
        """
        Retrieves the number of entries
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[IOrderedMultiMap]:
        """
        Splits this map into several smaller maps at the given indices; see :meth:`OrderedMultiMap.splitByInds` for the full semantics
        """
class IfContentPartColourChange:
    """
    
    Class to store the change in state of a particular key for a :class:`IfContentPartColouring`
    
    Parameters
    ----------
    old: Optional[Any]
        The old value of a particular key -- either a plain value (the key's value came from some
        previous :class:`CppIfContentPart`), or a ``List[Tuple[int, Any]]`` (the key's values come
        from the current :class:`CppIfContentPart`, each paired with its index of occurrence) :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning the key didn't exist beforehand
            
    """
    @staticmethod
    def restore(*args, **kwargs) -> None:
        """
        Restores the old value for a particular key within ``colouring``
        
        Parameters
        ----------
        colouring: :class:`IfContentPartColouring`
            The colouring to restore a value within
        
        key: Any
            The key to restore -- if ``key`` isn't currently in ``colouring``, this has no effect
        """
    def __copy__(self) -> IfContentPartColourChange:
        """
        Creates a copy of this change record (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IfContentPartColourChange:
        """
        Creates a copy of this change record (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    def __init__(self, old: typing.Any = None) -> None:
        ...
    def clone(self) -> IfContentPartColourChange:
        """
        Creates a copy of this change record
        """
    @property
    def old(self) -> typing.Any:
        """
        Optional[Any]: The old value of a particular key
        """
    @old.setter
    def old(self, arg1: typing.Any) -> None:
        ...
class IfContentPartColouring:
    """
    
    Class that keeps track of the current state of the `KVPs`_ within a :class:`CppIfContentPart` --
    the C++-backed port of the deprecated pure-Python ``IfContentPartColouringOld``
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines if 'key' currently has a tracked state
    
        .. describe:: len(x)
    
            Retrieves the number of keys currently tracked
    
        .. describe:: x[key]
    
            Retrieves the current state for 'key'; raises :class:`KeyError` if not tracked
    
        .. describe:: x[key] = value
    
            Sets the current state for 'key'
    
        .. describe:: del x[key]
    
            Removes the current state for 'key'; raises :class:`KeyError` if not tracked
    
        .. describe:: iter(x)
    
            Iterates every currently-tracked key, in insertion order
    
    :raw-html:`<br />` :raw-html:`<br />`
    
    * The keys are the names of the register keys
    * The values are either:
    
        * A plain value, indicating the value of the `KVP`_ comes from some previous :class:`CppIfContentPart`, OR
        * A ``List[Tuple[int, Any]]``. The list indicates that the values of the corresponding key
          come from the current :class:`CppIfContentPart`, each tuple containing the new state value
          for the corresponding key and its index of occurrence within the current part
    
    Parameters
    ----------
    src: Optional[Dict[Any, Any]]
        Initial state to populate this colouring with, in the same key -> value shape described above :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' currently has a tracked state
        """
    def __copy__(self) -> IfContentPartColouring:
        """
        Creates a copy of this colouring (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IfContentPartColouring:
        """
        Creates a copy of this colouring (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    def __delitem__(self, key: typing.Any) -> None:
        """
        Removes the current state for 'key'; raises :class:`KeyError` if not tracked
        """
    def __getitem__(self, key: typing.Any) -> typing.Any:
        """
        Retrieves the current state for 'key'; raises :class:`KeyError` if not tracked
        """
    def __init__(self, src: typing.Any = None) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterates every currently-tracked key, in insertion order
        """
    def __len__(self) -> int:
        """
        Retrieves the number of keys currently tracked
        """
    def __setitem__(self, key: typing.Any, value: typing.Any) -> None:
        """
        Sets the current state for 'key', inserting it if not already tracked
        """
    def clear(self) -> None:
        """
        Removes every tracked key
        """
    def clone(self) -> IfContentPartColouring:
        """
        Creates a copy of this colouring
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether 'key' currently has a tracked state
        """
    def empty(self) -> bool:
        """
        Checks whether no keys are currently tracked
        """
    def erase(self, key: typing.Any) -> bool:
        """
        Removes the current state for 'key', if any; returns whether 'key' was actually tracked
        """
    def get(self, key: typing.Any, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the current state for 'key', or 'default' if not tracked
        """
    def getIndVals(self, key: typing.Any, filter: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, typing.Any], bool] | None = None) -> list[tuple[int | None, typing.Any]]:
        """
        Retrieves both the corresponding values and the index of where the value occurs
        
        .. note::
            Unlike :meth:`getVals`, ``filter`` is only ever applied when ``key``'s state comes from the
            current :class:`CppIfContentPart` (a list of indexed occurrences) -- a value carried over from
            a previous part is always returned unfiltered, as ``(None, value)``.
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`
        
            The predicate takes in the following parameters:
        
            #. The index the value appears in the current :class:`CppIfContentPart`. If this argument is
               ``None``, then the value was carried over from a previous part
            #. The corresponding value
        
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        List[Tuple[Optional[:class:`int`], Any]]
            Both the values and their index within the current :class:`CppIfContentPart`. Empty if ``key``
            isn't tracked.
        """
    def getRanges(self, keysExists: collections.abc.Mapping[typing.Any, bool] | None = None, keyFilters: collections.abc.Mapping[typing.Any, collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, typing.Any], bool]] | None = None, existsRequireAll: bool = True, filtersRequireAll: bool = True, globalRequireAll: bool = True, includeKeyDefs: bool = True) -> Ranges:
        """
        Retrieves the ranges of indices within the current part that satisfy specified conditions for each key
        
        Parameters
        ----------
        keysExists: Optional[Dict[Any, :class:`bool`]]
            Checks whether a key exists or does not exist :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the registers and the values are whether to check for the
            existence/non-existence of the register :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        keyFilters: Optional[Dict[Any, Callable[[Optional[:class:`int`], Any], :class:`bool`]]]
            The conditions to satisfy for each key :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the registers and the values are the predicates, taking the same
            parameters as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        existsRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy all existence checks at ``keysExists`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        filtersRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy all the predicates specified at ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        globalRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy checks in both ``keysExists`` and ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        includeKeyDefs: :class:`bool`
            Whether to include indices where the values for the keys specified at ``keysExists`` or
            ``keyFilters`` are being (re)defined :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        Returns
        -------
        :class:`Ranges`
            The valid ranges that satisfy the specified conditions
        """
    def getUniqueVals(self, key: typing.Any, filter: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, typing.Any], bool] | None = None) -> set[typing.Any]:
        """
        Same as :meth:`getVals`, except the result is deduplicated into a real ``set`` -- a departure from
        the deprecated Python source's own ``getVals(unique=True)``, split into its own method the same
        way :class:`CppIfContentPart` itself splits ``getVals``/``getKeys`` rather than returning a value
        whose type depends on an argument
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            Same meaning as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Set[Any]
            The resultant unique values. Empty if ``key`` isn't tracked.
        """
    def getVals(self, key: typing.Any, filter: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, typing.Any], bool] | None = None) -> list[typing.Any]:
        """
        Retrieves the values for a given key, keeping duplicates and occurrence order
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            Same meaning as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        List[Any]
            The resultant values. Empty if ``key`` isn't tracked.
        """
    def items(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves every currently-tracked ``(key, state)`` pair, in insertion order
        """
    def keys(self) -> list[typing.Any]:
        """
        Retrieves every currently-tracked key, in insertion order
        """
    def restore(self, colourChange: collections.abc.Mapping[typing.Any, IfContentPartColourChange]) -> None:
        """
        Restores to a previous state
        
        Parameters
        ----------
        colourChange: Dict[Any, :class:`IfContentPartColourChange`]
            The change in the state, as returned by :meth:`updateColouring`
        """
    def set(self, key: typing.Any, value: typing.Any) -> None:
        """
        Sets the current state for 'key', inserting it if not already tracked
        """
    def size(self) -> int:
        """
        Retrieves the number of keys currently tracked
        """
    def updateColouring(self, ifContentPart: CppIfContentPart, targetKeys: collections.abc.Set[typing.Any] | None = None, updatePreviousKVPs: bool = True) -> dict[typing.Any, IfContentPartColourChange]:
        """
        Updates the current state of the `KVPs`_ based on the current :class:`CppIfContentPart`
        
        Parameters
        ----------
        ifContentPart: :class:`CppIfContentPart`
            The part to update the new `KVPs`_ from
        
        targetKeys: Optional[Set[Any]]
            The target keys to keep track of :raw-html:`<br />` :raw-html:`<br />`
        
            If this value is ``None``, then will keep track of all the keys :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        updatePreviousKVPs: :class:`bool`
            Whether to also update the `KVP`_ values from previous :class:`CppIfContentPart` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[Any, :class:`IfContentPartColourChange`]
            The change in the state :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the keys and the values are the state change for the keys
        """
class KeyRemapData:
    """
    
    A :meth:`OrderedMultiMap.remapKeys` ``keyRemap`` value (alongside a bare list of
    keys/:class:`RemappedKeyData`) that additionally controls what happens to an occurrence when
    none of its rules fire.
    
    Parameters
    ----------
    remappedKeys: List[Union[Any, :class:`RemappedKeyData`]]
        The remap rules for this key -- identical in meaning to passing a bare list directly
    
    keepKeyWithoutRemap: :class:`bool`
        If ``True``, an occurrence for which zero rules fired (an empty list, or every rule's
        ``check`` ``False``) retains its original ``(key, value)`` pair instead of being removed.
        Evaluated per occurrence :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``, matching a bare list's behavior: zero firings means removal
            
    """
    def __init__(self, remappedKeys: collections.abc.Sequence, keepKeyWithoutRemap: bool = False) -> None:
        ...
    @property
    def keepKeyWithoutRemap(self) -> bool:
        """
        :class:`bool`: Whether a non-firing occurrence retains its original pair
        """
class OrderedMultiMap:
    """
    
    An ordered multimap implemented in C++: preserves insertion/positional order, allows duplicate
    keys, and gives both fast key-based access and fast positional access. Backed by a plain linked
    list -- positional access walks from whichever end is closer to the requested index.
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines whether 'key' exists
    
        .. describe:: len(x)
    
            Retrieves the number of entries
    
        .. describe:: x[index]
    
            Retrieves the ``(key, value)`` pair at the given true positional index
    
        .. describe:: iter(x)
    
            Iterates every entry in true positional order, yielding ``(key, value,
            occurrenceIndex, orderIndex)`` tuples
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        deep copy (equivalent to ``x.copy()``)
    
    Parameters
    ----------
    items: Optional[List[Tuple[Any, Any]]]
        Key-value pairs to insert at the end, in order :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None`` (an empty instance)
            
    """
    @staticmethod
    def fromIndexed(indexed: dict) -> OrderedMultiMap:
        """
        Builds an instance from a fully-indexed description: for each key, a list of ``(index, value)``
        pairs. The index is treated as a sort key, not a strict absolute position: every ``(index, key,
        value)`` triple across every key is gathered, stable-sorted by index ascending, and inserted in
        that order -- gaps and out-of-order values just determine relative order, and duplicate indices
        land consecutively (tie-broken by encounter order: list order within a key, then 'indexed's own
        dict order across different keys).
        
        Parameters
        ----------
        indexed: Dict[Any, List[Tuple[:class:`int`, Any]]]
            The key -> list of ``(index, value)`` pairs to build from
        
        Returns
        -------
        :class:`OrderedMultiMap`
            The newly-built instance
        """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> OrderedMultiMap:
        """
        Creates a copy of this instance (equivalent to :meth:`copy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> OrderedMultiMap:
        """
        Creates a deep copy of this instance (equivalent to :meth:`copy`); supports ``copy.deepcopy()``
        """
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def __init__(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]] | None = None) -> None:
        ...
    def __iter__(self) -> OrderedMultiMapIterator:
        """
        Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of entries
        """
    def asInterface(self) -> ...:
        """
        Creates an independent snapshot of this instance, viewed through the generic
        :class:`IOrderedMultiMap` interface -- like :meth:`copy`, this is a deep copy; mutating the
        result does not affect this instance (or vice versa)
        
        Returns
        -------
        :class:`IOrderedMultiMap`
            An independent :class:`IOrderedMultiMap`-typed snapshot of this instance
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def copy(self) -> OrderedMultiMap:
        """
        Creates a deep copy of this instance -- rebuilt entry-by-entry, so the copy shares no internal
        state with the original
        
        Returns
        -------
        :class:`OrderedMultiMap`
            The newly-created copy
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many entries share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the map is empty
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves read-only access to the full ordered sequence
        
        Returns
        -------
        List[Tuple[Any, Any]]
            The full ordered sequence of ``(key, value)`` pairs
        """
    def getAll(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Any]
            The values for this key, in the requested order
        """
    def getAllWithInds(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index
        (equivalent to :meth:`getAll`, except each value is paired with its true positional index)
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Tuple[:class:`int`, Any]]
            The ``(order index, value)`` pairs for this key, in the requested order
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the entry at a true positional index
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[Any, Any]
            The ``(key, value)`` pair at that position
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the entry at a true positional index, paired with its occurrence index (how many
        times this same key already appeared earlier in the sequence, 0-based) instead of its key
        (equivalent to :meth:`getByInd`, except the entry's value is paired with its occurrence index)
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[:class:`int`, Any]
            The ``(occurrence index, value)`` pair at that position
        """
    def getKeys(self) -> set[typing.Any]:
        """
        Retrieves every distinct key currently in the map
        
        Returns
        -------
        Set[Any]
            Every distinct key, as a set (unordered)
        """
    def insert(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a key-value pair to the end
        """
    def insertAllAt(self, items: dict, sortIndices: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> int:
        """
        Bulk indexed insert: inserts many key-value pairs at their own target indices in a single pass.
        Index semantics match :meth:`insertAt` (Python-style negative indices, clamping), but with
        "original position" (numpy-style) semantics: each index refers to a position in the sequence as
        it was *before* this call, not a position in the growing result.
        
        Parameters
        ----------
        items: Dict[:class:`int`, Tuple[Any, Any]]
            Maps an index to insert at -> the key-value pair to insert there
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'items' is stable-sorted by normalized index first. If you
            already know 'items' iterates in ascending normalized-index order, pass ``False`` to skip
            that sort -- **this precondition is unchecked**, and violating it produces a silently wrong
            (not crashing) result
        
        ranges: Optional[:class:`Ranges`]
            If provided, an entry is only inserted when its normalized target index falls within
            'ranges'; filtered entries are dropped before sorting/the insertion pass :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually inserted
        """
    def insertAllEnd(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of key-value pairs to the end, in the order given
        """
    def insertAllStart(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of key-value pairs at the beginning, in the order given -- ``items[0]`` ends up
        first, ``items[1]`` right after it, and so on, all before whatever was originally at the front
        """
    def insertAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair so it ends up at position 'index' (0-based) :raw-html:`<br />` :raw-html:`<br />`
        
        Supports Python-style negative indices. Out-of-range indices are clamped rather than rejected:
        an index greater than ``len(self)`` is treated as ``len(self)`` (append); an index less than
        ``-(len(self) + 1)`` is treated as ``-(len(self) + 1)`` (front)
        
        Parameters
        ----------
        index: :class:`int`
            The target position
        
        key: Any
            The key of the pair to insert
        
        value: Any
            The value of the pair to insert
        """
    def insertStart(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair at the beginning
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of entries
        """
    def remapKeys(self, keyRemap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-renames keys. 'keyRemap' maps an old key -> either a bare list of keys/
        :class:`RemappedKeyData`, or a :class:`KeyRemapData`. :raw-html:`<br />` :raw-html:`<br />`
        
        For every existing entry, walked in true positional order: if its key is not a key in
        'keyRemap', it's left completely unchanged. Otherwise, each rule in the mapped list is evaluated
        independently against this occurrence -- a plain key always fires, a :class:`RemappedKeyData`
        fires if it has no ``check``, or ``check(oldKey, oldValue)`` is ``True``. Every rule that fires
        produces one new entry (that rule's key, this occurrence's original value); a
        :class:`RemappedKeyData` with ``toInd`` set instead moves its entry (as part of a group with
        every other entry sharing that same ``toInd`` across every occurrence) to that target index,
        using :meth:`reorder`'s exact index semantics. :raw-html:`<br />` :raw-html:`<br />`
        
        If zero rules fire for a given occurrence: with a bare list, or ``keepKeyWithoutRemap=False``,
        that occurrence is removed entirely. With ``keepKeyWithoutRemap=True`` (via
        :class:`KeyRemapData`), it retains its original ``(key, value)`` pair instead. :raw-html:`<br />` :raw-html:`<br />`
        
        Old keys mentioned in 'keyRemap' that don't actually exist right now are simply never
        triggered -- no error, nothing happens. This is a single pass over the original entries:
        newly-created entries are never looked up in 'keyRemap' again, so there's no cascading/recursive
        re-application.
        
        Parameters
        ----------
        keyRemap: Dict[Any, Union[List[Union[Any, :class:`RemappedKeyData`]], :class:`KeyRemapData`]]
            The old key -> remap rules mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an occurrence outside 'ranges' is treated exactly as if its key were never
            mentioned in 'keyRemap' at all -- a pure pass-through :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def removeAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: FixRaidenBoss2.core.Ranges | None = None) -> bool:
        """
        Removes the entry currently at position 'pos'
        
        Parameters
        ----------
        pos: :class:`int`
            The position of the entry to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, removal only proceeds when 'pos' falls within 'ranges'; otherwise this call is
            a no-op, same as an out-of-bounds 'pos' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether an entry was actually removed
        """
    def removeKey(self, key: typing.Any, ranges: FixRaidenBoss2.core.Ranges | None = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every entry with this key, subject to two independent, optional filters -- both must
        hold (where provided) for a given occurrence to actually be removed. With neither filter
        provided, this is unconditional removal of every entry with this key.
        
        Parameters
        ----------
        key: Any
            The key whose entries to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        check: Optional[Callable[[:class:`int`, Any], :class:`bool`]]
            If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
            positional index and value :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually removed
        """
    def reorder(self, orderMap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Reorders existing entries in place. 'orderMap' maps an old index -> new index for a subset (or
        all) of the current entries; every entry not mentioned keeps its relative order and fills
        whatever slots are left over :raw-html:`<br />` :raw-html:`<br />`
        
        **Old-index (key) semantics:** must be in ``[-len(self), len(self) - 1]`` -- anything outside
        that raises :class:`IndexError`. :raw-html:`<br />` :raw-html:`<br />`
        
        **New-index (value) semantics:** also Python-style, but out-of-range values are bucketed rather
        than rejected: a value ``>= len(self)`` goes in a trailing cluster at the very end, a value
        ``< -len(self)`` goes in a leading cluster at the very front, and within a cluster a smaller raw
        value sorts earlier. :raw-html:`<br />` :raw-html:`<br />`
        
        **Conflicts:** if two distinct entries of 'orderMap' target the same physical old entry, or the
        same effective new-index target, dict iteration order (Python 3.7+ insertion order) breaks the
        tie.
        
        Parameters
        ----------
        orderMap: Dict[:class:`int`, :class:`int`]
            The old index -> new index mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an 'orderMap' entry only takes effect when its old index falls within 'ranges';
            otherwise it's ignored entirely, and the old position it would have pinned floats instead :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-updates values by key. 'newVals' maps a key -> either a bare replacement value, a
        :class:`ReplaceList` (positional, by existing true left-to-right order), or a :class:`ReplaceIf`
        (conditional, by predicate).
        
        Parameters
        ----------
        newVals: Dict[Any, Union[Any, :class:`ReplaceList`, :class:`ReplaceIf`]]
            The key -> replace spec mapping to apply
        
        addNew: :class:`bool`
            What to do when a key in 'newVals' doesn't currently exist. If ``True`` (the default), it's
            added, appended at the end (a bare value -> one entry; :class:`ReplaceList` -> one entry per
            value, in order; :class:`ReplaceIf` -> one entry with just the value, predicate ignored
            since there's nothing existing to test it against). If ``False``, the key is skipped
            entirely; no error.
        
        ranges: Optional[:class:`Ranges`]
            If provided, gates whether an existing entry's value actually gets replaced, on top of
            whatever the spec itself already decides -- both must hold. Not consulted for 'addNew' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the entry at a true positional index, leaving its key untouched
        
        Parameters
        ----------
        index: :class:`int`
            The position to update. Python-style negative indices are supported
        
        value: Any
            The new value for that entry
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        """
    def size(self) -> int:
        """
        Retrieves the number of entries
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[OrderedMultiMap]:
        """
        Splits this map into several smaller maps at the given indices, preserving relative order both
        within each part and across parts. Each resulting part is a genuinely independent new instance --
        mutating one part never affects the original or any sibling part.
        
        'inds' uses the same index convention as :meth:`getByInd`. Each index becomes a boundary using
        standard slice semantics: everything before it goes in the earlier part, everything from it
        onward starts the next part.
        
        Parameters
        ----------
        inds: List[:class:`int`]
            The indices at which to split
        
        includeSplitKVP: :class:`bool`
            What happens to the entry at each split point. If ``True`` (the default), it starts the
            later part. If ``False``, it's dropped entirely, belonging to neither part
        
        includeEmptyParts: :class:`bool`
            Whether empty parts are included in the result or silently dropped :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'inds' is normalized, deduplicated, and sorted ascending first.
            If you already know 'inds' iterates in that exact order, pass ``False`` to skip that pass --
            **this precondition is unchecked**, and violating it produces a silently wrong (not
            crashing) result
        
        Raises
        ------
        :class:`IndexError`
            If an index in 'inds' is out of range
        
        Returns
        -------
        List[:class:`OrderedMultiMap`]
            The resulting parts, left to right
        """
class OrderedMultiMapIterator:
    """
    
    A forward iterator over a :class:`OrderedMultiMap`, yielding ``(key, value,
    occurrenceIndex, orderIndex)`` tuples in true positional order.
            
    """
    def __iter__(self) -> OrderedMultiMapIterator:
        ...
    def __next__(self) -> tuple[typing.Any, typing.Any, int, int]:
        ...
class OrderedMultiMapSqrt:
    """
    
    An ordered multimap implemented in C++: preserves insertion/positional order, allows duplicate
    keys, and gives both fast key-based access and fast positional access. Behaviorally
    interchangeable with :class:`OrderedMultiMap` -- backed instead by O(sqrt(n)) block
    decomposition, giving O(sqrt(n)) positional access (:meth:`getByInd`, :meth:`insertAt`,
    :meth:`removeAt`, etc.) instead of :class:`OrderedMultiMap`'s O(n) worst case for a middle
    index, at the cost of more rebalancing machinery underneath.
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines whether 'key' exists
    
        .. describe:: len(x)
    
            Retrieves the number of entries
    
        .. describe:: x[index]
    
            Retrieves the ``(key, value)`` pair at the given true positional index
    
        .. describe:: iter(x)
    
            Iterates every entry in true positional order, yielding ``(key, value,
            occurrenceIndex, orderIndex)`` tuples
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        deep copy (equivalent to ``x.copy()``)
    
    Parameters
    ----------
    items: Optional[List[Tuple[Any, Any]]]
        Key-value pairs to insert at the end, in order :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None`` (an empty instance)
            
    """
    @staticmethod
    def fromIndexed(indexed: dict) -> OrderedMultiMapSqrt:
        """
        Builds an instance from a fully-indexed description: for each key, a list of ``(index, value)``
        pairs. The index is treated as a sort key, not a strict absolute position: every ``(index, key,
        value)`` triple across every key is gathered, stable-sorted by index ascending, and inserted in
        that order -- gaps and out-of-order values just determine relative order, and duplicate indices
        land consecutively (tie-broken by encounter order: list order within a key, then 'indexed's own
        dict order across different keys).
        
        Parameters
        ----------
        indexed: Dict[Any, List[Tuple[:class:`int`, Any]]]
            The key -> list of ``(index, value)`` pairs to build from
        
        Returns
        -------
        :class:`OrderedMultiMapSqrt`
            The newly-built instance
        """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> OrderedMultiMapSqrt:
        """
        Creates a copy of this instance (equivalent to :meth:`copy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> OrderedMultiMapSqrt:
        """
        Creates a deep copy of this instance (equivalent to :meth:`copy`); supports ``copy.deepcopy()``
        """
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def __init__(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]] | None = None) -> None:
        ...
    def __iter__(self) -> OrderedMultiMapSqrtIterator:
        """
        Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of entries
        """
    def asInterface(self) -> ...:
        """
        Creates an independent snapshot of this instance, viewed through the generic
        :class:`IOrderedMultiMap` interface -- like :meth:`copy`, this is a deep copy; mutating the
        result does not affect this instance (or vice versa)
        
        Returns
        -------
        :class:`IOrderedMultiMap`
            An independent :class:`IOrderedMultiMap`-typed snapshot of this instance
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def copy(self) -> OrderedMultiMapSqrt:
        """
        Creates a deep copy of this instance -- rebuilt entry-by-entry, so the copy shares no internal
        state with the original
        
        Returns
        -------
        :class:`OrderedMultiMapSqrt`
            The newly-created copy
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many entries share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the map is empty
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves a copy of the full ordered sequence
        
        Returns
        -------
        List[Tuple[Any, Any]]
            The full ordered sequence of ``(key, value)`` pairs
        """
    def getAll(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Any]
            The values for this key, in the requested order
        """
    def getAllWithInds(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index
        (equivalent to :meth:`getAll`, except each value is paired with its true positional index)
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Tuple[:class:`int`, Any]]
            The ``(order index, value)`` pairs for this key, in the requested order
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the entry at a true positional index
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[Any, Any]
            The ``(key, value)`` pair at that position
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the entry at a true positional index, paired with its occurrence index (how many
        times this same key already appeared earlier in the sequence, 0-based) instead of its key
        (equivalent to :meth:`getByInd`, except the entry's value is paired with its occurrence index)
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[:class:`int`, Any]
            The ``(occurrence index, value)`` pair at that position
        """
    def getKeys(self) -> set[typing.Any]:
        """
        Retrieves every distinct key currently in the map
        
        Returns
        -------
        Set[Any]
            Every distinct key, as a set (unordered)
        """
    def insert(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a key-value pair to the end
        """
    def insertAllAt(self, items: dict, sortIndices: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> int:
        """
        Bulk indexed insert: inserts many key-value pairs at their own target indices in a single pass.
        Index semantics match :meth:`insertAt` (Python-style negative indices, clamping), but with
        "original position" (numpy-style) semantics: each index refers to a position in the sequence as
        it was *before* this call, not a position in the growing result.
        
        Parameters
        ----------
        items: Dict[:class:`int`, Tuple[Any, Any]]
            Maps an index to insert at -> the key-value pair to insert there
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'items' is stable-sorted by normalized index first. If you
            already know 'items' iterates in ascending normalized-index order, pass ``False`` to skip
            that sort -- **this precondition is unchecked**, and violating it produces a silently wrong
            (not crashing) result
        
        ranges: Optional[:class:`Ranges`]
            If provided, an entry is only inserted when its normalized target index falls within
            'ranges'; filtered entries are dropped before sorting/the insertion pass :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually inserted
        """
    def insertAllEnd(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of key-value pairs to the end, in the order given
        """
    def insertAllStart(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of key-value pairs at the beginning, in the order given -- ``items[0]`` ends up
        first, ``items[1]`` right after it, and so on, all before whatever was originally at the front
        """
    def insertAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair so it ends up at position 'index' (0-based) :raw-html:`<br />` :raw-html:`<br />`
        
        Supports Python-style negative indices. Out-of-range indices are clamped rather than rejected:
        an index greater than ``len(self)`` is treated as ``len(self)`` (append); an index less than
        ``-(len(self) + 1)`` is treated as ``-(len(self) + 1)`` (front)
        
        Parameters
        ----------
        index: :class:`int`
            The target position
        
        key: Any
            The key of the pair to insert
        
        value: Any
            The value of the pair to insert
        """
    def insertStart(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair at the beginning
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of entries
        """
    def remapKeys(self, keyRemap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-renames keys. 'keyRemap' maps an old key -> either a bare list of keys/
        :class:`CppRemappedKeyData`, or a :class:`CppKeyRemapData`. :raw-html:`<br />` :raw-html:`<br />`
        
        For every existing entry, walked in true positional order: if its key is not a key in
        'keyRemap', it's left completely unchanged. Otherwise, each rule in the mapped list is evaluated
        independently against this occurrence -- a plain key always fires, a :class:`CppRemappedKeyData`
        fires if it has no ``check``, or ``check(oldKey, oldValue)`` is ``True``. Every rule that fires
        produces one new entry (that rule's key, this occurrence's original value); a
        :class:`CppRemappedKeyData` with ``toInd`` set instead moves its entry (as part of a group with
        every other entry sharing that same ``toInd`` across every occurrence) to that target index,
        using :meth:`reorder`'s exact index semantics. :raw-html:`<br />` :raw-html:`<br />`
        
        If zero rules fire for a given occurrence: with a bare list, or ``keepKeyWithoutRemap=False``,
        that occurrence is removed entirely. With ``keepKeyWithoutRemap=True`` (via
        :class:`CppKeyRemapData`), it retains its original ``(key, value)`` pair instead. :raw-html:`<br />` :raw-html:`<br />`
        
        Old keys mentioned in 'keyRemap' that don't actually exist right now are simply never
        triggered -- no error, nothing happens. This is a single pass over the original entries:
        newly-created entries are never looked up in 'keyRemap' again, so there's no cascading/recursive
        re-application.
        
        Parameters
        ----------
        keyRemap: Dict[Any, Union[List[Union[Any, :class:`CppRemappedKeyData`]], :class:`CppKeyRemapData`]]
            The old key -> remap rules mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an occurrence outside 'ranges' is treated exactly as if its key were never
            mentioned in 'keyRemap' at all -- a pure pass-through :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def removeAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: FixRaidenBoss2.core.Ranges | None = None) -> bool:
        """
        Removes the entry currently at position 'pos'
        
        Parameters
        ----------
        pos: :class:`int`
            The position of the entry to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, removal only proceeds when 'pos' falls within 'ranges'; otherwise this call is
            a no-op, same as an out-of-bounds 'pos' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether an entry was actually removed
        """
    def removeKey(self, key: typing.Any, ranges: FixRaidenBoss2.core.Ranges | None = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every entry with this key, subject to two independent, optional filters -- both must
        hold (where provided) for a given occurrence to actually be removed. With neither filter
        provided, this is unconditional removal of every entry with this key.
        
        Parameters
        ----------
        key: Any
            The key whose entries to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        check: Optional[Callable[[:class:`int`, Any], :class:`bool`]]
            If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
            positional index and value :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually removed
        """
    def reorder(self, orderMap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Reorders existing entries in place. 'orderMap' maps an old index -> new index for a subset (or
        all) of the current entries; every entry not mentioned keeps its relative order and fills
        whatever slots are left over :raw-html:`<br />` :raw-html:`<br />`
        
        **Old-index (key) semantics:** must be in ``[-len(self), len(self) - 1]`` -- anything outside
        that raises :class:`IndexError`. :raw-html:`<br />` :raw-html:`<br />`
        
        **New-index (value) semantics:** also Python-style, but out-of-range values are bucketed rather
        than rejected: a value ``>= len(self)`` goes in a trailing cluster at the very end, a value
        ``< -len(self)`` goes in a leading cluster at the very front, and within a cluster a smaller raw
        value sorts earlier. :raw-html:`<br />` :raw-html:`<br />`
        
        **Conflicts:** if two distinct entries of 'orderMap' target the same physical old entry, or the
        same effective new-index target, dict iteration order (Python 3.7+ insertion order) breaks the
        tie.
        
        Parameters
        ----------
        orderMap: Dict[:class:`int`, :class:`int`]
            The old index -> new index mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an 'orderMap' entry only takes effect when its old index falls within 'ranges';
            otherwise it's ignored entirely, and the old position it would have pinned floats instead :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-updates values by key. 'newVals' maps a key -> either a bare replacement value, a
        :class:`ReplaceList` (positional, by existing true left-to-right order), or a :class:`ReplaceIf`
        (conditional, by predicate).
        
        Parameters
        ----------
        newVals: Dict[Any, Union[Any, :class:`ReplaceList`, :class:`ReplaceIf`]]
            The key -> replace spec mapping to apply
        
        addNew: :class:`bool`
            What to do when a key in 'newVals' doesn't currently exist. If ``True`` (the default), it's
            added, appended at the end (a bare value -> one entry; :class:`ReplaceList` -> one entry per
            value, in order; :class:`ReplaceIf` -> one entry with just the value, predicate ignored
            since there's nothing existing to test it against). If ``False``, the key is skipped
            entirely; no error.
        
        ranges: Optional[:class:`Ranges`]
            If provided, gates whether an existing entry's value actually gets replaced, on top of
            whatever the spec itself already decides -- both must hold. Not consulted for 'addNew' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the entry at a true positional index, leaving its key untouched
        
        Parameters
        ----------
        index: :class:`int`
            The position to update. Python-style negative indices are supported
        
        value: Any
            The new value for that entry
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        """
    def size(self) -> int:
        """
        Retrieves the number of entries
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[OrderedMultiMapSqrt]:
        """
        Splits this map into several smaller maps at the given indices, preserving relative order both
        within each part and across parts. Each resulting part is a genuinely independent new instance --
        mutating one part never affects the original or any sibling part.
        
        'inds' uses the same index convention as :meth:`getByInd`. Each index becomes a boundary using
        standard slice semantics: everything before it goes in the earlier part, everything from it
        onward starts the next part.
        
        Parameters
        ----------
        inds: List[:class:`int`]
            The indices at which to split
        
        includeSplitKVP: :class:`bool`
            What happens to the entry at each split point. If ``True`` (the default), it starts the
            later part. If ``False``, it's dropped entirely, belonging to neither part
        
        includeEmptyParts: :class:`bool`
            Whether empty parts are included in the result or silently dropped :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'inds' is normalized, deduplicated, and sorted ascending first.
            If you already know 'inds' iterates in that exact order, pass ``False`` to skip that pass --
            **this precondition is unchecked**, and violating it produces a silently wrong (not
            crashing) result
        
        Raises
        ------
        :class:`IndexError`
            If an index in 'inds' is out of range
        
        Returns
        -------
        List[:class:`OrderedMultiMapSqrt`]
            The resulting parts, left to right
        """
class OrderedMultiMapSqrtIterator:
    """
    
    A forward iterator over a :class:`OrderedMultiMapSqrt`, yielding ``(key, value,
    occurrenceIndex, orderIndex)`` tuples in true positional order.
            
    """
    def __iter__(self) -> OrderedMultiMapSqrtIterator:
        ...
    def __next__(self) -> tuple[typing.Any, typing.Any, int, int]:
        ...
class Ranges:
    """
    
    A class representing a collection of integer ranges
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: value in x
    
            Determines whether 'value' falls within any of the ranges
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same ranges
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different ranges
    
        .. describe:: x - y
    
            Computes the set difference between 'x' and 'y' (equivalent to 'x.difference(y)')
    
        .. describe:: ~x
    
            Computes the negation (complement) of 'x' (equivalent to 'x.negate()')
    
        .. describe:: x + y
    
            Computes the union of 'x' and 'y' (equivalent to 'x.union([y])')
    
        .. describe:: x += y
    
            Performs the union of 'x' with 'y', in place (equivalent to 'x.update([y])')
    
        .. describe:: x & y
    
            Computes the intersection of 'x' and 'y' (equivalent to 'x.intersect([y])')
    
        .. describe:: x &= y
    
            Performs the intersection of 'x' with 'y', in place (equivalent to 'x.intersectUpdate([y])')
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a deep copy (equivalent to 'x.deepcopy()')
    
    .. note::
        This constructor is overloaded. Instead of a list of range tuples, ``Ranges`` can also be constructed directly
        from a ``List[int]`` or a ``Set[int]``, in which case the resulting ranges cover exactly those values (equivalent
        to :meth:`createFromList` / :meth:`createFromSet`)
    
    Parameters
    ----------
    ranges: List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]
        The ranges to store :raw-html:`<br />` :raw-html:`<br />`
    
        Each range is a tuple containing the starting (inclusive) index and the ending (exclusive) index of the range :raw-html:`<br />` :raw-html:`<br />`
    
        If the starting index is ``None``, the range is unbounded towards -infinity. If the ending index is ``None``, the range is unbounded towards +infinity
    
    normalize: :class:`bool`
        Whether to normalize 'ranges' before storing it :raw-html:`<br />` :raw-html:`<br />`
    
        If ``True``, 'ranges' is sorted and any overlapping or touching ranges are merged, producing the minimal set of disjoint ranges :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
            
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def createEmpty() -> Ranges:
        """
        Creates a :class:`Ranges` with no ranges
        
        Returns
        -------
        :class:`Ranges`
            A new, empty instance
        """
    @staticmethod
    def createFromList(values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Ranges:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``[1, 2, 3, 5]`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: List[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFromSet(values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> Ranges:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: Set[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFull() -> Ranges:
        """
        Creates a :class:`Ranges` spanning from -infinity to +infinity
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing a single range from -infinity to +infinity
        """
    def __add__(self, other: Ranges) -> Ranges:
        """
        Computes the union of this instance with 'other' (equivalent to 'self.union([other])')
        """
    def __and__(self, other: Ranges) -> Ranges:
        """
        Computes the intersection of this instance with 'other' (equivalent to 'self.intersect([other])')
        """
    def __contains__(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        """
    def __copy__(self) -> Ranges:
        """
        Creates a copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> Ranges:
        """
        Creates a deep copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.deepcopy()``
        """
    def __eq__(self, other: Ranges) -> bool:
        """
        Determines whether 'self' and 'other' store the same ranges
        """
    def __iadd__(self, other: Ranges) -> Ranges:
        """
        Performs the union of this instance with 'other', in place (equivalent to 'self.update([other])')
        """
    def __iand__(self, other: Ranges) -> Ranges:
        """
        Performs the intersection of this instance with 'other', in place (equivalent to 'self.intersectUpdate([other])')
        """
    @typing.overload
    def __init__(self, ranges: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]], normalize: bool = True) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    def __invert__(self) -> Ranges:
        """
        Computes the negation (complement) of this instance (equivalent to :meth:`negate`)
        """
    def __ne__(self, other: Ranges) -> bool:
        """
        Determines whether 'self' and 'other' store different ranges
        """
    def __sub__(self, other: Ranges) -> Ranges:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        """
    def add(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Adds 'value' to the stored ranges, extending or merging existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' is already contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to add
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before adding 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate the insertion point efficiently). If that assumption doesn't hold, pass ``True``, or the
            result may be incorrect
        """
    def deepcopy(self) -> Ranges:
        """
        Creates a deep copy of this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance that is a deep copy of this one
        """
    def difference(self, other: Ranges) -> Ranges:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        
        Parameters
        ----------
        other: :class:`Ranges`
            The ranges to subtract from this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the values in ``self`` that are not in 'other'
        """
    def getOverlaps(self, ranges: collections.abc.Sequence[Ranges], requireAll: bool = True, normalizeSelf: bool = False, normalizeOthers: bool = False) -> Ranges:
        """
        Computes the overlap between this instance and a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to compute the overlap against
        
        requireAll: :class:`bool`
            When ``True`` (the default), the result is the intersection of ``self`` and *every* entry in 'ranges'
            (a value must fall within ``self`` and all of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            When ``False``, the result is the intersection of ``self`` and the *union* of 'ranges'
            (a value must fall within ``self`` and at least one of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the computed overlap
        """
    def has(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        
        Parameters
        ----------
        value: :class:`int`
            The value to check
        
        Returns
        -------
        :class:`bool`
            Whether 'value' is contained within any of the stored ranges
        """
    def intersect(self, ranges: collections.abc.Sequence[Ranges], normalizeSelf: bool = False, normalizeOthers: bool = False) -> Ranges:
        """
        Computes the intersection of this instance and a list of other :class:`Ranges` :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.getOverlaps(ranges, True, normalizeSelf, normalizeOthers)``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the intersection of ``self`` and every entry in 'ranges'
        """
    def intersectUpdate(self, ranges: collections.abc.Sequence[Ranges], normalizeSelf: bool = False, normalizeOthers: bool = False) -> None:
        """
        Performs the intersection of this instance with a list of other :class:`Ranges`, in place :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``self``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        """
    def isEmpty(self) -> bool:
        """
        Determines whether there are no stored ranges
        
        Returns
        -------
        :class:`bool`
            Whether there are no stored ranges
        """
    def isFull(self) -> bool:
        """
        Determines whether the stored ranges span from -infinity to +infinity
        
        .. note::
            This checks for a single stored range of ``(None, None)``. If this instance was constructed with ``normalize=False``, several ranges
            could collectively cover -infinity to +infinity without having been merged into one, in which case this returns ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the stored ranges span from -infinity to +infinity
        """
    def negate(self) -> Ranges:
        """
        Computes the negation (complement) of this instance :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``Ranges.createFull() - self``
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing every value not in ``self``
        """
    def remove(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Removes 'value' from the stored ranges, shrinking, splitting, or erasing existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' isn't contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to remove
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before removing 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate 'value' efficiently). If that assumption doesn't hold, pass ``True``, or the result may
            be incorrect
        """
    def union(self, ranges: collections.abc.Sequence[Ranges]) -> Ranges:
        """
        Computes the union of this instance with a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the union of ``self`` and 'ranges'
        """
    def update(self, ranges: collections.abc.Sequence[Ranges]) -> None:
        """
        Performs the union of this instance with a list of other :class:`Ranges`, in place
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        """
    @property
    def ranges(self) -> list[tuple[int | None, int | None]]:
        """
        List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]: The stored ranges
        """
    @ranges.setter
    def ranges(self, arg0: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]]) -> None:
        ...
class RangesInt:
    """
    
    A class representing a collection of integer ranges
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: value in x
    
            Determines whether 'value' falls within any of the ranges
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same ranges
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different ranges
    
        .. describe:: x - y
    
            Computes the set difference between 'x' and 'y' (equivalent to 'x.difference(y)')
    
        .. describe:: ~x
    
            Computes the negation (complement) of 'x' (equivalent to 'x.negate()')
    
        .. describe:: x + y
    
            Computes the union of 'x' and 'y' (equivalent to 'x.union([y])')
    
        .. describe:: x += y
    
            Performs the union of 'x' with 'y', in place (equivalent to 'x.update([y])')
    
        .. describe:: x & y
    
            Computes the intersection of 'x' and 'y' (equivalent to 'x.intersect([y])')
    
        .. describe:: x &= y
    
            Performs the intersection of 'x' with 'y', in place (equivalent to 'x.intersectUpdate([y])')
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a deep copy (equivalent to 'x.deepcopy()')
    
    .. note::
        This constructor is overloaded. Instead of a list of range tuples, ``Ranges`` can also be constructed directly
        from a ``List[int]`` or a ``Set[int]``, in which case the resulting ranges cover exactly those values (equivalent
        to :meth:`createFromList` / :meth:`createFromSet`)
    
    Parameters
    ----------
    ranges: List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]
        The ranges to store :raw-html:`<br />` :raw-html:`<br />`
    
        Each range is a tuple containing the starting (inclusive) index and the ending (exclusive) index of the range :raw-html:`<br />` :raw-html:`<br />`
    
        If the starting index is ``None``, the range is unbounded towards -infinity. If the ending index is ``None``, the range is unbounded towards +infinity
    
    normalize: :class:`bool`
        Whether to normalize 'ranges' before storing it :raw-html:`<br />` :raw-html:`<br />`
    
        If ``True``, 'ranges' is sorted and any overlapping or touching ranges are merged, producing the minimal set of disjoint ranges :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
            
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def createEmpty() -> RangesInt:
        """
        Creates a :class:`Ranges` with no ranges
        
        Returns
        -------
        :class:`Ranges`
            A new, empty instance
        """
    @staticmethod
    def createFromList(values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> RangesInt:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``[1, 2, 3, 5]`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: List[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFromSet(values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> RangesInt:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: Set[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFull() -> RangesInt:
        """
        Creates a :class:`Ranges` spanning from -infinity to +infinity
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing a single range from -infinity to +infinity
        """
    def __add__(self, other: RangesInt) -> RangesInt:
        """
        Computes the union of this instance with 'other' (equivalent to 'self.union([other])')
        """
    def __and__(self, other: RangesInt) -> RangesInt:
        """
        Computes the intersection of this instance with 'other' (equivalent to 'self.intersect([other])')
        """
    def __contains__(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        """
    def __copy__(self) -> RangesInt:
        """
        Creates a copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> RangesInt:
        """
        Creates a deep copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.deepcopy()``
        """
    def __eq__(self, other: RangesInt) -> bool:
        """
        Determines whether 'self' and 'other' store the same ranges
        """
    def __iadd__(self, other: RangesInt) -> RangesInt:
        """
        Performs the union of this instance with 'other', in place (equivalent to 'self.update([other])')
        """
    def __iand__(self, other: RangesInt) -> RangesInt:
        """
        Performs the intersection of this instance with 'other', in place (equivalent to 'self.intersectUpdate([other])')
        """
    @typing.overload
    def __init__(self, ranges: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]], normalize: bool = True) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    def __invert__(self) -> RangesInt:
        """
        Computes the negation (complement) of this instance (equivalent to :meth:`negate`)
        """
    def __ne__(self, other: RangesInt) -> bool:
        """
        Determines whether 'self' and 'other' store different ranges
        """
    def __sub__(self, other: RangesInt) -> RangesInt:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        """
    def add(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Adds 'value' to the stored ranges, extending or merging existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' is already contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to add
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before adding 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate the insertion point efficiently). If that assumption doesn't hold, pass ``True``, or the
            result may be incorrect
        """
    def deepcopy(self) -> RangesInt:
        """
        Creates a deep copy of this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance that is a deep copy of this one
        """
    def difference(self, other: RangesInt) -> RangesInt:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        
        Parameters
        ----------
        other: :class:`Ranges`
            The ranges to subtract from this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the values in ``self`` that are not in 'other'
        """
    def getOverlaps(self, ranges: collections.abc.Sequence[RangesInt], requireAll: bool = True, normalizeSelf: bool = False, normalizeOthers: bool = False) -> RangesInt:
        """
        Computes the overlap between this instance and a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to compute the overlap against
        
        requireAll: :class:`bool`
            When ``True`` (the default), the result is the intersection of ``self`` and *every* entry in 'ranges'
            (a value must fall within ``self`` and all of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            When ``False``, the result is the intersection of ``self`` and the *union* of 'ranges'
            (a value must fall within ``self`` and at least one of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the computed overlap
        """
    def has(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        
        Parameters
        ----------
        value: :class:`int`
            The value to check
        
        Returns
        -------
        :class:`bool`
            Whether 'value' is contained within any of the stored ranges
        """
    def intersect(self, ranges: collections.abc.Sequence[RangesInt], normalizeSelf: bool = False, normalizeOthers: bool = False) -> RangesInt:
        """
        Computes the intersection of this instance and a list of other :class:`Ranges` :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.getOverlaps(ranges, True, normalizeSelf, normalizeOthers)``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the intersection of ``self`` and every entry in 'ranges'
        """
    def intersectUpdate(self, ranges: collections.abc.Sequence[RangesInt], normalizeSelf: bool = False, normalizeOthers: bool = False) -> None:
        """
        Performs the intersection of this instance with a list of other :class:`Ranges`, in place :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``self``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        """
    def isEmpty(self) -> bool:
        """
        Determines whether there are no stored ranges
        
        Returns
        -------
        :class:`bool`
            Whether there are no stored ranges
        """
    def isFull(self) -> bool:
        """
        Determines whether the stored ranges span from -infinity to +infinity
        
        .. note::
            This checks for a single stored range of ``(None, None)``. If this instance was constructed with ``normalize=False``, several ranges
            could collectively cover -infinity to +infinity without having been merged into one, in which case this returns ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the stored ranges span from -infinity to +infinity
        """
    def negate(self) -> RangesInt:
        """
        Computes the negation (complement) of this instance :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``Ranges.createFull() - self``
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing every value not in ``self``
        """
    def remove(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Removes 'value' from the stored ranges, shrinking, splitting, or erasing existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' isn't contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to remove
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before removing 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate 'value' efficiently). If that assumption doesn't hold, pass ``True``, or the result may
            be incorrect
        """
    def union(self, ranges: collections.abc.Sequence[RangesInt]) -> RangesInt:
        """
        Computes the union of this instance with a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the union of ``self`` and 'ranges'
        """
    def update(self, ranges: collections.abc.Sequence[RangesInt]) -> None:
        """
        Performs the union of this instance with a list of other :class:`Ranges`, in place
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        """
    @property
    def ranges(self) -> list[tuple[int | None, int | None]]:
        """
        List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]: The stored ranges
        """
    @ranges.setter
    def ranges(self, arg0: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]]) -> None:
        ...
class RemappedKeyData:
    """
    
    A single rule inside a :meth:`OrderedMultiMap.remapKeys` remap list, expressing a
    *conditional* and/or *repositioned* rename (as opposed to a plain key, which always fires and
    stays in place).
    
    Parameters
    ----------
    key: Any
        The new key to remap matching occurrences to
    
    check: Optional[Callable[[Any, Any], :class:`bool`]]
        An optional predicate over ``(oldKey, oldValue)``. If omitted, this rule always fires. If
        present, it fires only when the predicate returns ``True`` for a given occurrence :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    toInd: Optional[:class:`int`]
        An optional target index. If present, every entry this rule produces (across all
        occurrences it fires for) is moved, as a group, to this index once the remap is complete --
        using the exact same index semantics, conflict resolution, and front/back
        overflow-clustering rules as :meth:`OrderedMultiMap.reorder`'s target index. If omitted,
        produced entries stay wherever the remap naturally places them (in place of the occurrence
        that produced them) :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __init__(self, key: typing.Any, check: collections.abc.Callable[[typing.Any, typing.Any], bool] | None = None, toInd: typing.SupportsInt | typing.SupportsIndex | None = None) -> None:
        ...
    @property
    def check(self) -> collections.abc.Callable[[typing.Any, typing.Any], bool] | None:
        """
        Optional[Callable[[Any, Any], :class:`bool`]]: The optional firing predicate
        """
    @property
    def key(self) -> typing.Any:
        """
        Any: The new key to remap matching occurrences to
        """
    @property
    def toInd(self) -> int | None:
        """
        Optional[:class:`int`]: The optional repositioning target index
        """
class ReplaceIf:
    """
    
    A :meth:`OrderedMultiMap.replaceVals` spec: replace this key's value with ``value``,
    wherever ``predicate(oldValue)`` is ``True``.
    
    Parameters
    ----------
    value: Any
        The replacement value
    
    predicate: Callable[[Any], :class:`bool`]
        The predicate deciding whether a given old value should be replaced
            
    """
    def __init__(self, value: typing.Any, predicate: collections.abc.Callable[[typing.Any], bool]) -> None:
        ...
    @property
    def predicate(self) -> collections.abc.Callable[[typing.Any], bool]:
        """
        Callable[[Any], :class:`bool`]: The replacement predicate
        """
    @property
    def value(self) -> typing.Any:
        """
        Any: The replacement value
        """
class ReplaceList:
    """
    
    A :meth:`OrderedMultiMap.replaceVals` spec: update this key's entries positionally from a
    list of values -- the i-th existing entry (true left-to-right order) gets ``values[i]``. A list
    shorter than the key's entry count leaves the remaining entries untouched; a longer list just
    has its extra values unused.
    
    Parameters
    ----------
    values: List[Any]
        The values to assign positionally
            
    """
    def __init__(self, values: collections.abc.Sequence[typing.Any]) -> None:
        ...
    @property
    def values(self) -> list[typing.Any]:
        """
        List[Any]: The values to assign positionally
        """
def appendAllToOrderedMultiMap(target: IOrderedMultiMap, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
    """
    Appends every ``(key, value)`` pair to any :class:`IOrderedMultiMap` implementation, in order --
    a small example of code written once against the interface, working identically whether
    'target' is :class:`OrderedMultiMap`/:class:`OrderedMultiMapSqrt` (via their
    ``asInterface()``) or a user's own Python subclass of :class:`IOrderedMultiMap`.
    
    Parameters
    ----------
    target: :class:`IOrderedMultiMap`
        The map to append to
    
    items: List[Tuple[Any, Any]]
        The key-value pairs to append, in order
    """
