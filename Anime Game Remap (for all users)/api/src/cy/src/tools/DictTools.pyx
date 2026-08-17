# distutils: name = CyDictTools
# cython: language_level=3, boundscheck=False, wraparound=False, nonecheck=False, cdivision=True


import numpy as np
cimport numpy as np

from cpython.dict cimport PyDict_Next, PyDict_Check, PyDict_Contains
from cpython.object cimport PyObject



cdef class CyDictTools():
    """
    Cython tools for handling with Dictionaries
    """

    cpdef np.ndarray nestedDictToNdArray(self, dict nestedDict, list colNames):
        """
        Transforms a nested dictionary into a `numpy array`_

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

        cdef int maxDepth = len(colNames)

        # 1. count leaves
        cdef list stack = [(nestedDict, 0)]
        cdef object nodeObj
        cdef PyObject *key
        cdef PyObject *value
        cdef Py_ssize_t pos
        cdef int leafCount = 0

        while stack:
            nodeObj, _ = stack.pop()

            # Pass the Python object directly. Cython handles the C translation.
            if PyDict_Check(nodeObj):
                pos = 0
                while PyDict_Next(<dict>nodeObj, &pos, &key, &value):
                    stack.append((<object>value, 0))
            else:
                leafCount += 1

        # 2. allocate array for the result
        cdef np.ndarray[object, ndim=2] arr = np.empty(
            (leafCount, maxDepth),
            dtype=object
        )

        # 3. traverse the tree and fill the array
        cdef list stack2 = [(None, nestedDict, 0)]
        cdef list pathBuffer = [None] * maxDepth

        cdef int depth
        cdef int i = 0
        cdef int row = 0
        cdef object nodeKey

        while stack2:
            nodeKey, nodeObj, depth = stack2.pop()

            if depth > 0:
                pathBuffer[depth - 1] = nodeKey

            if PyDict_Check(nodeObj):
                pos = 0
                while PyDict_Next(<dict>nodeObj, &pos, &key, &value):
                    stack2.append((<object>key, <object>value, depth + 1))
            else:
                for i in range(depth):
                    arr[row, i] = pathBuffer[i]

                arr[row, depth] = nodeObj
                row += 1

        return arr

    cpdef void forDict(self, object nestedDict, list keyNames, object func, bint ordered = True):
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

        ordered: :class:`bool`
            Whether to visit leaves in the same order the keys/values were inserted into ``nestedDict`` :raw-html:`<br />` :raw-html:`<br />`

            Setting this to ``False`` skips the extra bookkeeping needed to preserve order, which is slightly
            faster but the traversal order becomes unspecified :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``
        """

        cdef int maxDepth = len(keyNames)

        # stack of (key at this level, value/subdict at this level, depth)
        cdef list stack = [(None, nestedDict, 0)]
        cdef object nodeKey
        cdef object nodeObj
        cdef PyObject *key
        cdef PyObject *value
        cdef Py_ssize_t pos
        cdef int depth
        cdef int i

        # reused buffers holding the current path's keys/values at each depth
        cdef list keyBuffer = [None] * maxDepth
        cdef list valueBuffer = [None] * maxDepth
        cdef list children

        while stack:
            nodeKey, nodeObj, depth = stack.pop()

            if depth > 0:
                keyBuffer[depth - 1] = nodeKey
                valueBuffer[depth - 1] = nodeObj

            if depth >= maxDepth:
                func(
                    {keyNames[i]: keyBuffer[i] for i in range(maxDepth)},
                    {keyNames[i]: valueBuffer[i] for i in range(maxDepth)}
                )
                continue

            if PyDict_Check(nodeObj):
                pos = 0

                if ordered:
                    children = []
                    while PyDict_Next(<dict>nodeObj, &pos, &key, &value):
                        children.append((<object>key, <object>value, depth + 1))

                    # push in reverse so the LIFO stack pops them back out in insertion order
                    for i in range(len(children) - 1, -1, -1):
                        stack.append(children[i])
                else:
                    while PyDict_Next(<dict>nodeObj, &pos, &key, &value):
                        stack.append((<object>key, <object>value, depth + 1))

    def iterDict(self, object nestedDict, list keyNames, bint leafOnly = False, bint ordered = True):
        """
        Iterates over a nested dictionary, yielding at each leaf node

        This is the generator equivalent of :meth:`forDict`, so no callback function is needed

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

        cdef int maxDepth = len(keyNames)

        # stack of (key at this level, value/subdict at this level, depth)
        cdef list stack = [(None, nestedDict, 0)]
        cdef object nodeKey
        cdef object nodeObj
        cdef PyObject *key
        cdef PyObject *value
        cdef Py_ssize_t pos
        cdef int depth
        cdef int i

        # reused buffers holding the current path's keys/values at each depth
        cdef list keyBuffer = [None] * maxDepth
        cdef list valueBuffer = [None] * maxDepth
        cdef list children

        while stack:
            nodeKey, nodeObj, depth = stack.pop()

            if depth > 0:
                keyBuffer[depth - 1] = nodeKey
                valueBuffer[depth - 1] = nodeObj

            if depth >= maxDepth:
                if leafOnly:
                    yield valueBuffer[maxDepth - 1] if maxDepth > 0 else nestedDict
                else:
                    yield (
                        {keyNames[i]: keyBuffer[i] for i in range(maxDepth)},
                        {keyNames[i]: valueBuffer[i] for i in range(maxDepth)}
                    )
                continue

            if PyDict_Check(nodeObj):
                pos = 0

                if ordered:
                    children = []
                    while PyDict_Next(<dict>nodeObj, &pos, &key, &value):
                        children.append((<object>key, <object>value, depth + 1))

                    # push in reverse so the LIFO stack pops them back out in insertion order
                    for i in range(len(children) - 1, -1, -1):
                        stack.append(children[i])
                else:
                    while PyDict_Next(<dict>nodeObj, &pos, &key, &value):
                        stack.append((<object>key, <object>value, depth + 1))

    def getVal(self, object dct, object keys, bint errorOnNotFound = False, object default = None):
        """
        Retrieves the corresponding value from a nested dictionary

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

        cdef object result = dct
        cdef object key

        try:
            if not PyDict_Check(result):
                raise KeyError()

            for key in keys:
                if not PyDict_Check(result):
                    raise KeyError(key)

                result = (<dict>result)[key]
        except KeyError as e:
            if (errorOnNotFound):
                raise e

            return default

        return result

    def contains(self, object dct, object keys):
        """
        Determines whether a path of keys exists within a nested dictionary

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

        cdef object result = dct
        cdef object key

        if not PyDict_Check(result):
            return False

        for key in keys:
            if not PyDict_Check(result):
                return False

            if not PyDict_Contains(result, key):
                return False

            result = (<dict>result)[key]

        return True

    def setVal(self, object dct, object keys, object value):
        """
        Sets the value at a key-path within a nested dictionary, creating any missing
        intermediate :class:`dict` layers along the way (a "deep set")

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

        cdef list keysList
        cdef object node
        cdef object nextNode
        cdef object key
        cdef Py_ssize_t n
        cdef Py_ssize_t i

        if not PyDict_Check(dct):
            raise TypeError(f"'dct' must be a dict (or a subclass of it), got {type(dct).__name__} instead")

        keysList = list(keys)
        n = len(keysList)

        if n == 0:
            return

        node = dct
        for i in range(n - 1):
            key = keysList[i]

            if PyDict_Contains(node, key):
                nextNode = (<dict>node)[key]
            else:
                nextNode = None

            if not PyDict_Check(nextNode):
                nextNode = {}
                (<dict>node)[key] = nextNode

            node = nextNode

        (<dict>node)[keysList[n - 1]] = value

    def getKeys(self, list dictList, bint ordered = True):
        """
        Gets the unique keys found across a list of dictionaries

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

        cdef object d
        cdef object k
        cdef PyObject *key
        cdef PyObject *value
        cdef Py_ssize_t pos
        cdef set seen
        cdef list uniqueKeys

        if ordered:
            seen = set()
            uniqueKeys = []

            for d in dictList:
                if PyDict_Check(d):
                    pos = 0
                    while PyDict_Next(<dict>d, &pos, &key, &value):
                        k = <object>key
                        if k not in seen:
                            seen.add(k)
                            uniqueKeys.append(k)

            return uniqueKeys
        else:
            seen = set()

            for d in dictList:
                if PyDict_Check(d):
                    pos = 0
                    while PyDict_Next(<dict>d, &pos, &key, &value):
                        seen.add(<object>key)

            return list(seen)

    def getCommonKeys(self, list dictList, bint ordered = True):
        """
        Retrieves the intersection of the keys found across a list of dictionaries

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

        cdef object d
        cdef object k
        cdef PyObject *key
        cdef PyObject *value
        cdef Py_ssize_t pos
        cdef list validDicts
        cdef list orderedResult
        cdef object first
        cdef bint inAll
        cdef set result = None
        cdef set currentKeys

        if ordered:
            validDicts = [d for d in dictList if PyDict_Check(d)]
            if not validDicts:
                return []

            orderedResult = []
            first = validDicts[0]
            pos = 0
            while PyDict_Next(<dict>first, &pos, &key, &value):
                k = <object>key
                inAll = True
                for d in validDicts[1:]:
                    if not PyDict_Contains(d, k):
                        inAll = False
                        break

                if inAll:
                    orderedResult.append(k)

            return orderedResult

        for d in dictList:
            if not PyDict_Check(d):
                continue

            currentKeys = set()
            pos = 0
            while PyDict_Next(<dict>d, &pos, &key, &value):
                currentKeys.add(<object>key)

            if result is None:
                result = currentKeys
            else:
                result &= currentKeys

            if not result:
                break

        if result is None:
            result = set()

        return list(result)

    def getCommonPaths(self, list dictList, bint ordered = True):
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

        cdef list validDicts = [d for d in dictList if PyDict_Check(d)]
        cdef list result = []
        cdef list stack
        cdef list path
        cdef list nodes
        cdef list childNodes
        cdef list commonKeys
        cdef object node
        cdef object key
        cdef bint allDicts
        cdef Py_ssize_t i

        if not validDicts:
            return result

        stack = [([], validDicts)]
        while stack:
            path, nodes = stack.pop()

            allDicts = True
            for node in nodes:
                if not PyDict_Check(node):
                    allDicts = False
                    break

            if not allDicts:
                if path:
                    result.append(path)
                continue

            commonKeys = self.getCommonKeys(nodes, ordered = ordered)
            if not commonKeys:
                if path:
                    result.append(path)
                continue

            if ordered:
                # push in reverse so popping the stack yields keys in commonKeys' original order
                for i in range(len(commonKeys) - 1, -1, -1):
                    key = commonKeys[i]
                    childNodes = [(<dict>node)[key] for node in nodes]
                    stack.append((path + [key], childNodes))
            else:
                for key in commonKeys:
                    childNodes = [(<dict>node)[key] for node in nodes]
                    stack.append((path + [key], childNodes))

        return result

    def iterPaths(self, object dct):
        """
        Iterates over a nested dictionary, yielding at each leaf path

        This is the generator equivalent of applying :meth:`getCommonPaths` to a single
        dictionary: a "path" is the same notion used by :meth:`contains`/:meth:`getVal`/
        :meth:`setVal`/:meth:`getCommonPaths`, and only "leaf"/maximal paths are yielded -- a
        path that ends at either a non-:class:`dict` value, or an empty :class:`dict` (nothing
        further to descend into)

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

        cdef list stack
        cdef list path
        cdef object node
        cdef PyObject *keyPtr
        cdef PyObject *valuePtr
        cdef Py_ssize_t pos
        cdef object key
        cdef object value
        cdef list children
        cdef Py_ssize_t i

        stack = [([], dct)]
        while stack:
            path, node = stack.pop()

            if not PyDict_Check(node):
                if path:
                    yield path
                continue

            pos = 0
            children = []
            while PyDict_Next(<dict>node, &pos, &keyPtr, &valuePtr):
                key = <object>keyPtr
                value = <object>valuePtr
                children.append((path + [key], value))

            if not children:
                if path:
                    yield path
                continue

            for i in range(len(children) - 1, -1, -1):
                stack.append(children[i])

    def getPaths(self, object dct, bint ordered = True):
        """
        Retrieves all the maximal key-paths within a nested dictionary

        This is the eager, :class:`list`-returning equivalent of :meth:`iterPaths` -- a "path"
        is the same notion used by :meth:`contains`/:meth:`getVal`/:meth:`setVal`/
        :meth:`getCommonPaths`, and only "leaf"/maximal paths are retrieved -- a path that ends
        at either a non-:class:`dict` value, or an empty :class:`dict` (nothing further to
        descend into)

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

        cdef list result
        cdef list stack
        cdef list path
        cdef object node
        cdef PyObject *keyPtr
        cdef PyObject *valuePtr
        cdef Py_ssize_t pos
        cdef object key
        cdef object value
        cdef bint hasChildren

        if ordered:
            return list(self.iterPaths(dct))

        result = []
        stack = [([], dct)]
        while stack:
            path, node = stack.pop()

            if not PyDict_Check(node):
                if path:
                    result.append(path)
                continue

            hasChildren = False
            pos = 0
            while PyDict_Next(<dict>node, &pos, &keyPtr, &valuePtr):
                hasChildren = True
                key = <object>keyPtr
                value = <object>valuePtr
                stack.append((path + [key], value))

            if not hasChildren:
                if path:
                    result.append(path)

        return result

    cdef object _mergeMany(self, object target, list allDicts, object combineDuplicate):
        # Merges allDicts[1:] into target, where allDicts[0] represents target's own contents
        # (either target itself, or the dict target was just copied from). For any key found in
        # 2+ of allDicts, combineDuplicate(key, indexToValue) is called, where indexToValue only
        # has entries for the indices (into allDicts) that actually have that key.
        cdef object d
        cdef PyObject *keyPtr
        cdef PyObject *valuePtr
        cdef Py_ssize_t pos
        cdef object key
        cdef object value
        cdef dict occurrences
        cdef dict indexToValue
        cdef Py_ssize_t idx
        cdef Py_ssize_t onlyIdx

        if combineDuplicate is None:
            for d in allDicts[1:]:
                target.update(d)
            return target

        occurrences = {}
        idx = 0
        for d in allDicts:
            pos = 0
            while PyDict_Next(<dict>d, &pos, &keyPtr, &valuePtr):
                key = <object>keyPtr
                value = <object>valuePtr

                if key in occurrences:
                    occurrences[key][idx] = value
                else:
                    occurrences[key] = {idx: value}

            idx += 1

        for key, indexToValue in occurrences.items():
            if len(indexToValue) == 1:
                onlyIdx = next(iter(indexToValue))
                if onlyIdx == 0:
                    continue

                target[key] = indexToValue[onlyIdx]
            else:
                target[key] = combineDuplicate(key, indexToValue)

        return target

    def update(self, object srcDict, object newDict, object combineDuplicate = None):
        """
        Updates ``srcDict`` based off the new values from ``newDict``

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

            If this value is set to ``None``, then will use the value from ``newDict`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`TypeError`
            If ``srcDict`` or ``newDict`` is not a :class:`dict` (or a subclass of it)

        Returns
        -------
        Dict[Hashable, Any]
            Reference to the updated ``srcDict``
        """

        cdef Py_ssize_t srcLen
        cdef Py_ssize_t newLen
        cdef object shortDict
        cdef object longDict
        cdef bint srcIsShort
        cdef dict combinedValues
        cdef PyObject *keyPtr
        cdef PyObject *valuePtr
        cdef Py_ssize_t pos = 0
        cdef object key
        cdef object shortVal
        cdef object srcVal
        cdef object newVal

        if not PyDict_Check(srcDict):
            raise TypeError(f"'srcDict' must be a dict (or a subclass of it), got {type(srcDict).__name__} instead")

        if not PyDict_Check(newDict):
            raise TypeError(f"'newDict' must be a dict (or a subclass of it), got {type(newDict).__name__} instead")

        if combineDuplicate is None:
            srcDict.update(newDict)
            return srcDict

        srcLen = len(srcDict)
        newLen = len(newDict)

        if srcLen <= newLen:
            shortDict = srcDict
            longDict = newDict
            srcIsShort = True
        else:
            shortDict = newDict
            longDict = srcDict
            srcIsShort = False

        combinedValues = {}
        while PyDict_Next(<dict>shortDict, &pos, &keyPtr, &valuePtr):
            key = <object>keyPtr
            if PyDict_Contains(longDict, key):
                shortVal = <object>valuePtr

                if srcIsShort:
                    srcVal = shortVal
                    newVal = (<dict>longDict)[key]
                else:
                    srcVal = (<dict>longDict)[key]
                    newVal = shortVal

                combinedValues[key] = combineDuplicate(key, srcVal, newVal)

        srcDict.update(newDict)
        srcDict.update(combinedValues)

        return srcDict

    def updateMany(self, object srcDict, list dictList, object combineDuplicate = None):
        """
        Updates ``srcDict`` based off the new values from a list of dictionaries

        This is the same as :meth:`update`, generalized to more than one 'newDict' at once

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

        cdef list allDicts
        cdef object d

        if not PyDict_Check(srcDict):
            raise TypeError(f"'srcDict' must be a dict (or a subclass of it), got {type(srcDict).__name__} instead")

        allDicts = [srcDict]
        for d in dictList:
            if not PyDict_Check(d):
                raise TypeError(f"every entry in 'dictList' must be a dict (or a subclass of it), got {type(d).__name__} instead")

            allDicts.append(d)

        return self._mergeMany(srcDict, allDicts, combineDuplicate)

    def combine(self, object dict1, object dict2, object combineDuplicate = None, bint makeNewCopy = True):
        """
        Creates a dictionary from combining 2 dictionaries

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
            'dict1' itself, updated in place
        """

        cdef object result
        cdef object key
        cdef object dict2Val
        cdef PyObject *keyPtr
        cdef PyObject *valuePtr
        cdef Py_ssize_t pos = 0

        if not PyDict_Check(dict1):
            raise TypeError(f"'dict1' must be a dict (or a subclass of it), got {type(dict1).__name__} instead")

        if not PyDict_Check(dict2):
            raise TypeError(f"'dict2' must be a dict (or a subclass of it), got {type(dict2).__name__} instead")

        if makeNewCopy:
            result = dict(dict1)
        else:
            result = dict1

        if combineDuplicate is None:
            result.update(dict2)
            return result

        while PyDict_Next(<dict>dict2, &pos, &keyPtr, &valuePtr):
            key = <object>keyPtr
            dict2Val = <object>valuePtr

            if PyDict_Contains(dict1, key):
                result[key] = combineDuplicate(key, dict1[key], dict2Val)
            else:
                result[key] = dict2Val

        return result

    def combineMany(self, object dict1, list dictList, object combineDuplicate = None, bint makeNewCopy = True):
        """
        Creates a dictionary from combining ``dict1`` with a list of dictionaries

        This is the same as :meth:`combine`, generalized to more than one 'dict2' at once

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

        cdef list allDicts
        cdef object d
        cdef object result

        if not PyDict_Check(dict1):
            raise TypeError(f"'dict1' must be a dict (or a subclass of it), got {type(dict1).__name__} instead")

        allDicts = [dict1]
        for d in dictList:
            if not PyDict_Check(d):
                raise TypeError(f"every entry in 'dictList' must be a dict (or a subclass of it), got {type(d).__name__} instead")

            allDicts.append(d)

        if makeNewCopy:
            result = dict(dict1)
        else:
            result = dict1

        return self._mergeMany(result, allDicts, combineDuplicate)