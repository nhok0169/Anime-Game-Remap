#include "PyHash64.h"

#include <cstdint>
#include <string>

#include "AGRemapCore/tools/hashing/Hash64.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppHash64(pybind11::module_ &m) {
    py::class_<AGRC::Hash64>(m, "Hash64", R"doc(
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
    )doc")

        .def(py::init<>(), py::doc(R"doc(Constructs a hash with a value of 0)doc"))

        .def(py::init<std::uint64_t>(), py::arg("value"), py::doc(R"doc(
Constructs a hash from its raw 64-bit value

Parameters
----------
value: :class:`int`
    The raw 64-bit value of the hash
        )doc"))

        .def_static("hash", [](py::bytes data) {
            std::string bytes = data;
            return AGRC::Hash64::hash(bytes.data(), bytes.size());
        }, py::arg("data"), py::doc(R"doc(
Deterministically hashes a buffer of bytes

Parameters
----------
data: :class:`bytes`
    The buffer of bytes to hash

Returns
-------
:class:`Hash64`
    The resultant hash
        )doc"))

        .def_static("hash", [](const std::string &str) {
            return AGRC::Hash64::hash(str);
        }, py::arg("str"), py::doc(R"doc(
Deterministically hashes a string

Parameters
----------
str: :class:`str`
    The string to hash

Returns
-------
:class:`Hash64`
    The resultant hash
        )doc"))

        .def_property_readonly("value", &AGRC::Hash64::getValue, py::doc(R"doc(
:class:`int`: The raw 64-bit value of the hash
        )doc"))

        .def("toHexString", &AGRC::Hash64::toHexString, py::doc(R"doc(
Converts the hash to a fixed-length, lowercase hex string

Returns
-------
:class:`str`
    The hex string
        )doc"))

        .def("toBase64", &AGRC::Hash64::toBase64, py::doc(R"doc(
Converts the hash to a fixed-length base64 string

Returns
-------
:class:`str`
    The base64 string
        )doc"))

        .def("__eq__", [](const AGRC::Hash64 &self, const AGRC::Hash64 &other) { return self == other; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' store the same hash value)doc"))

        .def("__ne__", [](const AGRC::Hash64 &self, const AGRC::Hash64 &other) { return self != other; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' store different hash values)doc"))

        .def("__lt__", [](const AGRC::Hash64 &self, const AGRC::Hash64 &other) { return self < other; }, py::arg("other"),
    py::doc(R"doc(An arbitrary but consistent (and deterministic) total ordering)doc"))

        .def("__hash__", [](const AGRC::Hash64 &self) { return self.hashCode(); },
    py::doc(R"doc(Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set)doc"))

        .def("__repr__", [](const AGRC::Hash64 &self) { return "Hash64('" + self.toHexString() + "')"; })

        .def("__str__", &AGRC::Hash64::toHexString);
}
