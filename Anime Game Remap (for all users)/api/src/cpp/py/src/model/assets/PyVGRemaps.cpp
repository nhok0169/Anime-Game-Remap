#include "PyVGRemaps.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "PyModDictAssets.h"
#include "../PyVersion.h"


namespace {

    // The four non-version columns and the two version columns, both in index order -- VGRemaps'
    // own class note (full index order is fromVersion, fromChar, fromComp, toVersion, toChar,
    // toComp).
    const std::vector<std::string> &nonVersionIndexNames() {
        static const std::vector<std::string> names = {"fromChar", "fromComp", "toChar", "toComp"};
        return names;
    }

    const std::vector<std::string> &versionIndexNames() {
        static const std::vector<std::string> names = {"fromVersion", "toVersion"};
        return names;
    }

    std::vector<std::optional<AGRC::Version>> toVersionList(const py::object &raw) {
        std::vector<std::optional<std::string>> wildcards = toWildcardList(raw, versionIndexNames());

        std::vector<std::optional<AGRC::Version>> result;
        result.reserve(wildcards.size());
        for (const std::optional<std::string> &val : wildcards) {
            result.push_back(val.has_value() ? parseVersionArg(py::cast(*val)) : std::nullopt);
        }
        return result;
    }

    /**
     * A leaf value may arrive either as an already-bound VGRemap or as the plain
     * ``{fromIndex: toIndex}`` dict one is built from -- real callers (and the pure-Python
     * original's own data/tests) use both, and a dict is not implicitly convertible here.
     */
    AGRC::VGRemap toVGRemap(const py::object &raw) {
        if (py::isinstance<AGRC::VGRemap>(raw)) {
            return raw.cast<AGRC::VGRemap>();
        }

        if (py::isinstance<py::dict>(raw)) {
            return AGRC::VGRemap(raw.cast<std::unordered_map<long long, long long>>());
        }

        throw py::value_error("VGRemaps: expected a VGRemap or a dict of {fromIndex: toIndex}, got "
                              + py::str(raw.get_type()).cast<std::string>());
    }

    void flattenRemapNestedDictNode(const py::object &node, std::vector<std::string> &path, std::size_t depth,
                                    std::size_t totalIndices, std::vector<AGRC::Row<std::string, AGRC::VGRemap>> &rows) {
        if (depth == totalIndices) {
            rows.push_back(AGRC::Row<std::string, AGRC::VGRemap>{path, toVGRemap(node)});
            return;
        }

        if (!py::isinstance<py::dict>(node)) {
            throw py::value_error("VGRemaps.addRows: expected a dict at depth " + std::to_string(depth)
                                  + " (totalIndices=" + std::to_string(totalIndices) + ")");
        }

        py::dict asDict = node.cast<py::dict>();
        for (auto item : asDict) {
            path.push_back(py::str(item.first).cast<std::string>());
            flattenRemapNestedDictNode(py::reinterpret_borrow<py::object>(item.second), path, depth + 1, totalIndices, rows);
            path.pop_back();
        }
    }

    std::vector<AGRC::Row<std::string, AGRC::VGRemap>> convertRemapRows(const py::object &rowsOrNestedDict,
                                                                       std::size_t totalIndices) {
        std::vector<AGRC::Row<std::string, AGRC::VGRemap>> rows;

        if (py::isinstance<py::dict>(rowsOrNestedDict)) {
            std::vector<std::string> path;
            flattenRemapNestedDictNode(rowsOrNestedDict, path, 0, totalIndices, rows);
            return rows;
        }

        for (auto item : rowsOrNestedDict) {
            py::tuple row = py::reinterpret_borrow<py::object>(item).cast<py::tuple>();
            rows.push_back(AGRC::Row<std::string, AGRC::VGRemap>{
                row[0].cast<std::vector<std::string>>(),
                toVGRemap(py::reinterpret_borrow<py::object>(row[1]))});
        }

        return rows;
    }

}


void initCppVGRemaps(pybind11::module_ &m) {
    py::class_<AGRC::VGRemaps, py::smart_holder>(m, "VGRemaps", R"doc(
Class to handle the Vertex Group Remaps of a mod, pre-populated with this project's real remap data

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the :meth:`get` method are:

    * fromVersion (version index)
    * fromChar
    * fromComp
    * toVersion (version index)
    * toChar
    * toComp

    A non-version column left unspecified is a **wildcard** (match anything there), and a version
    column left unspecified resolves to the latest available value among the rows still matching
    everything resolved before it
    )doc")

        .def(py::init<>(), py::doc(R"doc(
Constructs a new, fully-populated vertex group remap table

:raw-html:`<br />`

.. note::
    Unlike the pure-Python original there is no 'repo' argument -- nothing in this project passed
    one, and :meth:`addRows` already covers extending the table. Note that
    :attr:`ModDataAssets.VGRemaps` hands out a **shared** instance, so mutating that one is visible
    to every :class:`ModType` that fell back to it; construct one directly for an independent table
        )doc"))

        .def("get", [](const AGRC::VGRemaps &self, const py::object &nonVersionVals, const py::object &versionVals,
                       bool errorOnNotFound, const py::object &default_) -> py::object {
            std::optional<AGRC::VGRemap> result = self.get(toWildcardList(nonVersionVals, nonVersionIndexNames()),
                                                           toVersionList(versionVals), false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No matching vertex group remap found for the given non-version/version values");
                }
                return default_;
            }
            return py::cast(*result);
        }, py::arg("nonVersionVals") = py::none(), py::arg("versionVals") = py::none(),
           py::arg("errorOnNotFound") = true, py::arg("default") = py::none(), py::doc(R"doc(
Retrieves the corresponding vertex group remap

Parameters
----------
nonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
    The values of the index columns that do not reference a version -- a bare value (taken as
    ``fromChar``), a positional list, or a dict keyed by index name. Any column left unspecified
    matches anything there

    **Default**: ``None``

versionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
    The versions to query at, same accepted shapes. A column left unspecified resolves to the
    latest available value for it :raw-html:`<br />` :raw-html:`<br />`

    The two version columns are resolved sequentially, in index order -- ``fromVersion``'s
    floor-match narrows the candidate rows before ``toVersion`` is resolved against them

    **Default**: ``None``

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if no matching remap is found

    **Default**: ``True``

default: Any
    If 'errorOnNotFound' is ``False``, the value to return when nothing is found

    **Default**: ``None``

Raises
------
:class:`KeyError`
    If no matching remap is found and 'errorOnNotFound' is ``True``

Returns
-------
:class:`VGRemap`
    The found remap, or 'default' if none is found and 'errorOnNotFound' is ``False``
        )doc"))

        .def("addRows", [](AGRC::VGRemaps &self, const py::object &rows) {
            self.addRows(convertRemapRows(rows, self.getTotalIndices()));
        }, py::arg("rows"), py::doc(R"doc(
Adds new rows to the table, overwriting the value of any row whose full key already exists

Parameters
----------
rows: Union[List[Tuple[List[:class:`str`], Any]], dict]
    The rows to add -- either a flat list of ``(indexVals, remap)`` tuples, or a real nested dict
    exactly :attr:`totalIndices` levels deep
    (``{fromVersion: {fromChar: {fromComp: {toVersion: {toChar: {toComp: remap}}}}}}``)
    :raw-html:`<br />` :raw-html:`<br />`

    Each leaf may be either a :class:`VGRemap` or the plain ``{fromIndex: toIndex}`` dict one is
    built from

Raises
------
:class:`ValueError`
    If the nesting depth doesn't match :attr:`totalIndices`, a leaf is neither a :class:`VGRemap`
    nor a dict, or a row's version value fails to parse
        )doc"))

        .def_property_readonly("totalIndices", &AGRC::VGRemaps::getTotalIndices,
            py::doc(R"doc(:class:`int`: The total number of index columns)doc"))

        .def_property_readonly("versionColumnCount", &AGRC::VGRemaps::getVersionColumnCount,
            py::doc(R"doc(:class:`int`: The number of version columns)doc"))

        .def_property_readonly("nonVersionColumnCount", &AGRC::VGRemaps::getNonVersionColumnCount,
            py::doc(R"doc(:class:`int`: The number of non-version columns)doc"))

        .def("__len__", &AGRC::VGRemaps::size,
            py::doc(R"doc(The total number of rows currently in the table)doc"))

        // Real call sites deep-copy these tables (test_Mod.py copies the shared
        // ModDataAssets.VGRemaps before mutating it) -- a fresh py::class_ has neither
        // copy.copy() nor copy.deepcopy() until they're bound. See Architecture's note.
        .def("clone", [](const AGRC::VGRemaps &self) { return AGRC::VGRemaps(self); },
            py::doc(R"doc(
Creates an independent copy of this table

Returns
-------
:class:`VGRemaps`
    The copied table
            )doc"))

        .def("__copy__", [](const AGRC::VGRemaps &self) { return AGRC::VGRemaps(self); })

        .def("__deepcopy__", [](const AGRC::VGRemaps &self, const py::dict &) { return AGRC::VGRemaps(self); },
            py::arg("memo"));
}
