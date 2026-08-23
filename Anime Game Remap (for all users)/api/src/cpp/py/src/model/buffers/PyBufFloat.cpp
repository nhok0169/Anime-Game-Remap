#include "PyBufFloat.h"

#include <string>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBufFloat(pybind11::module_ &m) {
    // Full replacement of the pure-Python originals (api/src/py/FixRaidenBoss2/model/buffers/BufFloat.py),
    // now deleted -- registered under their bare names directly.
    py::class_<AGRC::BufBaseFloat, AGRC::BufDataType, py::smart_holder>(m, "BufBaseFloat", R"doc(
This class inherits from :class:`BufDataType`

The type definition for a generic 32-bit IEEE 754 `floating point`_ number within a ``.buf`` file
    )doc")

        .def(py::init<std::string, std::size_t, bool>(),
    py::arg("name"), py::arg("size"), py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new `floating point`_ type

Parameters
----------
name: :class:`str`
    The name of the type

size: :class:`int`
    The byte size for the data type

isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));

    py::class_<AGRC::BufFloat, AGRC::BufBaseFloat, py::smart_holder>(m, "BufFloat", R"doc(
This class inherits from :class:`BufBaseFloat`

The type definition for a 32-bit `floating point`_ number within a ``.buf`` file
    )doc")

        .def(py::init<bool>(), py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new 32-bit `floating point`_ type

Parameters
----------
isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));

    py::class_<AGRC::BufFloat16, AGRC::BufBaseFloat, py::smart_holder>(m, "BufFloat16", R"doc(
This class inherits from :class:`BufBaseFloat`

The type definition for a 16-bit `half precision floating point`_ number within a ``.buf`` file
    )doc")

        .def(py::init<bool>(), py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new 16-bit `half precision floating point`_ type

Parameters
----------
isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));
}
