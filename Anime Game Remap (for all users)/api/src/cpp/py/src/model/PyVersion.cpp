// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "PyVersion.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "AGRemapCore/model/VersionSet.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// The lenient half of parseVersionArg: an argument that names no version is None, not an error --
// what VersionSet.getVersion and VersionSet.add have always done with one.
std::optional<AGRC::Version> tryVersionArg(const py::object &raw) {
    if (raw.is_none()) {
        return std::nullopt;
    }
    if (py::isinstance<AGRC::Version>(raw)) {
        return raw.cast<AGRC::Version>();
    }
    return AGRC::Version::parse(py::str(raw).cast<std::string>());
}


std::vector<AGRC::Version> parseVersionList(const py::iterable &versions) {
    std::vector<AGRC::Version> result;
    for (py::handle version : versions) {
        result.push_back(*parseVersionArg(py::reinterpret_borrow<py::object>(version)));
    }
    return result;
}

}


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
    // One version VALUE. A searchable collection of them is VersionSet, below -- the pure-Python
    // 'Version' class used to be the collection, and the two names were split when it was replaced.
    py::class_<AGRC::Version>(m, "Version", R"doc(
A single `PEP 440`_ version value -- a C++ implementation of Python's `packaging.version.Version`_,
matching its parsing/normalization/comparison behaviour exactly

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
Optional[:class:`Version`]
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
trailing zeros (e.g. ``Version.parse("2.0.0").release == (2, 0, 0)``)
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

        .def("__repr__", [](const AGRC::Version &self) { return "Version('" + self.toString() + "')"; })

        .def("__str__", &AGRC::Version::toString);

    py::class_<AGRC::VersionSet>(m, "VersionSet", R"doc(
A set of available :class:`Version`\s, for finding the closest available version to some queried
version

Wherever a version is taken, a :class:`str`, :class:`int` or :class:`float` naming one is accepted too

Parameters
----------
versions: Optional[List[Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]]]
    The versions available :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init([](const py::object &versions) {
            auto result = std::make_unique<AGRC::VersionSet>();
            if (!versions.is_none()) {
                for (const AGRC::Version &version : parseVersionList(versions.cast<py::iterable>())) {
                    result->add(version);
                }
            }
            return result;
        }), py::arg("versions") = py::none())

        .def_property("versions", &AGRC::VersionSet::getVersions,
            [](AGRC::VersionSet &self, const py::iterable &versions) {
                std::vector<AGRC::Version> parsed = parseVersionList(versions);
                self.clear();
                for (const AGRC::Version &version : parsed) {
                    self.add(version);
                }
            },
            py::doc(R"doc(
The available versions

:getter: The versions in sorted ascending order, without duplicates
:setter: Replaces every version, and clears the closest-version cache
:type: List[:class:`Version`]
            )doc"))

        .def_property_readonly("latestVersion", &AGRC::VersionSet::getLatestVersion,
            py::doc(R"doc(Optional[:class:`Version`]: The latest version available, or ``None`` if there is none)doc"))

        .def("clear", &AGRC::VersionSet::clear, py::doc(R"doc(
Clears all the version data, including the closest-version cache
        )doc"))

        .def("add", [](AGRC::VersionSet &self, const py::object &newVersion) {
            std::optional<AGRC::Version> parsed = tryVersionArg(newVersion);
            if (parsed.has_value()) {
                self.add(*parsed);
            }
        }, py::arg("newVersion"), py::doc(R"doc(
Adds a new version

.. note::
    A 'newVersion' that names no valid version is ignored, rather than raising

.. warning::
    Does **not** invalidate the closest-version cache :meth:`findClosest` keeps, so a query cached
    before this call may still answer with a version that is no longer the closest. Call
    :meth:`clear` first, or pass ``fromCache = False``, when that matters

Parameters
----------
newVersion: Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]
    The new version to add
        )doc"))

        .def("findClosest", [](AGRC::VersionSet &self, const py::object &version, bool fromCache) {
            return self.findClosest(parseVersionArg(version), fromCache);
        }, py::arg("version") = py::none(), py::arg("fromCache") = true, py::doc(R"doc(
Finds the closest version available: the largest available version that is not greater than
'version', or the smallest available version if every one of them is

Parameters
----------
version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]]
    The version to be searched :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

fromCache: :class:`bool`
    Whether to use (and fill) the cache of earlier answers :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

Raises
------
ValueError
    If 'version' names no valid version

Returns
-------
Optional[:class:`Version`]
    The closest version available, or ``None`` if there are no versions available
        )doc"))

        .def_static("getVersion", &tryVersionArg, py::arg("rawVersion"), py::doc(R"doc(
Retrieves the version an argument names

Parameters
----------
rawVersion: Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]
    The version to translate

Returns
-------
Optional[:class:`Version`]
    The corresponding version, or ``None`` if 'rawVersion' names no valid version
        )doc"))

        .def_static("compareVersions", [](const AGRC::Version &version1, const AGRC::Version &version2) {
            return (version1 == version2) ? 0 : ((version1 < version2) ? -1 : 1);
        }, py::arg("version1"), py::arg("version2"), py::doc(R"doc(
Compares two versions

Parameters
----------
version1: :class:`Version`
    The first version to compare

version2: :class:`Version`
    The second version to compare

Returns
-------
:class:`int`
    A negative number if 'version1' is less than 'version2', a positive number if 'version1' is
    greater than 'version2', and zero if they are equal
        )doc"))

        .def_static("findClosestFromSortedList", [](const py::iterable &versions, const py::object &version) -> std::optional<AGRC::Version> {
            std::vector<AGRC::Version> parsed = parseVersionList(versions);
            if (parsed.empty()) {
                return std::nullopt;
            }

            std::optional<AGRC::Version> target = parseVersionArg(version);
            if (!target.has_value()) {
                return parsed.back();
            }
            return AGRC::VersionSet::findClosestFromSorted(parsed, *target);
        }, py::arg("versions"), py::arg("version"), py::doc(R"doc(
Finds the closest version available from a sorted list of versions, by the same rule as
:meth:`findClosest`

Parameters
----------
versions: List[Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]]
    The list of versions to search, sorted in ascending order

version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]]
    The version to be searched :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then will assume we want the latest version

Returns
-------
Optional[:class:`Version`]
    The closest version available, or ``None`` if 'versions' is empty
        )doc"))

        .def_static("findClosestFromList", [](const py::iterable &versions, const py::object &version) -> std::optional<AGRC::Version> {
            std::optional<AGRC::Version> target = parseVersionArg(version);
            std::optional<AGRC::Version> result;
            for (const AGRC::Version &current : parseVersionList(versions)) {
                if (target.has_value() && *target < current) {
                    continue;
                }
                if (!result.has_value() || *result < current) {
                    result = current;
                }
            }
            return result;
        }, py::arg("versions"), py::arg("version"), py::doc(R"doc(
Finds the latest version in an unsorted list of versions that is not greater than 'version'

.. note::
    Unlike :meth:`findClosestFromSortedList`, there is no fallback to the smallest version: if
    every version in 'versions' is greater than 'version', there is no answer

Parameters
----------
versions: List[Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]]
    The list of versions to search

version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`Version`]]
    The version to be searched :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then will assume we want the latest version

Returns
-------
Optional[:class:`Version`]
    The version found, or ``None`` if there is none
        )doc"));
}
