#include "PyHash128.h"

#include <cstdint>
#include <string>

#include "AGRemapCore/tools/hashing/Hash128.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppHash128(pybind11::module_ &m) {
    py::class_<AGRC::Hash128>(m, "Hash128", R"doc(
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
    )doc")

        .def(py::init<>(), py::doc(R"doc(Constructs a hash with both halves set to 0)doc"))

        .def(py::init<std::uint64_t, std::uint64_t>(), py::arg("high"), py::arg("low"), py::doc(R"doc(
Constructs a hash from its 2 64-bit halves

Parameters
----------
high: :class:`int`
    The high 64 bits of the hash

low: :class:`int`
    The low 64 bits of the hash
        )doc"))

        .def_static("hash", [](py::bytes data) {
            std::string bytes = data;
            return AGRC::Hash128::hash(bytes.data(), bytes.size());
        }, py::arg("data"), py::doc(R"doc(
Deterministically hashes a buffer of bytes

Parameters
----------
data: :class:`bytes`
    The buffer of bytes to hash

Returns
-------
:class:`Hash128`
    The resultant hash
        )doc"))

        .def_static("hash", [](const std::string &str) {
            return AGRC::Hash128::hash(str);
        }, py::arg("str"), py::doc(R"doc(
Deterministically hashes a string

Parameters
----------
str: :class:`str`
    The string to hash

Returns
-------
:class:`Hash128`
    The resultant hash
        )doc"))

        .def_property_readonly("high", &AGRC::Hash128::getHigh, py::doc(R"doc(
:class:`int`: The high 64 bits of the hash
        )doc"))

        .def_property_readonly("low", &AGRC::Hash128::getLow, py::doc(R"doc(
:class:`int`: The low 64 bits of the hash
        )doc"))

        .def("toHexString", &AGRC::Hash128::toHexString, py::doc(R"doc(
Converts the hash to a fixed-length, lowercase hex string

Returns
-------
:class:`str`
    The hex string
        )doc"))

        .def("toBase64", &AGRC::Hash128::toBase64, py::doc(R"doc(
Converts the hash to a fixed-length base64 string

Returns
-------
:class:`str`
    The base64 string
        )doc"))

        .def("__eq__", [](const AGRC::Hash128 &self, const AGRC::Hash128 &other) { return self == other; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' store the same hash value)doc"))

        .def("__ne__", [](const AGRC::Hash128 &self, const AGRC::Hash128 &other) { return self != other; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' store different hash values)doc"))

        .def("__lt__", [](const AGRC::Hash128 &self, const AGRC::Hash128 &other) { return self < other; }, py::arg("other"),
    py::doc(R"doc(An arbitrary but consistent (and deterministic) total ordering)doc"))

        .def("__hash__", [](const AGRC::Hash128 &self) { return self.hashCode(); },
    py::doc(R"doc(Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set)doc"))

        .def("__repr__", [](const AGRC::Hash128 &self) { return "Hash128('" + self.toHexString() + "')"; })

        .def("__str__", &AGRC::Hash128::toHexString);
}
