#include "PyHashTools.h"

#include <string>

#include "AGRemapCore/tools/hashing/HashTools.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppHashTools(pybind11::module_ &m) {
    // Bound as "CppHashTools" (not the bare "HashTools") since that bare name is already taken
    // by the deprecated pure-Python FixRaidenBoss2.tools.HashTools.HashTools -- see this
    // project's "Cpp prefix" naming convention.
    py::class_<AGRC::HashTools>(m, "CppHashTools", "C++ tools for deterministically hashing data")

        .def_static("getDeterministicHash", [](py::bytes data) {
            std::string bytes = data;
            return AGRC::HashTools::getDeterministicHash(bytes.data(), bytes.size());
        }, py::arg("data"), py::doc(R"doc(
Deterministically hashes a buffer of bytes

Parameters
----------
data: :class:`bytes`
    The buffer of bytes to hash

Returns
-------
:class:`Hash128`
    The resultant deterministic hash
        )doc"))

        .def_static("getDeterministicHash", [](const std::string &str) {
            return AGRC::HashTools::getDeterministicHash(str);
        }, py::arg("str"), py::doc(R"doc(
Deterministically hashes a string

Parameters
----------
str: :class:`str`
    The string to hash

Returns
-------
:class:`Hash128`
    The resultant deterministic hash
        )doc"))

        .def_static("getDeterministicHashStr", [](py::bytes data) {
            std::string bytes = data;
            return AGRC::HashTools::getDeterministicHashStr(bytes.data(), bytes.size());
        }, py::arg("data"), py::doc(R"doc(
Deterministically hashes a buffer of bytes

Parameters
----------
data: :class:`bytes`
    The buffer of bytes to hash

Returns
-------
:class:`str`
    The resultant deterministic hash, as a base64 string (see :meth:`Hash128.toBase64`)
        )doc"))

        .def_static("getDeterministicHashStr", [](const std::string &str) {
            return AGRC::HashTools::getDeterministicHashStr(str);
        }, py::arg("str"), py::doc(R"doc(
Deterministically hashes a string

Parameters
----------
str: :class:`str`
    The string to hash

Returns
-------
:class:`str`
    The resultant deterministic hash, as a base64 string (see :meth:`Hash128.toBase64`)
        )doc"))

        .def_static("getShortDeterministicHashStr", [](py::bytes data) {
            std::string bytes = data;
            return AGRC::HashTools::getShortDeterministicHashStr(bytes.data(), bytes.size());
        }, py::arg("data"), py::doc(R"doc(
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
        )doc"))

        .def_static("getShortDeterministicHashStr", [](const std::string &str) {
            return AGRC::HashTools::getShortDeterministicHashStr(str);
        }, py::arg("str"), py::doc(R"doc(
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
        )doc"))

        .def_static("clear", &AGRC::HashTools::clear, py::doc(R"doc(
Clears any saved internal state this class accumulates across calls (currently just the
collision-disambiguation frequency counts used by :meth:`getShortDeterministicHashStr`)
        )doc"));
}
