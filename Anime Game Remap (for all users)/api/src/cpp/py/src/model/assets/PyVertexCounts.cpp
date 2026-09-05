#include "PyVertexCounts.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "PyModDictAssets.h"
#include "../PyVersion.h"


namespace {

    // The two non-version columns, in index order -- VertexCounts' own class note. "component" is
    // "" on every shipped row (a real key value, not a "missing" marker).
    const std::vector<std::string> &nonVersionIndexNames() {
        static const std::vector<std::string> names = {"name", "component"};
        return names;
    }

    const std::vector<std::string> &versionIndexNames() {
        static const std::vector<std::string> names = {"version"};
        return names;
    }

    /**
     * Normalizes a flexible non-version-values argument (bare value / list / dict keyed by index
     * name / None) into the complete, wildcard-free key ModDictAssets::get demands.
     *
     * A position the caller left unspecified becomes "", not a wildcard: ModDictAssets hashes the
     * whole key tuple, so it has no way to express "any" there (see Architecture's note on the
     * asset family). "" is also exactly what the C++ ModType::getVertexCount passes for the
     * component, and what every shipped row carries -- so a caller passing just a mod's name (the
     * pure-Python original's only calling convention) still resolves to that mod's overall count.
     */
    std::vector<std::string> toFullKey(const py::object &raw) {
        std::vector<std::optional<std::string>> wildcards = toWildcardList(raw, nonVersionIndexNames());

        std::vector<std::string> result;
        result.reserve(wildcards.size());
        for (const std::optional<std::string> &val : wildcards) {
            result.push_back(val.has_value() ? *val : std::string{});
        }
        return result;
    }

    std::optional<AGRC::Version> toSingleVersion(const py::object &raw) {
        // One version column, but the argument still accepts the same shapes as every other asset
        // class's (a bare value, a 1-element list, or {"version": ...}).
        std::vector<std::optional<std::string>> wildcards = toWildcardList(raw, versionIndexNames());
        if (wildcards.empty() || !wildcards[0].has_value()) {
            return std::nullopt;
        }
        return parseVersionArg(py::cast(*wildcards[0]));
    }

    std::vector<AGRC::Row<std::string, int>> convertIntRows(const py::object &rowsOrNestedDict, std::size_t totalIndices);

    void flattenIntNestedDictNode(const py::object &node, std::vector<std::string> &path, std::size_t depth,
                                  std::size_t totalIndices, std::vector<AGRC::Row<std::string, int>> &rows) {
        if (depth == totalIndices) {
            rows.push_back(AGRC::Row<std::string, int>{path, node.cast<int>()});
            return;
        }

        if (!py::isinstance<py::dict>(node)) {
            throw py::value_error("VertexCounts.addRows: expected a dict at depth " + std::to_string(depth)
                                  + " (totalIndices=" + std::to_string(totalIndices) + ")");
        }

        py::dict asDict = node.cast<py::dict>();
        for (auto item : asDict) {
            path.push_back(py::str(item.first).cast<std::string>());
            flattenIntNestedDictNode(py::reinterpret_borrow<py::object>(item.second), path, depth + 1, totalIndices, rows);
            path.pop_back();
        }
    }

    std::vector<AGRC::Row<std::string, int>> convertIntRows(const py::object &rowsOrNestedDict, std::size_t totalIndices) {
        std::vector<AGRC::Row<std::string, int>> rows;

        if (py::isinstance<py::dict>(rowsOrNestedDict)) {
            std::vector<std::string> path;
            flattenIntNestedDictNode(rowsOrNestedDict, path, 0, totalIndices, rows);
            return rows;
        }

        auto flat = rowsOrNestedDict.cast<std::vector<std::pair<std::vector<std::string>, int>>>();
        rows.reserve(flat.size());
        for (auto &row : flat) {
            rows.push_back(AGRC::Row<std::string, int>{std::move(row.first), row.second});
        }
        return rows;
    }

}


void initCppVertexCounts(pybind11::module_ &m) {
    py::class_<AGRC::VertexCounts, py::smart_holder>(m, "VertexCounts", R"doc(
Class for managing the vertex counts of a mod, pre-populated with this project's real vertex
count data

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the :meth:`get` method are:

    * version (version index)
    * name
    * component

    ``component`` is ``""`` on every row the software currently ships, so a caller wanting a mod's
    overall count can simply leave it out -- an unspecified non-version column is filled in with
    ``""``, not treated as a wildcard (this table is hashed on the whole key, so it has no
    wildcards to give)
    )doc")

        .def(py::init<>(), py::doc(R"doc(
Constructs a new, fully-populated vertex count lookup table

:raw-html:`<br />`

.. note::
    Unlike the pure-Python original there is no 'repo' argument to swap the whole table out with --
    nothing in this project ever passed one, and :meth:`addRows` already covers extending it
        )doc"))

        .def("get", [](const AGRC::VertexCounts &self, const py::object &nonVersionVals, const py::object &versionVals,
                       bool errorOnNotFound, const py::object &default_) -> py::object {
            std::optional<int> result = self.get(toFullKey(nonVersionVals), toSingleVersion(versionVals), false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No matching vertex count found for the given non-version values");
                }
                return default_;
            }
            return py::cast(*result);
        }, py::arg("nonVersionVals"), py::arg("versionVals") = py::none(), py::arg("errorOnNotFound") = true,
           py::arg("default") = py::none(), py::doc(R"doc(
Retrieves the corresponding vertex count

Parameters
----------
nonVersionVals: Union[Any, List[Any], Dict[:class:`str`, Any]]
    The values of the index columns that do not reference a version -- a bare value (taken as
    ``name``), a positional list, or a dict keyed by index name. Any column left unspecified is
    filled in with ``""``

versionVals: Optional[Union[Any, List[Any], Dict[:class:`str`, Any]]]
    The version to query at -- the latest available version for the key is used if this is ``None``

    **Default**: ``None``

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if no matching vertex count is found

    **Default**: ``True``

default: Any
    If 'errorOnNotFound' is ``False``, the value to return when nothing is found

    **Default**: ``None``

Raises
------
:class:`KeyError`
    If no matching vertex count is found and 'errorOnNotFound' is ``True``

Returns
-------
:class:`int`
    The found vertex count, or 'default' if none is found and 'errorOnNotFound' is ``False``
        )doc"))

        .def("addRows", [](AGRC::VertexCounts &self, const py::object &rows) {
            self.addRows(convertIntRows(rows, self.getTotalIndices()));
        }, py::arg("rows"), py::doc(R"doc(
Adds new rows to the table, overwriting the value of any row whose full key (every non-version
index value, plus its parsed version) already exists

Parameters
----------
rows: Union[List[Tuple[List[:class:`str`], :class:`int`]], dict]
    The rows to add -- either a flat list of ``(indexVals, count)`` tuples, or a real nested dict
    exactly :attr:`totalIndices` levels deep (``{version: {name: {component: count}}}``)

Raises
------
:class:`ValueError`
    If the nesting depth doesn't match :attr:`totalIndices`, or a row's version value fails to parse
        )doc"))

        .def_property_readonly("totalIndices", &AGRC::VertexCounts::getTotalIndices,
            py::doc(R"doc(:class:`int`: The total number of index columns (including the version index))doc"))

        .def_property_readonly("versionIndexPos", &AGRC::VertexCounts::getVersionIndexPos,
            py::doc(R"doc(:class:`int`: The position (0-based) of the version index within a row's index values)doc"))

        .def("__len__", &AGRC::VertexCounts::size,
            py::doc(R"doc(The total number of rows currently in the table)doc"))

        // A fresh py::class_ supports neither copy.copy() nor copy.deepcopy() unless bound --
        // see Architecture's note. Both are real call sites' expectations for these asset tables.
        .def("clone", [](const AGRC::VertexCounts &self) { return AGRC::VertexCounts(self); },
            py::doc(R"doc(
Creates an independent copy of this table

Returns
-------
:class:`VertexCounts`
    The copied table
            )doc"))

        .def("__copy__", [](const AGRC::VertexCounts &self) { return AGRC::VertexCounts(self); })

        .def("__deepcopy__", [](const AGRC::VertexCounts &self, const py::dict &) { return AGRC::VertexCounts(self); },
            py::arg("memo"));
}
