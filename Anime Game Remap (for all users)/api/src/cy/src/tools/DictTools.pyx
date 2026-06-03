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