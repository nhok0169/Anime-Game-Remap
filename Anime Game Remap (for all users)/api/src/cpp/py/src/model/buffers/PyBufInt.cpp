#include "PyBufInt.h"

#include <string>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufInt.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBufInt(pybind11::module_ &m) {
    // Full replacement of the pure-Python originals (api/src/py/FixRaidenBoss2/model/buffers/BufInt.py),
    // now deleted -- registered under their bare names directly.
    py::class_<AGRC::BufBaseInt, AGRC::BufDataType, py::smart_holder>(m, "BufBaseInt", R"doc(
This class inherits from :class:`BufDataType`

The type definition for some generic integer type within a ``.buf`` file, at most 8 bytes wide
(see :class:`BufDataType`'s class-level warning)
    )doc")

        .def(py::init<std::string, std::size_t, bool, bool>(),
    py::arg("name"), py::arg("size"), py::arg("isBigEndian") = false, py::arg("isSigned") = true, py::doc(R"doc(
Constructs a new integer type

Parameters
----------
name: :class:`str`
    The name of the type

size: :class:`int`
    The byte size for the data type

isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``

isSigned: :class:`bool`
    Whether the type is signed. **Default**: ``True``
        )doc"))

        .def_property_readonly("isSigned", &AGRC::BufBaseInt::getIsSigned, py::doc(R"doc(
:class:`bool`: Whether the data type is signed
        )doc"));

    py::class_<AGRC::BufSignedInt, AGRC::BufBaseInt, py::smart_holder>(m, "BufSignedInt", R"doc(
This class inherits from :class:`BufBaseInt`

The type definition for some signed integer type within a ``.buf`` file
    )doc")

        .def(py::init<std::string, std::size_t, bool>(),
    py::arg("name") = "SignedInt32", py::arg("size") = 4, py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new signed integer type

Parameters
----------
name: :class:`str`
    The name of the type. **Default**: ``"SignedInt32"``

size: :class:`int`
    The byte size for the data type. **Default**: ``4``

isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));

    py::class_<AGRC::BufUnSignedInt, AGRC::BufBaseInt, py::smart_holder>(m, "BufUnSignedInt", R"doc(
This class inherits from :class:`BufBaseInt`

The type definition for some unsigned integer type within a ``.buf`` file
    )doc")

        .def(py::init<std::string, std::size_t, bool>(),
    py::arg("name") = "UnsignedInt32", py::arg("size") = 4, py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new unsigned integer type

Parameters
----------
name: :class:`str`
    The name of the type. **Default**: ``"UnsignedInt32"``

size: :class:`int`
    The byte size for the data type. **Default**: ``4``

isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));
}
