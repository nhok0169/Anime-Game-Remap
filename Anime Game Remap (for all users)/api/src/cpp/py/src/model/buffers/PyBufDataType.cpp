#include "PyBufDataType.h"

#include <pybind11/stl.h>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufType.h"
#include "PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBufDataType(pybind11::module_ &m) {
    // Registered under the bare 'BufDataType' name -- full replacement of the pure-Python
    // original (api/src/py/FixRaidenBoss2/model/buffers/BufDataType.py), now deleted.
    py::class_<AGRC::BufDataType, AGRC::BufType, py::smart_holder>(m, "BufDataType", R"doc(
This class inherits from :class:`BufType`

The abstract base for an elementary data type within a ``.buf`` file (eg. a single integer or
`floating point`_ number) -- a real format is one of :class:`BufBaseInt`'s or
:class:`BufBaseFloat`'s concrete subclasses, or :class:`BufUnorm`

.. warning::
    Unlike the pure-Python original this replaces (where any subclass could be defined in plain
    Python and used immediately), a brand-new elementary data type not already covered by one of
    this class's existing subclasses needs a real C++ subclass and a rebuild of this extension --
    ``decode``/``encode`` are not overridable from pure Python here
    )doc")

        .def_property("size", &AGRC::BufDataType::getSize, &AGRC::BufDataType::setSize, py::doc(R"doc(
:class:`int`: The byte size for the data type (at most 8 bytes)

Raises
------
:class:`ValueError`
    If set to ``0`` or a value greater than ``8``
        )doc"))

        .def_property("isBigEndian", &AGRC::BufDataType::getIsBigEndian, &AGRC::BufDataType::setIsBigEndian, py::doc(R"doc(
:class:`bool`: The `endianness`_ for the data type
        )doc"))

        // Bound once here, on the abstract base -- every concrete subclass (BufBaseInt,
        // BufBaseFloat, BufUnorm, ...) inherits this method through normal Python attribute
        // resolution, and the call dispatches through the real C++ vtable to whichever leaf
        // override applies. See Architecture/CLAUDE.md's note on binding an inherited method
        // directly via a base pointer-to-member when the base only ever appears as 'self'.
        //
        // Bound via a lambda (not '&AGRC::BufDataType::decode' directly) so 'src'/the return
        // value go through real Python 'bytes' rather than <pybind11/stl.h>'s default
        // vector<uint8_t> <-> list-of-ints mapping -- see PyBufBytes.h's own comment.
        .def("decode", [](const AGRC::BufDataType &self, const py::bytes &src) {
            return self.decode(pyBytesToVec(src));
        }, py::arg("src"), py::doc(R"doc(
Decode the raw bytes to the required format for the type

.. warning::
    Please make sure the number of bytes passed into 'src' matches :attr:`size`

Parameters
----------
src: :class:`bytes`
    The raw bytes to decode

Returns
-------
Union[:class:`int`, :class:`float`]
    The decoded value for the type
        )doc"))

        .def("encode", [](const AGRC::BufDataType &self, const AGRC::BufValue &src) {
            return vecToPyBytes(self.encode(src));
        }, py::arg("src"), py::doc(R"doc(
Encodes the format of the type back to raw bytes

.. warning::
    Please make sure 'src' is within the acceptable range for the type

Parameters
----------
src: Union[:class:`int`, :class:`float`]
    The decoded value to encode

Returns
-------
:class:`bytes`
    The encoded raw bytes
        )doc"));
}
