"""
C++ internal core of AGRemap
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['BaseDFA', 'BiMap', 'CppAhoCorasickDFA', 'CppAlgo', 'CppIntTools', 'CppListTools', 'CppTrie', 'DFA', 'Ranges']
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
        This constructor is overloaded. Instead of a list of range tuples, ``CppRanges`` can also be constructed directly
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
        Creates a :class:`CppRanges` with no ranges
         
        Returns
        -------
        :class:`CppRanges`
            A new, empty instance
        """
    @staticmethod
    def createFromList(values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Ranges:
        """
        Creates a :class:`CppRanges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
         
        Consecutive integers are merged into contiguous ranges (e.g. ``[1, 2, 3, 5]`` becomes ``[1,4)`` and ``[5,6)``)
         
        Parameters
        ----------
        values: List[:class:`int`]
            The values to include
         
        Returns
        -------
        :class:`CppRanges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFromSet(values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> Ranges:
        """
        Creates a :class:`CppRanges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
         
        Consecutive integers are merged into contiguous ranges (e.g. ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)
         
        Parameters
        ----------
        values: Set[:class:`int`]
            The values to include
         
        Returns
        -------
        :class:`CppRanges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFull() -> Ranges:
        """
        Creates a :class:`CppRanges` spanning from -infinity to +infinity
         
        Returns
        -------
        :class:`CppRanges`
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
        :class:`CppRanges`
            A new instance that is a deep copy of this one
        """
    def difference(self, other: Ranges) -> Ranges:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
         
        Parameters
        ----------
        other: :class:`CppRanges`
            The ranges to subtract from this instance
         
        Returns
        -------
        :class:`CppRanges`
            A new instance containing the values in ``self`` that are not in 'other'
        """
    def getOverlaps(self, ranges: collections.abc.Sequence[Ranges], requireAll: bool = True, normalizeSelf: bool = False, normalizeOthers: bool = False) -> Ranges:
        """
        Computes the overlap between this instance and a list of other :class:`CppRanges`
         
        Parameters
        ----------
        ranges: List[:class:`CppRanges`]
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
        :class:`CppRanges`
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
        Computes the intersection of this instance and a list of other :class:`CppRanges` :raw-html:`<br />` :raw-html:`<br />`
         
        Equivalent to ``self.getOverlaps(ranges, True, normalizeSelf, normalizeOthers)``
         
        Parameters
        ----------
        ranges: List[:class:`CppRanges`]
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
        :class:`CppRanges`
            A new instance containing the intersection of ``self`` and every entry in 'ranges'
        """
    def intersectUpdate(self, ranges: collections.abc.Sequence[Ranges], normalizeSelf: bool = False, normalizeOthers: bool = False) -> None:
        """
        Performs the intersection of this instance with a list of other :class:`CppRanges`, in place :raw-html:`<br />` :raw-html:`<br />`
         
        Equivalent to ``self.intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``self``
         
        Parameters
        ----------
        ranges: List[:class:`CppRanges`]
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
         
        Equivalent to ``CppRanges.createFull() - self``
         
        Returns
        -------
        :class:`CppRanges`
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
        Computes the union of this instance with a list of other :class:`CppRanges`
         
        Parameters
        ----------
        ranges: List[:class:`CppRanges`]
            The list of other ranges to union with
         
        Returns
        -------
        :class:`CppRanges`
            A new instance containing the union of ``self`` and 'ranges'
        """
    def update(self, ranges: collections.abc.Sequence[Ranges]) -> None:
        """
        Performs the union of this instance with a list of other :class:`CppRanges`, in place
         
        Parameters
        ----------
        ranges: List[:class:`CppRanges`]
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
