# distutils: name = CyDictTools
# cython: language_level=3, boundscheck=False, wraparound=False, nonecheck=False, cdivision=True


import numpy as np
cimport numpy as np

from cpython.dict cimport PyDict_Next, PyDict_Check
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