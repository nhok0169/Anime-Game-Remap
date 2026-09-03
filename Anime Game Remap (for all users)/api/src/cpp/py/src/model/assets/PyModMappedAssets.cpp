#include "PyModMappedAssets.h"

#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../PyVersion.h"


template class AGRC::ModMappedAssets<std::string, std::string>;

PyObjectMap convertMap(const py::dict &mapDict) {
    PyObjectMap result;
    for (auto item : mapDict) {
        std::string key = py::str(item.first).cast<std::string>();
        std::vector<std::string> vals;
        for (auto v : py::reinterpret_borrow<py::object>(item.second)) {
            vals.push_back(py::str(v).cast<std::string>());
        }
        result.emplace(std::move(key), std::move(vals));
    }
    return result;
}

namespace {

    // fromNonVersionVals/nonVersionVals filter arguments: None means "no filtering at all"
    // (converts to an empty vector, matching AGRC::ModMappedAssets's own "empty = all
    // wildcard" convention); a list's elements are taken as real filter values, except a
    // None element, which means "wildcard at this position" -- a Pythonic None-based
    // alternative to the pure-Python original's UnHashableNone sentinel class, safe here
    // since every real index value in this project's actual data is a plain string/number
    // and None is never itself a legitimate index value.
    std::vector<std::optional<std::string>> convertNonVersionValsFilter(const py::object &raw) {
        if (raw.is_none()) {
            return {};
        }
        std::vector<std::optional<std::string>> result;
        for (auto item : raw) {
            py::object obj = py::reinterpret_borrow<py::object>(item);
            if (obj.is_none()) {
                result.push_back(std::nullopt);
            } else {
                result.push_back(py::str(obj).cast<std::string>());
            }
        }
        return result;
    }

    py::tuple toPyTuple(const std::vector<std::string> &vals) {
        py::tuple result(vals.size());
        for (std::size_t i = 0; i < vals.size(); ++i) {
            result[i] = vals[i];
        }
        return result;
    }

    std::vector<std::string> toVector(const py::object &raw) {
        std::vector<std::string> result;
        if (raw.is_none()) {
            return result;
        }
        for (auto item : raw) {
            result.push_back(py::str(item).cast<std::string>());
        }
        return result;
    }

    // Every method taking a non-version-values filter routes through here instead of calling
    // convertNonVersionValsFilter directly: when 'self' was built with nonVersionIndexNames (the
    // real Hashes/Indices case), 'raw' may be the flexible bare/list/dict shape real callers
    // still use (e.g. ModType.py's getHashRanges forwarding straight into Hashes.hasFrom) -- see
    // toWildcardList and PyModMappedAssets::nonVersionIndexNames. Without it (nonVersionIndexNames
    // unset -- any other/generic ModMappedAssets use), behaves exactly as before: an
    // already-positional list, or None for "no filtering at all".
    std::vector<std::optional<std::string>> resolveNonVersionValsFilter(const PyModMappedAssets &self, const py::object &raw) {
        if (self.nonVersionIndexNames.has_value()) {
            return toWildcardList(raw, *self.nonVersionIndexNames);
        }
        return convertNonVersionValsFilter(raw);
    }

}


void initCppModMappedAssets(pybind11::module_ &m) {
    py::class_<PyModMappedAssets>(m, "ModMappedAssets", R"doc(
Handles assets of any type where asset retrieval is based on a mapping -- a `bipartite graph`_
that maps assets to fix from to assets to fix to
    )doc")

        .def(py::init([](const PyModDictAssets &repo, const py::object &map, const py::object &nonVersionIndexNames) {
            auto result = std::make_unique<PyModMappedAssets>(repo, map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
            if (!nonVersionIndexNames.is_none()) {
                result->nonVersionIndexNames = nonVersionIndexNames.cast<std::vector<std::string>>();
            }
            return result;
        }), py::arg("repo"), py::arg("map") = py::none(), py::arg("nonVersionIndexNames") = py::none(), py::doc(R"doc(
Constructs a new mapped asset table

Parameters
----------
repo: :class:`ModDictAssets`
    The underlying asset data

map: Optional[Dict[Any, List[Any]]]
    The initial adjacency list mapping assets to fix from to assets to fix to

    **Default**: ``None``

nonVersionIndexNames: Optional[List[:class:`str`]]
    The names of 'repo''s non-version index columns, in position order -- when given, ``hasFrom``/
    ``getKey``/``replace``/``replaceAll``/``_convertNonVersionVals`` accept a flexible bare value,
    a list, or a dict keyed by one of these names for their non-version-values filter, instead of
    requiring an already-positional list. ``None`` (the default) keeps the strictly positional
    behaviour, appropriate for any use that isn't backed by named indices

    **Default**: ``None``
        )doc"))

        .def("addRepoRows", [](PyModMappedAssets &self, const py::object &rows) {
            self.addRepoRows(convertRowsOrNestedDict(rows, self.getRepo().getTotalIndices()));
        }, py::arg("rows"), py::doc(R"doc(
Adds new rows to :attr:`repo`, then rebuilds the reverse index to reflect them

Parameters
----------
rows: Union[List[Tuple[List[Any], Any]], dict]
    The rows to add -- either a flat list or a real nested dict -- see :meth:`ModDictAssets.addRows`
        )doc"))

        .def("addMap", [](PyModMappedAssets &self, const py::dict &assetMap, const py::object &rows) {
            self.addMap(convertMap(assetMap), convertRowsOrNestedDict(rows, self.getRepo().getTotalIndices()));
        }, py::arg("assetMap"), py::arg("rows") = py::list(), py::doc(R"doc(
Merges new entries into the existing adjacency list (see :attr:`map`) -- for any 'fromAsset'
already present, new 'toAsset' values are appended after the existing ones, skipping any that are
already present

Parameters
----------
assetMap: Dict[Any, List[Any]]
    The new adjacency entries to merge in

rows: Union[List[Tuple[List[Any], Any]], dict]
    Any new rows needed to support 'assetMap' -- either a flat list or a real nested dict --
    if non-empty, added to :attr:`repo` first (matches the pure-Python original's ``addMap``,
    whose own ``assets`` argument is a nested dict in exactly this same shape)

    **Default**: ``[]``
        )doc"))

        .def("get", [](const PyModMappedAssets &self, const std::vector<std::string> &nonVersionVals, const py::object &version, bool errorOnNotFound) -> py::object {
            std::optional<AGRC::Version> parsedVersion = parseVersionArg(version);
            std::optional<std::string> result = self.get(nonVersionVals, parsedVersion, false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No matching asset found for the given non-version values");
                }
                return py::none();
            }
            return py::cast(*result);
        }, py::arg("nonVersionVals"), py::arg("version") = py::none(), py::arg("errorOnNotFound") = true, py::doc(R"doc(
Retrieves the corresponding asset -- forwards directly to :attr:`repo`'s own :meth:`ModDictAssets.get`
        )doc"))

        .def("hasFrom", [](const PyModMappedAssets &self, const std::string &asset, const py::object &version, const py::object &nonVersionVals) {
            return self.hasFrom(asset, parseVersionArg(version), resolveNonVersionValsFilter(self, nonVersionVals));
        }, py::arg("asset"), py::arg("version") = py::none(), py::arg("nonVersionVals") = py::none(), py::doc(R"doc(
Determines whether 'asset' exists in the assets to map from

Parameters
----------
asset: Any
    The asset to search for

version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The version to search from -- the latest available version is used if this is ``None``

    **Default**: ``None``

nonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
    A per-position filter over the candidate keys' non-version index values -- ``None`` at a
    position means "match any value there"; ``None`` for the whole argument means "no filtering
    at all". If :attr:`nonVersionIndexNames` was given, also accepts a bare value (filters only
    the first position) or a dict keyed by index name

    **Default**: ``None``
        )doc"))

        .def("getKey", [](const PyModMappedAssets &self, const std::string &asset, const py::object &fromVersion, const py::object &fromNonVersionVals, bool errorOnNotFound) -> py::object {
            auto result = self.getKey(asset, parseVersionArg(fromVersion), resolveNonVersionValsFilter(self, fromNonVersionVals), false);
            if (!result.has_value()) {
                if (errorOnNotFound) {
                    throw py::key_error("No key found for the given asset");
                }
                return py::none();
            }
            return toPyTuple(*result);
        }, py::arg("asset"), py::arg("fromVersion") = py::none(), py::arg("fromNonVersionVals") = py::none(), py::arg("errorOnNotFound") = true, py::doc(R"doc(
Retrieves the key that produced 'asset', disambiguating between multiple candidates via
'fromNonVersionVals' -- the first remaining candidate wins if more than one still matches after
filtering

Parameters
----------
asset: Any
    The asset value to search for

fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The version to search from -- see :meth:`hasFrom`

    **Default**: ``None``

fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
    The non-version value filter -- see :meth:`hasFrom`

    **Default**: ``None``

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if no matching key is found

    **Default**: ``True``

Raises
------
:class:`KeyError`
    If no matching key is found and 'errorOnNotFound' is ``True``

Returns
-------
Optional[Tuple[Any, ...]]
    The found key, or ``None`` if none is found and 'errorOnNotFound' is ``False`` -- deliberately
    just the key, not the version it was resolved at (matching the exact contract real callers
    like GIMIParser rely on; see the C++ core's own note on this)
        )doc"))

        .def("replace", [](const PyModMappedAssets &self, const std::string &asset, const py::object &fromVersion, const py::object &fromNonVersionVals, const py::object &toVersion, const std::string &toAssetName, bool errorOnNotFound) -> py::object {
            std::optional<AGRC::Version> from = parseVersionArg(fromVersion);
            std::optional<AGRC::Version> to = parseVersionArg(toVersion);
            auto filter = resolveNonVersionValsFilter(self, fromNonVersionVals);
            // Passes 'errorOnNotFound' straight through to the core method, which is the one
            // place that actually knows which of its two distinct failure paths ("asset not
            // found at all" vs. "asset's name isn't in the map") occurred -- reimplementing
            // that decision here from the outside (e.g. via a separate hasFrom() check) would
            // only ever re-derive the first path, silently losing errorOnNotFound coverage for
            // the second (a real mistake caught during development, not a hypothetical one).
            try {
                std::optional<std::string> result = self.replace(asset, from, filter, to, toAssetName, errorOnNotFound);
                return result.has_value() ? py::cast(*result) : py::object(py::none());
            } catch (const std::out_of_range &e) {
                throw py::key_error(e.what());
            }
        }, py::arg("asset"), py::arg("fromVersion") = py::none(), py::arg("fromNonVersionVals") = py::none(), py::arg("toVersion") = py::none(), py::arg("toAssetName"), py::arg("errorOnNotFound") = true, py::doc(R"doc(
Retrieves the single corresponding asset to replace 'asset' with, for one specific target asset
name

Parameters
----------
asset: Any
    The asset to be replaced

fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The version to replace from -- see :meth:`getKey`

    **Default**: ``None``

fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[str, Any]]]
    The non-version value filter -- see :meth:`getKey`

    **Default**: ``None``

toVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The version to replace to -- the latest available version is used if this is ``None``

    **Default**: ``None``

toAssetName: Any
    The specific name of the asset to map to

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if 'asset' (or a mapping for it) isn't found at all --
    once past that point, "toAssetName isn't actually mapped from asset's name" or "no data
    exists for it at the queried version" always just returns ``None``, regardless of this flag

    **Default**: ``True``

Returns
-------
Any
    The replacement asset, or ``None`` if none is found
        )doc"))

        .def("replaceAll", [](const PyModMappedAssets &self, const std::string &asset, const py::object &fromVersion, const py::object &fromNonVersionVals, const py::object &toVersion, const py::object &toAssetNames, bool errorOnNotFound) -> py::dict {
            std::optional<AGRC::Version> from = parseVersionArg(fromVersion);
            std::optional<AGRC::Version> to = parseVersionArg(toVersion);
            auto filter = resolveNonVersionValsFilter(self, fromNonVersionVals);
            // See replace()'s comment just above on why 'errorOnNotFound' is passed straight
            // through rather than re-derived from a separate check.
            try {
                auto result = self.replaceAll(asset, from, filter, to, toVector(toAssetNames), errorOnNotFound);
                py::dict pyResult;
                for (const auto &entry : result) {
                    pyResult[py::cast(entry.first)] = py::cast(entry.second);
                }
                return pyResult;
            } catch (const std::out_of_range &e) {
                throw py::key_error(e.what());
            }
        }, py::arg("asset"), py::arg("fromVersion") = py::none(), py::arg("fromNonVersionVals") = py::none(), py::arg("toVersion") = py::none(), py::arg("toAssetNames") = py::none(), py::arg("errorOnNotFound") = true, py::doc(R"doc(
Retrieves every corresponding asset to replace 'asset' with

Parameters
----------
asset: Any
    The asset to be replaced

fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The version to replace from -- see :meth:`getKey`

    **Default**: ``None``

fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[str, Any]]]
    The non-version value filter -- see :meth:`getKey`

    **Default**: ``None``

toVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
    The version to replace to -- the latest available version is used if this is ``None``

    **Default**: ``None``

toAssetNames: Optional[List[Any]]
    The specific names of the assets to map to -- every asset name 'asset' maps to is used if
    this is ``None``

    **Default**: ``None``

errorOnNotFound: :class:`bool`
    Whether to raise :class:`KeyError` if 'asset' (or a mapping for it) isn't found at all --
    see :meth:`replace`'s note on this parameter

    **Default**: ``True``

Returns
-------
Dict[Any, Any]
    The corresponding assets for the fix to replace, keyed by asset name -- empty if nothing is
    found
        )doc"))

        .def_property_readonly("fromAssets", &PyModMappedAssets::getFromAssets, py::doc(R"doc(
List[Any]: Every asset value that has at least one known originating key -- a property (not a
method), matching the pure-Python original's contract exactly (real callers, e.g. IniFile.py's
``type.hashes.fromAssets``, access it as one)
        )doc"))

        .def("_convertNonVersionVals", [](const PyModMappedAssets &self, const py::object &indexVals) -> py::list {
            if (!self.nonVersionIndexNames.has_value()) {
                throw py::value_error("_convertNonVersionVals: this instance wasn't constructed with nonVersionIndexNames");
            }
            auto wildcards = toWildcardList(indexVals, *self.nonVersionIndexNames);
            py::list result(wildcards.size());
            for (std::size_t i = 0; i < wildcards.size(); ++i) {
                result[i] = wildcards[i].has_value() ? py::cast(*wildcards[i]) : py::object(py::none());
            }
            return result;
        }, py::arg("indexVals"), py::doc(R"doc(
Normalizes a flexible non-version-values filter into the plain positional
``List[Optional[Any]]`` shape :meth:`getKey`/:meth:`hasFrom`/:meth:`replace` accept for their own
'nonVersionVals'/'fromNonVersionVals' argument (``None`` = wildcard at that position) --
:attr:`nonVersionIndexNames` names each position :raw-html:`<br />` :raw-html:`<br />`

.. note::
    Calling this directly is rarely necessary any more -- :meth:`getKey`/:meth:`hasFrom`/
    :meth:`replace`/:meth:`replaceAll` all already accept the same flexible shape for their own
    non-version-values argument. Kept as public API for callers that want to convert once and
    reuse the result across several calls (e.g. ``GIMIParser.py``, filtering many hash/index
    values per parse against the same fixed non-version filter)

Parameters
----------
indexVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
    The raw, flexibly-shaped filter values to normalize -- ``None`` means "no values given at
    all" (every position wildcarded)

Raises
------
:class:`ValueError`
    If this instance wasn't constructed with 'nonVersionIndexNames'

Returns
-------
List[Optional[Any]]
    The normalized, positional filter values
        )doc"))

        .def_property_readonly("nonVersionIndexNames", [](const PyModMappedAssets &self) -> py::object {
            if (!self.nonVersionIndexNames.has_value()) {
                return py::none();
            }
            return py::cast(*self.nonVersionIndexNames);
        }, py::doc(R"doc(
Optional[List[:class:`str`]]: The names of the non-version index columns, in position order --
``None`` if this instance wasn't constructed with them (see the constructor's own note)
        )doc"))

        // fixFrom/fixTo: the pure-Python original declares these (Set[str], via self._fixFrom/
        // self._fixTo) but never actually populates either anywhere -- confirmed by reading every
        // method body, not assumed. They're still genuinely read though (ModType.py's own
        // fixTo-aggregating property does `self.hashes.fixTo`/`self.indices.fixTo`), so the
        // property has to exist for that real call site to keep working -- it just has nothing
        // real to report, matching current (if seemingly unfinished) behavior exactly. No core
        // involvement needed for either: there's no actual capability here to port, just dead-but-
        // accessed API surface to preserve.
        .def_property_readonly("fixFrom", [](const PyModMappedAssets &) { return py::set(); }, py::doc(R"doc(
Set[Any]: Always empty -- matches the pure-Python original, which declares this but never
populates it anywhere
        )doc"))

        .def_property_readonly("fixTo", [](const PyModMappedAssets &) { return py::set(); }, py::doc(R"doc(
Set[Any]: Always empty -- matches the pure-Python original, which declares this but never
populates it anywhere
        )doc"))

        .def_property_readonly("repo", [](const PyModMappedAssets &self) {
            return PyModDictAssets(self.getRepo());
        }, py::doc(R"doc(:class:`ModDictAssets`: The underlying asset data)doc"))

        .def_property_readonly("map", [](const PyModMappedAssets &self) {
            py::dict result;
            for (const auto &entry : self.getMap()) {
                result[py::cast(entry.first)] = py::cast(entry.second);
            }
            return result;
        }, py::doc(R"doc(Dict[Any, List[Any]]: The adjacency list mapping assets to fix from to assets to fix to)doc"));
}
