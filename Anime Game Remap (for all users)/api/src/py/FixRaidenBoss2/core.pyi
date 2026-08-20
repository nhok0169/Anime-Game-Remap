"""
C++ internal core of AGRemap
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['BaseDFA', 'BaseTokenizer', 'BiMap', 'CppAhoCorasickDFA', 'CppAlgo', 'CppBaseIniClassifier', 'CppHashTools', 'CppIniClassifyStats', 'CppIntTools', 'CppListTools', 'CppModAssets', 'CppModType', 'CppTrie', 'CppVersion', 'DFA', 'FilteredTokenizer', 'GameTypeId', 'GameTypeIdTools', 'Hash128', 'Hash64', 'Hashes', 'IOrderedMultiMap', 'IfContentPart', 'IfContentPartColourChange', 'IfContentPartColouring', 'IfPredTokenizer', 'IfTemplatePart', 'Indices', 'KeyRemapData', 'ModDictAssets', 'ModMappedAssets', 'ModTypeId', 'ModTypeIdTools', 'OrderedMultiMap', 'OrderedMultiMapIterator', 'OrderedMultiMapSqrt', 'OrderedMultiMapSqrtIterator', 'ParseContext', 'Ranges', 'RangesInt', 'RemappedKeyData', 'ReplaceIf', 'ReplaceList', 'SympyTokenizer', 'Token', 'appendAllToOrderedMultiMap']
class BaseDFA:
    pass
class BaseTokenizer:
    """
    
    The base class used for tokenizing text
    
    Parameters
    ----------
    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
    
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, tokens: collections.abc.Mapping[str, str], setup: bool = True) -> None:
        ...
    def addASCIIRangeTransitions(self, srcId: str, startChar: str, endChar: str, destId: str) -> None:
        """
        Adds a group of transitions from one state to another according to a range of `ASCII`_ characters
        
        Parameters
        ----------
        srcId: :class:`str`
            The id of the source state for the transition
        
        startChar: :class:`str`
            The starting character within the ASCII range to add a transition for
        
        endChar: :class:`str`
            The ending character within the ASCII range to add a transition for
        
        destId: :class:`str`
            The id of the destination state for the transition
        """
    def addKeyword(self, keyword: str) -> str:
        """
        Adds a keyword into the `DFA`_ of the tokenizer
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add
        
        Returns
        -------
        :class:`str`
            The id of the accepting node in the `DFA`_
        """
    def addStartState(self) -> str:
        """
        Adds the start state representing an empty string
        
        Returns
        -------
        :class:`str`
            The id of the start state
        """
    def clear(self) -> None:
        """
        Clears the `DFA`_ of the tokenizer
        """
    def reset(self) -> None:
        """
        Resets the state of the `DFA`_ for the tokenizer
        """
    def setup(self) -> None:
        """
        Performs any necessary setup to the tokenizer
        """
    @typing.overload
    def simplifiedMaximalMunch(self, src: ParseContext, includeFiltered: bool = False) -> list[Token]:
        """
        Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm
        
        Parameters
        ----------
        src: Union[:class:`str`, :class:`ParseContext`]
            The source text to be tokenized
        
        includeFiltered: :class:`bool`
            Ignored by this base class -- see :class:`FilteredTokenizer` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Raises
        ------
        :class:`SyntaxErr`
            The provided source text cannot be correctly tokenized
        
        Returns
        -------
        List[:class:`Token`]
            The list of tokens to the source text
        """
    @typing.overload
    def simplifiedMaximalMunch(self, src: str, includeFiltered: bool = False) -> list[Token]:
        ...
    @property
    def startStateId(self) -> str:
        """
        :class:`str`: The id of the starting state of the `DFA`_
        """
    @property
    def tokens(self) -> dict[str, str]:
        """
        Dict[:class:`str`, :class:`str`]: The tokens used for tokenization
        
        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
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
    def findMaximal(self, txt: str, count: typing.SupportsInt | typing.SupportsIndex = 1, pred: collections.abc.Callable[[str], bool] | None = None) -> tuple[str | None, int] | tuple[list[str], list[int]]:
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
        
        pred: Optional[Callable[[:class:`str`], :class:`bool`]]
            If provided, only a keyword satisfying this predicate can be picked -- among the keywords
            ending at a given position, the largest one satisfying 'pred' is picked, not necessarily the
            largest one overall :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
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
    def getMaximal(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None, count: typing.SupportsInt | typing.SupportsIndex = 1, pred: collections.abc.Callable[[str], bool] | None = None) -> tuple[str | None, typing.Any] | tuple[list[str], list[typing.Any]]:
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
        
        pred: Optional[Callable[[:class:`str`], :class:`bool`]]
            If provided, only a keyword satisfying this predicate can be picked -- among the keywords
            ending at a given position, the largest one satisfying 'pred' is picked, not necessarily the
            largest one overall :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
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
class CppBaseIniClassifier:
    """
    
    Base class to help classify the type of mod given the mod's .ini files
        
    """
    def __init__(self) -> None:
        ...
    @typing.overload
    def classify(self, iniTxt: str, gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> CppIniClassifyStats:
        """
        Determines the type of mod given the text from the mod's .ini file
        
        Parameters
        ----------
        iniTxt: Union[:class:`str`, List[:class:`str`]]
            The text of the .ini file to read from, given as either:
        
            * the full text OR
            * lines of text with each line ending with a newline character
        
        gameTypeId: Optional[:class:`GameTypeId`]
            The game the .ini file is expected to belong to, if known
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`CppIniClassifyStats`
            The stats about the classification of the .ini file
        """
    @typing.overload
    def classify(self, iniTxt: collections.abc.Sequence[str], gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> CppIniClassifyStats:
        ...
    def clear(self) -> None:
        """
        Clears the state of the classifier
        """
class CppHashTools:
    """
    C++ tools for deterministically hashing data
    """
    @staticmethod
    def clear() -> None:
        """
        Clears any saved internal state this class accumulates across calls (currently just the
        collision-disambiguation frequency counts used by :meth:`getShortDeterministicHashStr`)
        """
    @staticmethod
    @typing.overload
    def getDeterministicHash(data: bytes) -> Hash128:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant deterministic hash
        """
    @staticmethod
    @typing.overload
    def getDeterministicHash(str: str) -> Hash128:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant deterministic hash
        """
    @staticmethod
    @typing.overload
    def getDeterministicHashStr(data: bytes) -> str:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`str`
            The resultant deterministic hash, as a base64 string (see :meth:`Hash128.toBase64`)
        """
    @staticmethod
    @typing.overload
    def getDeterministicHashStr(str: str) -> str:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`str`
            The resultant deterministic hash, as a base64 string (see :meth:`Hash128.toBase64`)
        """
    @staticmethod
    @typing.overload
    def getShortDeterministicHashStr(data: bytes) -> str:
        """
        Deterministically hashes a buffer of bytes into a short, compact base64 string :raw-html:`<br />` :raw-html:`<br />`
        
        The hash is reduced modulo :math:`2^{16}` before being converted to base64, so unlike
        :meth:`getDeterministicHashStr`, collisions across different inputs are expected. To
        disambiguate a collision, every occurrence of a short hash value after the first has
        ``_<frequency>`` appended, where ``<frequency>`` (itself base64-encoded) counts how many times
        that short hash value has already been produced by this method
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`str`
            The resultant short, possibly-colliding hash, as a base64 string
        """
    @staticmethod
    @typing.overload
    def getShortDeterministicHashStr(str: str) -> str:
        """
        Deterministically hashes a string into a short, compact base64 string :raw-html:`<br />` :raw-html:`<br />`
        
        See :meth:`getShortDeterministicHashStr` (the ``bytes`` overload) for the full explanation of
        the collision-disambiguation behaviour
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`str`
            The resultant short, possibly-colliding hash, as a base64 string
        """
class CppIniClassifyStats:
    """
    
    Stores the statistics about the classification result of a .ini file
    
    Parameters
    ----------
    modType: Dict[:class:`int`, :class:`CppModType`]
        The types of mod found, keyed by their id
    
        **Default**: ``{}``
    
    isMod: :class:`bool`
        Whether the .ini file belongs to a mod
    
        **Default**: ``False``
    
    isFixed: :class:`bool`
        Whether the .ini file is fixed
    
        **Default**: ``False``
        
    """
    def __init__(self, modType: dict = {}, isMod: bool = False, isFixed: bool = False) -> None:
        ...
    @property
    def isFixed(self) -> bool:
        """
        :class:`bool`: Whether the .ini file is fixed
        """
    @isFixed.setter
    def isFixed(self, arg0: bool) -> None:
        ...
    @property
    def isMod(self) -> bool:
        """
        :class:`bool`: Whether the .ini file belongs to a mod
        """
    @isMod.setter
    def isMod(self, arg0: bool) -> None:
        ...
    @property
    def modType(self) -> dict:
        """
        Dict[:class:`int`, :class:`CppModType`]: The types of mod found, keyed by their id
        """
    @modType.setter
    def modType(self, arg1: dict) -> None:
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
class CppModAssets:
    """
    
    Handles assets of any type for a mod where retrieval is based on some keys where one or more of
    the keys refer to some versioning
    
    :raw-html:`<br />`
    
    If an asset has only one version column, :class:`CppModDictAssets` is the better fit (a real
    hash-map lookup instead of this class's linear scan) -- this class exists specifically for the
    multi-version-column case (e.g. this project's real ``VGRemaps``, which resolves a ``fromVersion``
    and a ``toVersion`` independently and sequentially)
    
    :raw-html:`<br />`
    
    Like :class:`CppModDictAssets`, the source data is never a nested dict internally -- rows are
    supplied already-flattened, as a list of ``(indexVals, value)`` tuples, or as a real nested dict
    (flattened automatically -- see the constructor's 'rows' argument)
        
    """
    def __init__(self, isVersionColumn: collections.abc.Sequence[bool], rows: typing.Any = []) -> None:
        """
        Constructs a new asset lookup table
        
        Parameters
        ----------
        isVersionColumn: List[:class:`bool`]
            One entry per index column, in index order -- ``True`` marks that column as a version column.
            Must have at least 1 element
        
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The initial rows to populate the table with -- either a flat list of ``(indexVals, value)``
            tuples, or a real nested dict ('len(isVersionColumn)' levels deep)
        
            **Default**: ``[]``
        """
    def __len__(self) -> int:
        """
        The total number of rows currently in the table
        """
    def addRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to the table (an addition beyond the pure-Python original, which has no
        incremental-add capability at all) -- overwrites the value of any row whose full key already
        exists
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The rows to add, in the same shape as the constructor's own 'rows' argument
        """
    def get(self, nonVersionVals: collections.abc.Sequence[typing.Any], versionVals: collections.abc.Sequence[typing.Any], errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the corresponding asset
        
        Parameters
        ----------
        nonVersionVals: List[Optional[Any]]
            One entry per non-version column, in their relative index order -- ``None`` at a position
            means "match any value there". Must have exactly :attr:`nonVersionColumnCount` elements
        
        versionVals: List[Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]]
            One entry per version column, in their relative index order -- ``None`` at a position means
            "use the latest available value for this column, among rows still matching everything
            resolved so far". Must have exactly :attr:`versionColumnCount` elements :raw-html:`<br />` :raw-html:`<br />`
        
            Version columns are resolved sequentially, in index order -- each one's floor-match narrows
            the candidate set before the next version column is resolved against it
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching asset is found
        
            **Default**: ``True``
        
        Raises
        ------
        :class:`ValueError`
            If 'nonVersionVals'/'versionVals' don't have exactly :attr:`nonVersionColumnCount`/
            :attr:`versionColumnCount` elements respectively, or if a version value doesn't parse
        
        :class:`KeyError`
            If no matching asset is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        Any
            The found asset, or ``None`` if none is found and 'errorOnNotFound' is ``False``
        """
    @property
    def nonVersionColumnCount(self) -> int:
        """
        :class:`int`: The number of non-version columns
        """
    @property
    def totalIndices(self) -> int:
        """
        :class:`int`: The total number of index columns
        """
    @property
    def versionColumnCount(self) -> int:
        """
        :class:`int`: The number of version columns
        """
class CppModType:
    """
    
    Class for defining a type of mod
    
    Parameters
    ----------
    gameTypeId: :class:`int`
        The id for the game this type of mod belongs to -- stored as-is, with no validation that it
        corresponds to one of :class:`GameTypeId`'s declared values (see :class:`GameTypeIdTools` if
        that's needed)
    
    modTypeId: :class:`int`
        The id for this specific type of mod -- stored as-is, with no validation that it corresponds
        to one of :class:`ModTypeId`'s declared values (see :class:`ModTypeIdTools` if that's needed),
        so a custom mod type using some id not registered in :class:`ModTypeId` can still be represented
        
    """
    def __init__(self, gameTypeId: typing.SupportsInt | typing.SupportsIndex, modTypeId: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def gameTypeId(self) -> int:
        """
        :class:`int`: The id for the game this type of mod belongs to
        """
    @gameTypeId.setter
    def gameTypeId(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def modTypeId(self) -> int:
        """
        :class:`int`: The id for this specific type of mod -- stored as-is, with no
        validation that it corresponds to one of :class:`ModTypeId`'s declared values, so a custom mod
        type using some id not registered in :class:`ModTypeId` can still be represented
        :raw-html:`<br />` :raw-html:`<br />`
        
        Setting this also updates 'name': if the new value corresponds to one of :class:`ModTypeId`'s
        declared values, 'name' is set to that value's name (see :class:`ModTypeIdTools`); otherwise,
        'name' is set to the empty string
        """
    @modTypeId.setter
    def modTypeId(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def name(self) -> str:
        """
        :class:`str`: The name for this type of mod, kept in sync with 'modTypeId' by
        its setter -- if 'modTypeId' corresponds to one of :class:`ModTypeId`'s declared values, this is
        that value's name (see :class:`ModTypeIdTools`); otherwise, this is the empty string
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
class CppVersion:
    """
    
    A single `PEP 440`_ version value -- a from-scratch C++ port of Python's `packaging.version.Version`_,
    matching its parsing/normalization/comparison behaviour exactly (verified empirically against the
    real ``packaging`` library during development, not just read off its source)
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' are the same version
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' are different versions
    
        .. describe:: x < y, x <= y, x > y, x >= y
    
            Compares two versions following `PEP 440`_'s ordering rules
    
        .. describe:: hash(x)
    
            Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`
    
        .. describe:: str(x)
    
            Equivalent to ``x.toString()``
        
    """
    @staticmethod
    def parse(raw: str) -> FixRaidenBoss2.core.CppVersion | None:
        """
        Parses a raw version string
        
        Parameters
        ----------
        raw: :class:`str`
            The raw version string to parse
        
        Returns
        -------
        Optional[:class:`CppVersion`]
            The parsed version, or ``None`` if 'raw' does not conform to `PEP 440`_ in any way
        """
    def __eq__(self, other: CppVersion) -> bool:
        """
        Determines whether 'self' and 'other' are the same version
        """
    def __ge__(self, other: CppVersion) -> bool:
        ...
    def __gt__(self, other: CppVersion) -> bool:
        ...
    def __hash__(self) -> int:
        """
        Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set
        """
    def __le__(self, other: CppVersion) -> bool:
        ...
    def __lt__(self, other: CppVersion) -> bool:
        ...
    def __ne__(self, other: CppVersion) -> bool:
        """
        Determines whether 'self' and 'other' are different versions
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def toString(self) -> str:
        """
        Converts the version back into its normalized, round-trippable string form
        
        Returns
        -------
        :class:`str`
            The string form of the version
        """
    @property
    def base_version(self) -> str:
        """
        :class:`str`: The epoch and release segment only, with no pre/post/dev/local segment
        """
    @property
    def dev(self) -> int | None:
        """
        Optional[:class:`int`]: The dev-release number, or ``None`` if there is none
        """
    @property
    def epoch(self) -> int:
        """
        :class:`int`: The epoch of the version (``0`` if none was specified)
        """
    @property
    def is_devrelease(self) -> bool:
        """
        :class:`bool`: Whether this is a dev-release
        """
    @property
    def is_postrelease(self) -> bool:
        """
        :class:`bool`: Whether this is a post-release
        """
    @property
    def is_prerelease(self) -> bool:
        """
        :class:`bool`: Whether this is a pre-release (has a pre-release or dev-release segment)
        """
    @property
    def local(self) -> str | None:
        """
        Optional[:class:`str`]: The local version segment, dot-joined, or ``None`` if there is none
        """
    @property
    def major(self) -> int:
        """
        :class:`int`: The first component of :attr:`release`, or ``0`` if unavailable
        """
    @property
    def micro(self) -> int:
        """
        :class:`int`: The third component of :attr:`release`, or ``0`` if unavailable
        """
    @property
    def minor(self) -> int:
        """
        :class:`int`: The second component of :attr:`release`, or ``0`` if unavailable
        """
    @property
    def post(self) -> int | None:
        """
        Optional[:class:`int`]: The post-release number, or ``None`` if there is none
        """
    @property
    def pre(self) -> tuple[str, int] | None:
        """
        Optional[Tuple[:class:`str`, :class:`int`]]: The pre-release segment (normalized letter and number), or ``None`` if there is none
        """
    @property
    def public(self) -> str:
        """
        :class:`str`: :meth:`toString` without the local segment
        """
    @property
    def release(self) -> list[int]:
        """
        Tuple[:class:`int`, ...]: The numeric components of the release segment, in order, including any
        trailing zeros (e.g. ``CppVersion.parse("2.0.0").release == (2, 0, 0)``)
        """
class DFA(BaseDFA):
    """
    
    Class for a `DFA (Deterministic Finite Automaton)`_
            
    """
    def __init__(self) -> None:
        ...
    def acceptLen(self) -> int:
        """
        Retrieves the number of accepting states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of accepting states in the `DFA`_
        """
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
    def clear(self) -> None:
        """
        Clears the `DFA`_
        """
    def getKeywordToState(self, srcId: typing.Any, keyword: typing.Any) -> typing.Any | None:
        """
        Retrieves the destination state of a keyword transition from a particular state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
        keyword: `Hashable`_
            The keyword for the transition
        
        Returns
        -------
        Optional[`Hashable`_]
            The id of the destination state of the transition, or ``None`` if no such transition exists
            from 'srcId'
        """
    def getKeywordTransitions(self, id: typing.Any) -> tuple[list[typing.Any], bool]:
        """
        Retrieves all the keyword transitions connected to a particular state
        
        Parameters
        ----------
        id: `Hashable`_
            The id of the state to retrieve the keyword transitions for
        
        Returns
        -------
        Tuple[List[Hashable], :class:`bool`]
            A tuple containing:
        
            #. The keyword transitions connected to 'id'
            #. Whether 'id' corresponds to an existing state in the `DFA`_ -- if ``False``, the list is empty
        """
    def hasKeywordTransition(self, srcId: typing.Any, keyword: typing.Any) -> bool:
        """
        Determines whether a keyword transition exists from a particular state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state to check
        
        keyword: `Hashable`_
            The keyword for the transition to check
        
        Returns
        -------
        :class:`bool`
            Whether the transition exists from 'srcId'
        """
    def isAccept(self, id: typing.Any) -> bool:
        """
        Determines whether some state is an accepting state
        
        Parameters
        ----------
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
        
        Parameters
        ----------
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
        
        Parameters
        ----------
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
class FilteredTokenizer(BaseTokenizer):
    """
    
    This class inherits from :class:`BaseTokenizer`
    
    A tokenizer that still accepts all tokens, but does not include certain tokens into the tokenized result
    
    Parameters
    ----------
    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
    
    keywordTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
    
    filteredTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
    
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, tokens: collections.abc.Mapping[str, str], keywordTokenIds: collections.abc.Set[str], filteredTokenIds: collections.abc.Set[str], setup: bool = True) -> None:
        ...
    @property
    def filteredTokenIds(self) -> set[str]:
        """
        Set[:class:`str`]: The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
        """
    @property
    def keywordTokenIds(self) -> set[str]:
        """
        Set[:class:`str`]: The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
        """
class GameTypeId:
    """
    
    The names of the different supported games
        
    
    Members:
    
      GI : Genshin Impact
    
      WuWa : Wuthering Waves
    """
    GI: typing.ClassVar[GameTypeId]  # value = <GameTypeId.GI: 0>
    WuWa: typing.ClassVar[GameTypeId]  # value = <GameTypeId.WuWa: 1>
    __members__: typing.ClassVar[dict[str, GameTypeId]]  # value = {'GI': <GameTypeId.GI: 0>, 'WuWa': <GameTypeId.WuWa: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class GameTypeIdTools:
    """
    
    Tools for handling :class:`GameTypeId`
        
    """
    @staticmethod
    def getEnum(value: typing.SupportsInt | typing.SupportsIndex) -> FixRaidenBoss2.core.GameTypeId | None:
        """
        Retrieves the corresponding :class:`GameTypeId` for some integer value, checking that the value
        actually corresponds to one of :class:`GameTypeId`'s declared values
        
        Parameters
        ----------
        value: :class:`int`
            The integer value to convert
        
        Returns
        -------
        Optional[:class:`GameTypeId`]
            The corresponding :class:`GameTypeId`, if 'value' is valid
        """
    @staticmethod
    def getName(value: GameTypeId) -> str:
        """
        Retrieves the corresponding name for a :class:`GameTypeId`
        
        Parameters
        ----------
        value: :class:`GameTypeId`
            The :class:`GameTypeId` to retrieve the name for
        
        Returns
        -------
        :class:`str`
            The name for 'value'
        """
class Hash128:
    """
    
    A deterministic 128-bit hash id, the long counterpart to :class:`Hash64`
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same hash value
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different hash values
    
        .. describe:: x < y
    
            An arbitrary but consistent (and deterministic) total ordering
    
        .. describe:: hash(x)
    
            Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`
    
        .. describe:: str(x)
    
            Equivalent to ``x.toHexString()``
        
    """
    @staticmethod
    @typing.overload
    def hash(data: bytes) -> Hash128:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant hash
        """
    @staticmethod
    @typing.overload
    def hash(str: str) -> Hash128:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant hash
        """
    def __eq__(self, other: Hash128) -> bool:
        """
        Determines whether 'self' and 'other' store the same hash value
        """
    def __hash__(self) -> int:
        """
        Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Constructs a hash with both halves set to 0
        """
    @typing.overload
    def __init__(self, high: typing.SupportsInt | typing.SupportsIndex, low: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Constructs a hash from its 2 64-bit halves
        
        Parameters
        ----------
        high: :class:`int`
            The high 64 bits of the hash
        
        low: :class:`int`
            The low 64 bits of the hash
        """
    def __lt__(self, other: Hash128) -> bool:
        """
        An arbitrary but consistent (and deterministic) total ordering
        """
    def __ne__(self, other: Hash128) -> bool:
        """
        Determines whether 'self' and 'other' store different hash values
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def toBase64(self) -> str:
        """
        Converts the hash to a fixed-length base64 string
        
        Returns
        -------
        :class:`str`
            The base64 string
        """
    def toHexString(self) -> str:
        """
        Converts the hash to a fixed-length, lowercase hex string
        
        Returns
        -------
        :class:`str`
            The hex string
        """
    @property
    def high(self) -> int:
        """
        :class:`int`: The high 64 bits of the hash
        """
    @property
    def low(self) -> int:
        """
        :class:`int`: The low 64 bits of the hash
        """
class Hash64:
    """
    
    A deterministic 64-bit hash id, the short counterpart to :class:`Hash128`
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same hash value
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different hash values
    
        .. describe:: x < y
    
            An arbitrary but consistent (and deterministic) total ordering
    
        .. describe:: hash(x)
    
            Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`
    
        .. describe:: str(x)
    
            Equivalent to ``x.toHexString()``
        
    """
    @staticmethod
    @typing.overload
    def hash(data: bytes) -> Hash64:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`Hash64`
            The resultant hash
        """
    @staticmethod
    @typing.overload
    def hash(str: str) -> Hash64:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`Hash64`
            The resultant hash
        """
    def __eq__(self, other: Hash64) -> bool:
        """
        Determines whether 'self' and 'other' store the same hash value
        """
    def __hash__(self) -> int:
        """
        Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Constructs a hash with a value of 0
        """
    @typing.overload
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Constructs a hash from its raw 64-bit value
        
        Parameters
        ----------
        value: :class:`int`
            The raw 64-bit value of the hash
        """
    def __lt__(self, other: Hash64) -> bool:
        """
        An arbitrary but consistent (and deterministic) total ordering
        """
    def __ne__(self, other: Hash64) -> bool:
        """
        Determines whether 'self' and 'other' store different hash values
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def toBase64(self) -> str:
        """
        Converts the hash to a fixed-length base64 string
        
        Returns
        -------
        :class:`str`
            The base64 string
        """
    def toHexString(self) -> str:
        """
        Converts the hash to a fixed-length, lowercase hex string
        
        Returns
        -------
        :class:`str`
            The hex string
        """
    @property
    def value(self) -> int:
        """
        :class:`int`: The raw 64-bit value of the hash
        """
class Hashes(ModMappedAssets):
    """
    
    This class inherits from :class:`ModMappedAssets`
    
    Class for managing hashes for a mod, pre-populated with this project's real hash data
    
    :raw-html:`<br />`
    
    .. note::
        Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
        ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:
    
        * version (version index)
        * name
        * type
        
    """
    def __init__(self, map: typing.Any = None) -> None:
        """
        Constructs a new, fully-populated hash lookup table
        
        Parameters
        ----------
        map: Optional[Dict[Any, List[Any]]]
            The `adjacency list`_ that maps the hashes to fix from to the hashes to fix to using the
            predefined mods
        
            **Default**: ``None``
        """
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
class IfContentPart(IfTemplatePart):
    """
    
    This class inherits from :class:`IfTemplatePart`
    
    The content part of an `IfTemplate`, holding the key-value pairs (e.g. a `.ini` section's
    registers) for one part of the template.
    
    This class owns its data purely through a caller-supplied :class:`IOrderedMultiMap`
    implementation -- pick which concrete ordered-multimap backs a given :class:`IfContentPart`
    (:class:`CppOrderedMultiMap`/:class:`CppOrderedMultiMapSqrt` via their ``asInterface()`` method,
    or any custom :class:`IOrderedMultiMap` implementation of your own, including one implemented
    from Python), and every method on this class is a thin, renamed delegation straight to that
    implementation -- the semantics for every operation are exactly :class:`CppOrderedMultiMap`'s
    documented rules; only the *method names* below intentionally echo this project's deprecated,
    pre-C++-port `IfContentPart` naming (e.g. ``insertAllAt`` -> ``addKVPsByInds``).
    
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
    
    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part.
    
        **Default**: ``None``
            
    """
    @staticmethod
    def buildFromOrder(src: typing.Any, depth: typing.SupportsInt | typing.SupportsIndex = 0, content: typing.Any = None, id: typing.Any = None) -> IfContentPart:
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
            :class:`IfContentPart`'s top-level warning about what that means for 'content' afterward
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used
        
        id: Optional[:class:`int`]
            The id for the part. If this parameter is ``None``, will generate a new id for the part.
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The newly-created part
        """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> IfContentPart:
        """
        Creates a copy of this part (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IfContentPart:
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
    def __init__(self, src: typing.Any = None, depth: typing.SupportsInt | typing.SupportsIndex = 0, content: typing.Any = None, id: typing.Any = None) -> None:
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
    def clone(self, newId: bool = False) -> IfContentPart:
        """
        Creates a deep copy of this part, at the same depth
        
        Parameters
        ----------
        newId: :class:`bool`
            Whether to generate a new id for the cloned part :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning the clone keeps this part's own :attr:`id`
        
        Returns
        -------
        :class:`IfContentPart`
            The cloned part
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
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[IfContentPart]:
        """
        Splits this part into several smaller parts at the given indices, each at the same depth as
        this part; see :meth:`CppOrderedMultiMap.splitByInds` for the full semantics
        
        Returns
        -------
        List[:class:`IfContentPart`]
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
class IfContentPartColourChange:
    """
    
    Class to store the change in state of a particular key for a :class:`IfContentPartColouring`
    
    Parameters
    ----------
    old: Optional[Any]
        The old value of a particular key -- either a plain value (the key's value came from some
        previous :class:`IfContentPart`), or a ``List[Tuple[int, Any]]`` (the key's values come
        from the current :class:`IfContentPart`, each paired with its index of occurrence) :raw-html:`<br />` :raw-html:`<br />`
    
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
    
    Class that keeps track of the current state of the `KVPs`_ within a :class:`IfContentPart` --
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
    
        * A plain value, indicating the value of the `KVP`_ comes from some previous :class:`IfContentPart`, OR
        * A ``List[Tuple[int, Any]]``. The list indicates that the values of the corresponding key
          come from the current :class:`IfContentPart`, each tuple containing the new state value
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
            current :class:`IfContentPart` (a list of indexed occurrences) -- a value carried over from
            a previous part is always returned unfiltered, as ``(None, value)``.
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`
        
            The predicate takes in the following parameters:
        
            #. The index the value appears in the current :class:`IfContentPart`. If this argument is
               ``None``, then the value was carried over from a previous part
            #. The corresponding value
        
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        List[Tuple[Optional[:class:`int`], Any]]
            Both the values and their index within the current :class:`IfContentPart`. Empty if ``key``
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
        way :class:`IfContentPart` itself splits ``getVals``/``getKeys`` rather than returning a value
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
    def updateColouring(self, ifContentPart: IfContentPart, targetKeys: collections.abc.Set[typing.Any] | None = None, updatePreviousKVPs: bool = True) -> dict[typing.Any, IfContentPartColourChange]:
        """
        Updates the current state of the `KVPs`_ based on the current :class:`IfContentPart`
        
        Parameters
        ----------
        ifContentPart: :class:`IfContentPart`
            The part to update the new `KVPs`_ from
        
        targetKeys: Optional[Set[Any]]
            The target keys to keep track of :raw-html:`<br />` :raw-html:`<br />`
        
            If this value is ``None``, then will keep track of all the keys :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        updatePreviousKVPs: :class:`bool`
            Whether to also update the `KVP`_ values from previous :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[Any, :class:`IfContentPartColourChange`]
            The change in the state :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the keys and the values are the state change for the keys
        """
class IfPredTokenizer(FilteredTokenizer):
    """
    
    This class inherits from :class:`FilteredTokenizer`
    
    The tokenizer used for conditional predicates within a .ini file
    
    eg.
    
    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1,3
    
        if pred1
            ...
        else if pred2
            ...
        endif
    
    Parameters
    ----------
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, setup: bool = True) -> None:
        ...
class IfTemplatePart:
    """
    
    Base class for some part in an `IfTemplate`
    
    Parameters
    ----------
    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part.
    
        **Default**: ``None``
            
    """
    def __init__(self, id: typing.Any = None) -> None:
        ...
    def refreshId(self) -> int:
        """
        Regenerates the id for the part
        
        Returns
        -------
        :class:`int`
            The newly generated id
        """
    @property
    def id(self) -> int:
        """
        :class:`int`: The id for the part
        """
class Indices(ModMappedAssets):
    """
    
    This class inherits from :class:`ModMappedAssets`
    
    Class for managing indices for a mod, pre-populated with this project's real index data
    
    :raw-html:`<br />`
    
    .. note::
        Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
        ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:
    
        * version (version index)
        * name
        * component
        * type
        
    """
    def __init__(self, map: typing.Any = None) -> None:
        """
        Constructs a new, fully-populated index lookup table
        
        Parameters
        ----------
        map: Optional[Dict[Any, List[Any]]]
            The `adjacency list`_ that maps the indices to fix from to the indices to fix to using the
            predefined mods
        
            **Default**: ``None``
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
class ModDictAssets:
    """
    
    Handles assets of any type for a mod where retrieval is based on some keys where only one of the
    keys refers to some versioning
    
    :raw-html:`<br />`
    
    Internally, the source data is never a nested dict -- rows are stored already-flattened, as
    ``(indexVals, value)`` tuples, where ``indexVals`` holds every index column's raw value in index
    order (including the version index's own raw, not-yet-parsed value). The constructor takes rows
    already in that shape; :meth:`fromNestedDict` builds an instance from a real nested dict instead
    (the shape ``HashData``/``IndexData`` are written as), flattening it in C++ rather than Python
        
    """
    @staticmethod
    def fromNestedDict(totalIndices: typing.SupportsInt | typing.SupportsIndex, versionIndexPos: typing.SupportsInt | typing.SupportsIndex, repo: dict) -> ModDictAssets:
        """
        Constructs a new asset lookup table from a real nested dict, flattening it first
        
        Parameters
        ----------
        totalIndices: :class:`int`
            The total number of index columns (including the version index)
        
        versionIndexPos: :class:`int`
            The position (0-based) of the version index within a row's index values
        
        repo: dict
            The nested dict to flatten, exactly 'totalIndices' levels deep (e.g. for
            ``totalIndices = 3``: ``{version: {name: {type: leafValue}}}``)
        
        Raises
        ------
        :class:`ValueError`
            If 'repo' is not nested exactly 'totalIndices' levels deep
        """
    def __init__(self, totalIndices: typing.SupportsInt | typing.SupportsIndex, versionIndexPos: typing.SupportsInt | typing.SupportsIndex, rows: typing.Any = []) -> None:
        """
        Constructs a new asset lookup table
        
        Parameters
        ----------
        totalIndices: :class:`int`
            The total number of index columns (including the version index)
        
        versionIndexPos: :class:`int`
            The position (0-based) of the version index within a row's index values
        
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The initial rows to populate the table with -- either a flat list of ``(indexVals, value)``
            tuples, or a real nested dict ('totalIndices' levels deep) -- see :meth:`addRows`
        
            **Default**: ``[]``
        """
    def __len__(self) -> int:
        """
        The total number of rows currently in the table, across every non-version index group
        """
    def addRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to the table, overwriting the value of any row whose full key (every non-version
        index value, plus its parsed version) already exists
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The rows to add, in the same shape as the constructor's own 'rows' argument (a flat list or a
            real nested dict)
        
        Raises
        ------
        :class:`ValueError`
            If any row's index values don't match :attr:`totalIndices` in length, or if a row's version
            index value fails to parse as a version
        """
    def get(self, nonVersionVals: collections.abc.Sequence[typing.Any], version: typing.Any = None, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the corresponding asset
        
        Parameters
        ----------
        nonVersionVals: List[Any]
            The values of every index column that does not refer to a version, in index order (with the
            version column's position skipped)
        
        version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The specific version to query the asset -- the latest available version is used if this is
            ``None`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching asset is found
        
            **Default**: ``True``
        
        Raises
        ------
        :class:`ValueError`
            If 'nonVersionVals' doesn't have exactly :attr:`totalIndices` ``- 1`` elements, or if
            'version' doesn't parse as a valid version
        
        :class:`KeyError`
            If no matching asset is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        Any
            The found asset, or ``None`` if none is found and 'errorOnNotFound' is ``False``
        """
    def toNestedDict(self) -> dict:
        """
        Rebuilds the original nested-dict form of this table's data (``{indexVal0: {indexVal1: {... :
        value}}}``, in index-column order, the version column's original raw value replaced with its
        normalized string form) -- the inverse of :meth:`fromNestedDict`/the constructor's own nested-dict
        'rows' shape
        
        Returns
        -------
        dict
            The reconstructed nested dict
        """
    @property
    def totalIndices(self) -> int:
        """
        :class:`int`: The total number of index columns (including the version index)
        """
    @property
    def versionIndexPos(self) -> int:
        """
        :class:`int`: The position (0-based) of the version index within a row's index values
        """
class ModMappedAssets:
    """
    
    Handles assets of any type where asset retrieval is based on a mapping -- a `bipartite graph`_
    that maps assets to fix from to assets to fix to
        
    """
    def __init__(self, repo: ModDictAssets, map: typing.Any = None, nonVersionIndexNames: typing.Any = None) -> None:
        """
        Constructs a new mapped asset table
        
        Parameters
        ----------
        repo: :class:`ModDictAssets`
            The underlying asset data
        
        map: Optional[Dict[Any, List[Any]]]
            The initial adjacency list mapping assets to fix from to assets to fix to
        
            **Default**: ``None``
        
        nonVersionIndexNames: Optional[List[:class:`str`]]
            The names of 'repo''s non-version index columns, in position order -- when given, ``hasFrom``/
            ``getKey``/``replace``/``replaceAll``/``_convertNonVersionVals`` accept a flexible bare value,
            a list, or a dict keyed by one of these names for their non-version-values filter, instead of
            requiring an already-positional list. ``None`` (the default) keeps the strictly positional
            behaviour, appropriate for any use that isn't backed by named indices
        
            **Default**: ``None``
        """
    def _convertNonVersionVals(self, indexVals: typing.Any) -> list:
        """
        Normalizes a flexible non-version-values filter into the plain positional
        ``List[Optional[Any]]`` shape :meth:`getKey`/:meth:`hasFrom`/:meth:`replace` accept for their own
        'nonVersionVals'/'fromNonVersionVals' argument (``None`` = wildcard at that position) --
        :attr:`nonVersionIndexNames` names each position :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            Calling this directly is rarely necessary any more -- :meth:`getKey`/:meth:`hasFrom`/
            :meth:`replace`/:meth:`replaceAll` all already accept the same flexible shape for their own
            non-version-values argument. Kept as public API for callers that want to convert once and
            reuse the result across several calls (e.g. ``GIMIParser.py``, filtering many hash/index
            values per parse against the same fixed non-version filter)
        
        Parameters
        ----------
        indexVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The raw, flexibly-shaped filter values to normalize -- ``None`` means "no values given at
            all" (every position wildcarded)
        
        Raises
        ------
        :class:`ValueError`
            If this instance wasn't constructed with 'nonVersionIndexNames'
        
        Returns
        -------
        List[Optional[Any]]
            The normalized, positional filter values
        """
    def addMap(self, assetMap: dict, rows: typing.Any = []) -> None:
        """
        Merges new entries into the existing adjacency list (see :attr:`map`) -- for any 'fromAsset'
        already present, new 'toAsset' values are appended after the existing ones, skipping any that are
        already present
        
        Parameters
        ----------
        assetMap: Dict[Any, List[Any]]
            The new adjacency entries to merge in
        
        rows: Union[List[Tuple[List[Any], Any]], dict]
            Any new rows needed to support 'assetMap' -- either a flat list or a real nested dict --
            if non-empty, added to :attr:`repo` first (matches the pure-Python original's ``addMap``,
            whose own ``assets`` argument is a nested dict in exactly this same shape)
        
            **Default**: ``[]``
        """
    def addRepoRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to :attr:`repo`, then rebuilds the reverse index to reflect them
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The rows to add -- either a flat list or a real nested dict -- see :meth:`ModDictAssets.addRows`
        """
    def get(self, nonVersionVals: collections.abc.Sequence[typing.Any], version: typing.Any = None, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the corresponding asset -- forwards directly to :attr:`repo`'s own :meth:`ModDictAssets.get`
        """
    def getKey(self, asset: typing.Any, fromVersion: typing.Any = None, fromNonVersionVals: typing.Any = None, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the key that produced 'asset', disambiguating between multiple candidates via
        'fromNonVersionVals' -- the first remaining candidate wins if more than one still matches after
        filtering
        
        Parameters
        ----------
        asset: Any
            The asset value to search for
        
        fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to search from -- see :meth:`hasFrom`
        
            **Default**: ``None``
        
        fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The non-version value filter -- see :meth:`hasFrom`
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching key is found
        
            **Default**: ``True``
        
        Raises
        ------
        :class:`KeyError`
            If no matching key is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        Optional[Tuple[Any, ...]]
            The found key, or ``None`` if none is found and 'errorOnNotFound' is ``False`` -- deliberately
            just the key, not the version it was resolved at (matching the exact contract real callers
            like GIMIParser rely on; see the C++ core's own note on this)
        """
    def hasFrom(self, asset: typing.Any, version: typing.Any = None, nonVersionVals: typing.Any = None) -> bool:
        """
        Determines whether 'asset' exists in the assets to map from
        
        Parameters
        ----------
        asset: Any
            The asset to search for
        
        version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to search from -- the latest available version is used if this is ``None``
        
            **Default**: ``None``
        
        nonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            A per-position filter over the candidate keys' non-version index values -- ``None`` at a
            position means "match any value there"; ``None`` for the whole argument means "no filtering
            at all". If :attr:`nonVersionIndexNames` was given, also accepts a bare value (filters only
            the first position) or a dict keyed by index name
        
            **Default**: ``None``
        """
    def replace(self, asset: typing.Any, fromVersion: typing.Any = None, fromNonVersionVals: typing.Any = None, toVersion: typing.Any = None, toAssetName: typing.Any, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the single corresponding asset to replace 'asset' with, for one specific target asset
        name
        
        Parameters
        ----------
        asset: Any
            The asset to be replaced
        
        fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace from -- see :meth:`getKey`
        
            **Default**: ``None``
        
        fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[str, Any]]]
            The non-version value filter -- see :meth:`getKey`
        
            **Default**: ``None``
        
        toVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace to -- the latest available version is used if this is ``None``
        
            **Default**: ``None``
        
        toAssetName: Any
            The specific name of the asset to map to
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if 'asset' (or a mapping for it) isn't found at all --
            once past that point, "toAssetName isn't actually mapped from asset's name" or "no data
            exists for it at the queried version" always just returns ``None``, regardless of this flag
        
            **Default**: ``True``
        
        Returns
        -------
        Any
            The replacement asset, or ``None`` if none is found
        """
    def replaceAll(self, asset: typing.Any, fromVersion: typing.Any = None, fromNonVersionVals: typing.Any = None, toVersion: typing.Any = None, toAssetNames: typing.Any = None, errorOnNotFound: bool = True) -> dict:
        """
        Retrieves every corresponding asset to replace 'asset' with
        
        Parameters
        ----------
        asset: Any
            The asset to be replaced
        
        fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace from -- see :meth:`getKey`
        
            **Default**: ``None``
        
        fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[str, Any]]]
            The non-version value filter -- see :meth:`getKey`
        
            **Default**: ``None``
        
        toVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace to -- the latest available version is used if this is ``None``
        
            **Default**: ``None``
        
        toAssetNames: Optional[List[Any]]
            The specific names of the assets to map to -- every asset name 'asset' maps to is used if
            this is ``None``
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if 'asset' (or a mapping for it) isn't found at all --
            see :meth:`replace`'s note on this parameter
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[Any, Any]
            The corresponding assets for the fix to replace, keyed by asset name -- empty if nothing is
            found
        """
    @property
    def fixFrom(self) -> set:
        """
        Set[Any]: Always empty -- matches the pure-Python original, which declares this but never
        populates it anywhere
        """
    @property
    def fixTo(self) -> set:
        """
        Set[Any]: Always empty -- matches the pure-Python original, which declares this but never
        populates it anywhere
        """
    @property
    def fromAssets(self) -> list[typing.Any]:
        """
        List[Any]: Every asset value that has at least one known originating key -- a property (not a
        method), matching the pure-Python original's contract exactly (real callers, e.g. IniFile.py's
        ``type.hashes.fromAssets``, access it as one)
        """
    @property
    def map(self) -> dict:
        """
        Dict[Any, List[Any]]: The adjacency list mapping assets to fix from to assets to fix to
        """
    @property
    def nonVersionIndexNames(self) -> typing.Any:
        """
        Optional[List[:class:`str`]]: The names of the non-version index columns, in position order --
        ``None`` if this instance wasn't constructed with them (see the constructor's own note)
        """
    @property
    def repo(self) -> ModDictAssets:
        """
        :class:`ModDictAssets`: The underlying asset data
        """
class ModTypeId:
    """
    
    The names of the different types of mods this fix will fix from or fix to
    
    Mirrors the keys of the pure-Python ``ModTypeNames`` enum (``constants/ModTypeNames.py``)
        
    
    Members:
    
      Amber : Amber from GI
    
      AmberCN : Amber Chinese version from GI
    
      Ayaka : Ayaka from GI
    
      AyakaSpringbloom : Ayaka Fontaine skin from GI
    
      Arlecchino : Arlecchino from GI
    
      ArlecchinoBoss : The first phase of the Arlecchino boss from GI
    
      Barbara : Barbara from GI
    
      BarbaraSummertime : Barbara summer skin from GI
    
      CherryHuTao : Hu Tao Lantern Rite skin from GI
    
      Diluc : Diluc from GI
    
      DilucFlamme : Diluc Red Dead of the Night skin from GI
    
      Fischl : Fischl from GI
    
      FischlHighness : Fischl summer skin from GI
    
      Ganyu : Ganyu from GI
    
      GanyuTwilight : Ganyu Lantern Rite skin from GI
    
      HuTao : HuTao from GI
    
      Jean : Jean from GI
    
      JeanCN : Jean Chinese version from GI
    
      JeanSea : Jean summer skin from GI
    
      Kaeya : Kaeya from GI
    
      KaeyaSailwind : KaeyaSailwind from GI
    
      Keqing : Keqing from GI
    
      KeqingOpulent : Keqing Lantern Rite skin from GI
    
      Kirara : Kirara from GI
    
      KiraraBoots : Kirara summer skin from GI
    
      Klee : Klee from GI
    
      KleeBlossomingStarlight : Klee summer skin from GI
    
      Lisa : Lisa from GI
    
      LisaStudent : Lisa Sumeru skin from GI
    
      Mona : Mona from GI
    
      MonaCN : Mona Chinese version from GI
    
      Nilou : Nilou from GI
    
      NilouBreeze : Nilou summer skin from GI
    
      Ningguang : Ningguang from GI
    
      NingguangOrchid : Ningguang Lantern Rite from GI
    
      Raiden : Ei from GI
    
      RaidenBoss : The first phase of the Raiden Shogun boss from GI
    
      Rosaria : Rosaria from GI
    
      RosariaCN : Rosaria Chinese version from GI
    
      Shenhe : Shenhe from GI
    
      ShenheFrostFlower : Shenhe Lantern Rite skin from GI
    
      Xiangling : Xiangling from GI
    
      XianglingCheer : Xiangling Lantern Rite skin from GI
    
      Xingqiu : Xingqiu from GI
    
      XingqiuBamboo : Xingqiu Lantern Rite skin from GI
    """
    Amber: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Amber: 0>
    AmberCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.AmberCN: 1>
    Arlecchino: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Arlecchino: 4>
    ArlecchinoBoss: typing.ClassVar[ModTypeId]  # value = <ModTypeId.ArlecchinoBoss: 5>
    Ayaka: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Ayaka: 2>
    AyakaSpringbloom: typing.ClassVar[ModTypeId]  # value = <ModTypeId.AyakaSpringbloom: 3>
    Barbara: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Barbara: 6>
    BarbaraSummertime: typing.ClassVar[ModTypeId]  # value = <ModTypeId.BarbaraSummertime: 7>
    CherryHuTao: typing.ClassVar[ModTypeId]  # value = <ModTypeId.CherryHuTao: 8>
    Diluc: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Diluc: 9>
    DilucFlamme: typing.ClassVar[ModTypeId]  # value = <ModTypeId.DilucFlamme: 10>
    Fischl: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Fischl: 11>
    FischlHighness: typing.ClassVar[ModTypeId]  # value = <ModTypeId.FischlHighness: 12>
    Ganyu: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Ganyu: 13>
    GanyuTwilight: typing.ClassVar[ModTypeId]  # value = <ModTypeId.GanyuTwilight: 14>
    HuTao: typing.ClassVar[ModTypeId]  # value = <ModTypeId.HuTao: 15>
    Jean: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Jean: 16>
    JeanCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.JeanCN: 17>
    JeanSea: typing.ClassVar[ModTypeId]  # value = <ModTypeId.JeanSea: 18>
    Kaeya: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Kaeya: 19>
    KaeyaSailwind: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KaeyaSailwind: 20>
    Keqing: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Keqing: 21>
    KeqingOpulent: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KeqingOpulent: 22>
    Kirara: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Kirara: 23>
    KiraraBoots: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KiraraBoots: 24>
    Klee: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Klee: 25>
    KleeBlossomingStarlight: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KleeBlossomingStarlight: 26>
    Lisa: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Lisa: 27>
    LisaStudent: typing.ClassVar[ModTypeId]  # value = <ModTypeId.LisaStudent: 28>
    Mona: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Mona: 29>
    MonaCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.MonaCN: 30>
    Nilou: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Nilou: 31>
    NilouBreeze: typing.ClassVar[ModTypeId]  # value = <ModTypeId.NilouBreeze: 32>
    Ningguang: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Ningguang: 33>
    NingguangOrchid: typing.ClassVar[ModTypeId]  # value = <ModTypeId.NingguangOrchid: 34>
    Raiden: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Raiden: 35>
    RaidenBoss: typing.ClassVar[ModTypeId]  # value = <ModTypeId.RaidenBoss: 36>
    Rosaria: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Rosaria: 37>
    RosariaCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.RosariaCN: 38>
    Shenhe: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Shenhe: 39>
    ShenheFrostFlower: typing.ClassVar[ModTypeId]  # value = <ModTypeId.ShenheFrostFlower: 40>
    Xiangling: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Xiangling: 41>
    XianglingCheer: typing.ClassVar[ModTypeId]  # value = <ModTypeId.XianglingCheer: 42>
    Xingqiu: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Xingqiu: 43>
    XingqiuBamboo: typing.ClassVar[ModTypeId]  # value = <ModTypeId.XingqiuBamboo: 44>
    __members__: typing.ClassVar[dict[str, ModTypeId]]  # value = {'Amber': <ModTypeId.Amber: 0>, 'AmberCN': <ModTypeId.AmberCN: 1>, 'Ayaka': <ModTypeId.Ayaka: 2>, 'AyakaSpringbloom': <ModTypeId.AyakaSpringbloom: 3>, 'Arlecchino': <ModTypeId.Arlecchino: 4>, 'ArlecchinoBoss': <ModTypeId.ArlecchinoBoss: 5>, 'Barbara': <ModTypeId.Barbara: 6>, 'BarbaraSummertime': <ModTypeId.BarbaraSummertime: 7>, 'CherryHuTao': <ModTypeId.CherryHuTao: 8>, 'Diluc': <ModTypeId.Diluc: 9>, 'DilucFlamme': <ModTypeId.DilucFlamme: 10>, 'Fischl': <ModTypeId.Fischl: 11>, 'FischlHighness': <ModTypeId.FischlHighness: 12>, 'Ganyu': <ModTypeId.Ganyu: 13>, 'GanyuTwilight': <ModTypeId.GanyuTwilight: 14>, 'HuTao': <ModTypeId.HuTao: 15>, 'Jean': <ModTypeId.Jean: 16>, 'JeanCN': <ModTypeId.JeanCN: 17>, 'JeanSea': <ModTypeId.JeanSea: 18>, 'Kaeya': <ModTypeId.Kaeya: 19>, 'KaeyaSailwind': <ModTypeId.KaeyaSailwind: 20>, 'Keqing': <ModTypeId.Keqing: 21>, 'KeqingOpulent': <ModTypeId.KeqingOpulent: 22>, 'Kirara': <ModTypeId.Kirara: 23>, 'KiraraBoots': <ModTypeId.KiraraBoots: 24>, 'Klee': <ModTypeId.Klee: 25>, 'KleeBlossomingStarlight': <ModTypeId.KleeBlossomingStarlight: 26>, 'Lisa': <ModTypeId.Lisa: 27>, 'LisaStudent': <ModTypeId.LisaStudent: 28>, 'Mona': <ModTypeId.Mona: 29>, 'MonaCN': <ModTypeId.MonaCN: 30>, 'Nilou': <ModTypeId.Nilou: 31>, 'NilouBreeze': <ModTypeId.NilouBreeze: 32>, 'Ningguang': <ModTypeId.Ningguang: 33>, 'NingguangOrchid': <ModTypeId.NingguangOrchid: 34>, 'Raiden': <ModTypeId.Raiden: 35>, 'RaidenBoss': <ModTypeId.RaidenBoss: 36>, 'Rosaria': <ModTypeId.Rosaria: 37>, 'RosariaCN': <ModTypeId.RosariaCN: 38>, 'Shenhe': <ModTypeId.Shenhe: 39>, 'ShenheFrostFlower': <ModTypeId.ShenheFrostFlower: 40>, 'Xiangling': <ModTypeId.Xiangling: 41>, 'XianglingCheer': <ModTypeId.XianglingCheer: 42>, 'Xingqiu': <ModTypeId.Xingqiu: 43>, 'XingqiuBamboo': <ModTypeId.XingqiuBamboo: 44>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class ModTypeIdTools:
    """
    
    Tools for handling :class:`ModTypeId`
        
    """
    @staticmethod
    def getEnum(value: typing.SupportsInt | typing.SupportsIndex) -> FixRaidenBoss2.core.ModTypeId | None:
        """
        Retrieves the corresponding :class:`ModTypeId` for some integer value, checking that the value
        actually corresponds to one of :class:`ModTypeId`'s declared values
        
        Parameters
        ----------
        value: :class:`int`
            The integer value to convert
        
        Returns
        -------
        Optional[:class:`ModTypeId`]
            The corresponding :class:`ModTypeId`, if 'value' is valid
        """
    @staticmethod
    def getName(value: ModTypeId) -> str:
        """
        Retrieves the corresponding name for a :class:`ModTypeId`
        
        Parameters
        ----------
        value: :class:`ModTypeId`
            The :class:`ModTypeId` to retrieve the name for
        
        Returns
        -------
        :class:`str`
            The name for 'value'
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
class ParseContext:
    """
    
    Context for parsing some text
    
    Parameters
    ----------
    src: Union[:class:`str`, List[:class:`str`]]
        The source text to parse
    
        If this argument is a list, then assumes that the lines of the source text is given
    
        **Default**: ``""``
    
    file: Optional[:class:`str`]
        The file path to the source text
    
        **Default**: ``None``
    
    startLineNo: :class:`int`
        The starting line of the source text
    
        **Default**: ``1``
        
    """
    @typing.overload
    def __init__(self, src: str = '', file: str | None = None, startLineNo: typing.SupportsInt | typing.SupportsIndex = 1) -> None:
        ...
    @typing.overload
    def __init__(self, src: collections.abc.Sequence[str], file: str | None = None, startLineNo: typing.SupportsInt | typing.SupportsIndex = 1) -> None:
        ...
    def getEndLineNo(self) -> int:
        """
        Retrieves the line number after the last line
        
        Returns
        -------
        :class:`int`
            The ending line number after the last line
        """
    @property
    def file(self) -> str | None:
        """
        Optional[:class:`str`]: The file path to the source text
        """
    @file.setter
    def file(self, arg0: str | None) -> None:
        ...
    @property
    def lines(self) -> list[str]:
        """
        List[:class:`str`]: The lines of the source text
        """
    @lines.setter
    def lines(self, arg0: collections.abc.Sequence[str]) -> None:
        ...
    @property
    def startLineNo(self) -> int:
        """
        :class:`int`: The starting line of the source text
        """
    @startLineNo.setter
    def startLineNo(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
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
class SympyTokenizer(FilteredTokenizer):
    """
    
    This class inherits from :class:`FilteredTokenizer`
    
    The tokenizer used for a subset of the string representation of a `sympy logic query`_
    
    eg.
    
    .. code-block::
        :linenos:
    
        ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))
    
    Parameters
    ----------
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, setup: bool = True) -> None:
        ...
class Token:
    """
    
    A token when parsing some language
    
    Parameters
    ----------
    type: Optional[:class:`str`]
        The name for the type of token, if available
    
    val: :class:`str`
        The value of the token
    
    lineNo: :class:`int`
        The line number the token belongs to
    
    charNo: :class:`int`
        The character number the token belongs to within some line
        
    """
    def __init__(self, type: str | None, val: str, lineNo: typing.SupportsInt | typing.SupportsIndex, charNo: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def charNo(self) -> int:
        """
        :class:`int`: The character number the token belongs to within some line
        """
    @charNo.setter
    def charNo(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def lineNo(self) -> int:
        """
        :class:`int`: The line number the token belongs to
        """
    @lineNo.setter
    def lineNo(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def type(self) -> str | None:
        """
        Optional[:class:`str`]: The name for the type of token, if available
        """
    @type.setter
    def type(self, arg0: str | None) -> None:
        ...
    @property
    def val(self) -> str:
        """
        :class:`str`: The value of the token
        """
    @val.setter
    def val(self, arg0: str) -> None:
        ...
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
