#include "PyVersion.h"

#include <stdexcept>
#include <string>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


std::optional<AGRC::Version> parseVersionArg(const py::object &raw) {
    if (raw.is_none()) {
        return std::nullopt;
    }
    if (py::isinstance<AGRC::Version>(raw)) {
        return raw.cast<AGRC::Version>();
    }

    std::string str = py::str(raw).cast<std::string>();
    std::optional<AGRC::Version> parsed = AGRC::Version::parse(str);
    if (!parsed.has_value()) {
        throw py::value_error("Invalid version: '" + str + "'");
    }
    return parsed;
}


void initCppVersion(pybind11::module_ &m) {
    // Bound as "CppVersion", not the bare "Version" -- FixRaidenBoss2.model.Version.Version
    // already exists as the bare name, but it plays a completely different role there (a
    // searchable *collection* of versions with closest-match lookup, this port's
    // AGRC::VersionSet) than this class (a single version *value*, closer in spirit to
    // packaging.version.Version). Binding this as bare "Version" would silently shadow the
    // existing, unrelated FixRaidenBoss2.Version at the package top level.
    py::class_<AGRC::Version>(m, "CppVersion", R"doc(
A single `PEP 440`_ version value -- a from-scratch C++ port of Python's `packaging.version.Version`_,
matching its parsing/normalization/comparison behaviour exactly (verified empirically against the
real ``packaging`` library during development, not just read off its source)

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: x == y

        Determines whether 'x' and 'y' are the same version

    .. describe:: x != y

        Determines whether 'x' and 'y' are different versions

    .. describe:: x < y, x <= y, x > y, x >= y

        Compares two versions following `PEP 440`_'s ordering rules

    .. describe:: hash(x)

        Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`

    .. describe:: str(x)

        Equivalent to ``x.toString()``
    )doc")

        .def_static("parse", &AGRC::Version::parse, py::arg("raw"), py::doc(R"doc(
Parses a raw version string

Parameters
----------
raw: :class:`str`
    The raw version string to parse

Returns
-------
Optional[:class:`CppVersion`]
    The parsed version, or ``None`` if 'raw' does not conform to `PEP 440`_ in any way
        )doc"))

        .def("toString", &AGRC::Version::toString, py::doc(R"doc(
Converts the version back into its normalized, round-trippable string form

Returns
-------
:class:`str`
    The string form of the version
        )doc"))

        .def_property_readonly("epoch", &AGRC::Version::getEpoch, py::doc(R"doc(:class:`int`: The epoch of the version (``0`` if none was specified))doc"))

        .def_property_readonly("release", &AGRC::Version::getRelease, py::doc(R"doc(
Tuple[:class:`int`, ...]: The numeric components of the release segment, in order, including any
trailing zeros (e.g. ``CppVersion.parse("2.0.0").release == (2, 0, 0)``)
        )doc"))

        .def_property_readonly("pre", &AGRC::Version::getPre, py::doc(R"doc(Optional[Tuple[:class:`str`, :class:`int`]]: The pre-release segment (normalized letter and number), or ``None`` if there is none)doc"))

        .def_property_readonly("post", &AGRC::Version::getPost, py::doc(R"doc(Optional[:class:`int`]: The post-release number, or ``None`` if there is none)doc"))

        .def_property_readonly("dev", &AGRC::Version::getDev, py::doc(R"doc(Optional[:class:`int`]: The dev-release number, or ``None`` if there is none)doc"))

        .def_property_readonly("local", &AGRC::Version::getLocal, py::doc(R"doc(Optional[:class:`str`]: The local version segment, dot-joined, or ``None`` if there is none)doc"))

        .def_property_readonly("is_prerelease", &AGRC::Version::isPrerelease, py::doc(R"doc(:class:`bool`: Whether this is a pre-release (has a pre-release or dev-release segment))doc"))

        .def_property_readonly("is_postrelease", &AGRC::Version::isPostrelease, py::doc(R"doc(:class:`bool`: Whether this is a post-release)doc"))

        .def_property_readonly("is_devrelease", &AGRC::Version::isDevrelease, py::doc(R"doc(:class:`bool`: Whether this is a dev-release)doc"))

        .def_property_readonly("major", &AGRC::Version::getMajor, py::doc(R"doc(:class:`int`: The first component of :attr:`release`, or ``0`` if unavailable)doc"))

        .def_property_readonly("minor", &AGRC::Version::getMinor, py::doc(R"doc(:class:`int`: The second component of :attr:`release`, or ``0`` if unavailable)doc"))

        .def_property_readonly("micro", &AGRC::Version::getMicro, py::doc(R"doc(:class:`int`: The third component of :attr:`release`, or ``0`` if unavailable)doc"))

        .def_property_readonly("public", &AGRC::Version::getPublic, py::doc(R"doc(:class:`str`: :meth:`toString` without the local segment)doc"))

        .def_property_readonly("base_version", &AGRC::Version::getBaseVersion, py::doc(R"doc(:class:`str`: The epoch and release segment only, with no pre/post/dev/local segment)doc"))

        .def("__eq__", [](const AGRC::Version &self, const AGRC::Version &other) { return self == other; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' are the same version)doc"))

        .def("__ne__", [](const AGRC::Version &self, const AGRC::Version &other) { return self != other; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' are different versions)doc"))

        .def("__lt__", [](const AGRC::Version &self, const AGRC::Version &other) { return self < other; }, py::arg("other"))

        .def("__le__", [](const AGRC::Version &self, const AGRC::Version &other) { return self <= other; }, py::arg("other"))

        .def("__gt__", [](const AGRC::Version &self, const AGRC::Version &other) { return self > other; }, py::arg("other"))

        .def("__ge__", [](const AGRC::Version &self, const AGRC::Version &other) { return self >= other; }, py::arg("other"))

        .def("__hash__", [](const AGRC::Version &self) { return std::hash<AGRC::Version>{}(self); },
    py::doc(R"doc(Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set)doc"))

        .def("__repr__", [](const AGRC::Version &self) { return "CppVersion('" + self.toString() + "')"; })

        .def("__str__", &AGRC::Version::toString);
}
