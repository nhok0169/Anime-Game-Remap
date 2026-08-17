# distutils: name = CyHashTools
# cython: language_level=3, boundscheck=False, wraparound=False, nonecheck=False, cdivision=True


import json

from cpython.dict cimport PyDict_Check
from cpython.list cimport PyList_Check
from cpython.tuple cimport PyTuple_Check


cdef class CyHashTools():
    """
    Cython tools for handling with hashing
    """

    cdef object _serialize(self, object obj):
        cdef dict serializedDict
        cdef list serializedList
        cdef object item
        cdef object key
        cdef object value

        if (obj is None or isinstance(obj, (int, float, str, bool))):
            return obj

        if PyList_Check(obj) or PyTuple_Check(obj):
            serializedList = []
            for item in obj:
                serializedList.append(self._serialize(item))
            return serializedList

        if PyDict_Check(obj):
            serializedDict = {}
            for key, value in sorted((<dict>obj).items()):
                serializedDict[key] = self._serialize(value)
            return serializedDict

        # Handle custom objects
        serializedDict = {}
        for key, value in sorted(obj.__dict__.items()):
            if not key.startswith('_'):
                serializedDict[key] = self._serialize(value)
        return serializedDict

    def hashLibSerialize(self, object obj):
        """
        Converts some hashable into deterministic bytes, suitable as the ``data``/``str``
        parameter for :class:`CppHashTools`'s hashing methods

        Parameters
        ----------
        obj: Hashable
            The object to convert

        Returns
        -------
        :class:`bytes`
            The resultant bytes converted from the object
        """

        cdef object serializedData = self._serialize(obj)
        return json.dumps(serializedData, sort_keys = True).encode('utf-8')
