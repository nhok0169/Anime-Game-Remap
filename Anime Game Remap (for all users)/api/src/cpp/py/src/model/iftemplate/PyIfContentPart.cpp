#include "PyIfContentPart.h"

#include <string>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// items()'s Item isn't itself pybind-castable (a plain aggregate, not tuple-shaped) -- copied
// out into plain tuples, same treatment as PyIOrderedMultiMap.cpp's own items()/__iter__.
std::vector<std::tuple<std::string, std::string, size_t, size_t>> itemsAsTuples(const PyIfContentPart &self) {
    std::vector<std::tuple<std::string, std::string, size_t, size_t>> result;
    for (const auto &item : self.items()) {
        result.emplace_back(item.key, item.value, item.occurrenceIndex, item.orderIndex);
    }
    return result;
}

// 'src' constructor parameter: Dict[key, List[Tuple[index, value]]] -- one entry per occurrence
// of that key. Each value is cast wholesale via <pybind11/stl.h> rather than iterated by hand,
// matching how parseIniInsertAllAtItems()/parseOrderMap() (PyIOrderedMultiMap.cpp) cast dict values
// directly where the target shape is already something stl.h knows how to build.
std::vector<std::pair<std::string, std::vector<std::pair<long long, std::string>>>> parseSrc(const py::dict &src) {
    std::vector<std::pair<std::string, std::vector<std::pair<long long, std::string>>>> result;
    result.reserve(src.size());
    for (auto item : src) {
        std::string key = py::str(item.first).cast<std::string>();
        auto indexedVals = item.second.cast<std::vector<std::pair<long long, std::string>>>();
        result.emplace_back(std::move(key), std::move(indexedVals));
    }
    return result;
}

// Shared by IfTemplatePart's and IfContentPart's constructors -- both accept an optional
// 'id' the same way (None -> auto-generated, otherwise cast straight to size_t).
std::optional<size_t> parseId(const py::object &id) {
    if (id.is_none()) {
        return std::nullopt;
    }
    return id.cast<size_t>();
}

}


// removeKeys()'s dict values are each an optional check predicate -- unlike remapKeys()/
// replaceVals(), there's no marker-class ambiguity here (a bare None or a bare callable, no
// third alternative that could itself be a callable), so this is a direct conversion, no
// isinstance dispatch needed.
//
// Deliberately NOT in this file's anonymous namespace (unlike itemsAsTuples/parseSrc/parseId
// above): PyRegRemove.cpp reuses it verbatim, exactly the way PyIOrderedMultiMap.h already
// shares parseRanges/parseKeyRemap/parseReplaceVals with this file -- see that header's own
// note about keeping dict-parsing helpers in one place instead of duplicating them per binding.
std::vector<std::pair<std::string, std::optional<PyIfContentPart::RemoveKeyCheck>>> parseRemoveKeys(const py::dict &keys) {
    std::vector<std::pair<std::string, std::optional<PyIfContentPart::RemoveKeyCheck>>> result;
    result.reserve(keys.size());
    for (auto item : keys) {
        std::string key = py::str(item.first).cast<std::string>();
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        std::optional<PyIfContentPart::RemoveKeyCheck> check = std::nullopt;
        if (!value.is_none()) {
            check = value.cast<PyIfContentPart::RemoveKeyCheck>();
        }
        result.emplace_back(std::move(key), std::move(check));
    }
    return result;
}


void initCppIfContentPart(pybind11::module_ &m) {
    // Registered before IfContentPart below (and passed as IfContentPart's real Python base,
    // not just mentioned in a docstring) so 'isinstance(somePart, core.IfTemplatePart)' and
    // inherited attribute lookup (e.g. '.id') actually work, matching how CppTrie/CppDFA
    // inherit from a real registered BaseTrie/BaseDFA rather than just documenting it.
    //
    // Registered under the bare 'IfTemplatePart' name (no 'Cpp' prefix) rather than the
    // 'CppXxx' pattern used elsewhere in this file -- there's no deprecated bare-named Python
    // class shadowing it to disambiguate from (the deprecated pure-Python original has since
    // been removed entirely), so the 'Cpp' prefix isn't needed; see Documentation/CLAUDE.md's
    // naming-pitfall section for when the prefix is/isn't required.
    // py::smart_holder (not the default unique_ptr holder) -- required so a unique_ptr<IfTemplatePart>
    // (or a derived IfContentPart/IfPredPart) can be moved *from* Python *into* C++, not just
    // returned to Python -- see Architecture.md's own note on this. IfTemplate's own constructor/
    // 'parts' setter/'add'/'__setitem__' all need this (they adopt already-existing Python
    // IfContentPart/IfPredPart objects, taking ownership the same way this class's own 'content'
    // constructor parameter already does for IOrderedMultiMap). The base class needs the same
    // holder as any derived class that does -- pybind11 requires it consistently up an inheritance
    // chain for a unique_ptr-from-Python cast to work through a base-typed parameter.
    py::class_<AGRC::IfTemplatePart, py::smart_holder>(m, "IfTemplatePart", R"doc(
Base class for some part in an `IfTemplate`

Parameters
----------
id: Optional[:class:`int`]
    The id for the part. If this parameter is ``None``, will generate a new id for the part.

    **Default**: ``None``
        )doc")
        .def(py::init([](const py::object &id) {
            return std::make_unique<AGRC::IfTemplatePart>(parseId(id));
        }), py::arg("id") = py::none())

        .def_property_readonly("id", &AGRC::IfTemplatePart::id,
    py::doc(R"doc(:class:`int`: The id for the part)doc"))

        .def("refreshId", &AGRC::IfTemplatePart::refreshId,
    py::doc(R"doc(
Regenerates the id for the part

Returns
-------
:class:`int`
    The newly generated id
        )doc"));


    // py::smart_holder here too -- see the IfTemplatePart registration just above for why.
    py::class_<PyIfContentPart, AGRC::IfTemplatePart, py::smart_holder>(m, "IfContentPart", R"doc(
This class inherits from :class:`IfTemplatePart`

The content part of an `IfTemplate`, holding the key-value pairs (e.g. a `.ini` section's
registers) for one part of the template.

This class owns its data purely through a caller-supplied :class:`IOrderedMultiMap`
implementation -- pick which concrete ordered-multimap backs a given :class:`IfContentPart`
(:class:`CppOrderedMultiMap`/:class:`CppOrderedMultiMapSqrt` via their ``asInterface()`` method,
or any custom :class:`IOrderedMultiMap` implementation of your own, including one implemented
from Python), and every method on this class is a thin, renamed delegation straight to that
implementation -- the semantics for every operation are exactly :class:`CppOrderedMultiMap`'s
documented rules; only the *method names* below intentionally echo this project's deprecated,
pre-C++-port `IfContentPart` naming (e.g. ``insertAllAt`` -> ``addKVPsByInds``).

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: key in x

        Determines if 'key' exists in the content part

    .. describe:: len(x)

        Retrieves the number of KVPs in the content part

    .. describe:: x[index]

        Retrieves the ``(key, value)`` pair at the given true positional index, if ``index`` is
        an :class:`int`

    .. describe:: x[key]

        Retrieves every value currently stored under ``key``, in true positional order
        (equivalent to :meth:`getVals` with ``ordered=True``), if ``key`` is a :class:`str` --
        raises :class:`KeyError` if ``key`` doesn't exist

    .. describe:: iter(x)

        Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex,
        orderIndex)`` tuples

.. note::
    Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
    deep copy (equivalent to ``x.clone()``)

Parameters
----------
src: Optional[Dict[Any, List[Tuple[:class:`int`, Any]]]]
    Initial data to populate ``content`` with, as key -> list of ``(index, value)`` pairs, one
    entry per occurrence of that key :raw-html:`<br />` :raw-html:`<br />`

    ``index`` orders every occurrence relative to every other occurrence *across all keys*
    (gathered, stable-sorted by ``index`` ascending, then appended in that order); it is **not**
    a strict absolute position, so gaps and duplicate/out-of-order values are fine. If
    ``content`` already holds data (a pre-populated instance was passed in rather than left to
    default), ``src``'s entries are appended after it, not merged/interleaved with it.
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning no initial data is inserted

depth: :class:`int`
    The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

content: Optional[:class:`IOrderedMultiMap`]
    The backing ordered-multimap implementation to use, taken by ownership -- see this class's
    top-level warning about what that means for 'content' afterward :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used

id: Optional[:class:`int`]
    The id for the part. If this parameter is ``None``, will generate a new id for the part.

    **Default**: ``None``
        )doc")

        .def(py::init([](const py::object &src, int depth, const py::object &content, const py::object &id) {
            // Always this class's own now. An IfContentPart holds std::string KVPs (a .ini file
            // has no others), and the IOrderedMultiMap of *that* instantiation is deliberately not
            // bound to Python -- only the generic CppOrderedMultiMap is. A caller can therefore no
            // longer supply one: 'content' stays in the signature and is rejected rather than
            // silently ignored, which would be the worse failure.
            if (!content.is_none()) {
                throw py::value_error("'content' is no longer supported: an IfContentPart always uses its own backing store");
            }

            // Always a fresh CppOrderedMultiMap (the list-backed OrderedMultiMap), matching how
            // CppOrderedMultiMap() itself constructs an empty map with no arguments.
            auto actualContent = std::make_unique<AGRC::OrderedMultiMapListAdapter<std::string, std::string>>();

            auto actualId = parseId(id);

            if (src.is_none()) {
                return std::make_unique<PyIfContentPart>(depth, std::move(actualContent), actualId);
            }

            // A Python dict already preserves insertion order (guaranteed since 3.7), so this
            // routes through the core's tsl::ordered_map overload rather than its
            // std::unordered_map one -- the latter would needlessly throw away an ordering
            // Python already guarantees. Built via the range constructor (matching how
            // OrderedMultiMapAdapter::insertAllAt() itself builds a tsl::ordered_map from a flat
            // vector) since <pybind11/stl.h> has no caster for tsl::ordered_map directly.
            auto parsed = parseSrc(src.cast<py::dict>());
            tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>
                actualSrc(std::make_move_iterator(parsed.begin()), std::make_move_iterator(parsed.end()));

            return std::make_unique<PyIfContentPart>(actualSrc, depth, std::move(actualContent), actualId);
        }), py::arg("src") = py::none(), py::arg("depth") = 0, py::arg("content") = py::none(), py::arg("id") = py::none())

        .def_static("buildFromOrder", [](const py::object &src, int depth, const py::object &content, const py::object &id) {
            // Always this class's own now. An IfContentPart holds std::string KVPs (a .ini file
            // has no others), and the IOrderedMultiMap of *that* instantiation is deliberately not
            // bound to Python -- only the generic CppOrderedMultiMap is. A caller can therefore no
            // longer supply one: 'content' stays in the signature and is rejected rather than
            // silently ignored, which would be the worse failure.
            if (!content.is_none()) {
                throw py::value_error("'content' is no longer supported: an IfContentPart always uses its own backing store");
            }

            auto actualContent = std::make_unique<AGRC::OrderedMultiMapListAdapter<std::string, std::string>>();

            auto actualSrc = src.cast<std::vector<std::pair<std::string, std::string>>>();
            return std::make_unique<PyIfContentPart>(actualSrc, depth, std::move(actualContent), parseId(id));
        }, py::arg("src"), py::arg("depth") = 0, py::arg("content") = py::none(), py::arg("id") = py::none(),
    py::doc(R"doc(
Creates a new part, populated from a flat, already-ordered list of key-value pairs -- a thin
convenience over default-constructing then calling :meth:`addKVPs`

Parameters
----------
src: List[Tuple[Any, Any]]
    The key-value pairs to populate ``content`` with, appended in the order given (``src[0]``
    ends up first, right after whatever ``content`` already held, and so on) -- a key may repeat
    here directly, e.g. ``[("a", "1"), ("b", "2"), ("a", "3")]``

depth: :class:`int`
    The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

content: Optional[:class:`IOrderedMultiMap`]
    The backing ordered-multimap implementation to use, taken by ownership -- see
    :class:`IfContentPart`'s top-level warning about what that means for 'content' afterward
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used

id: Optional[:class:`int`]
    The id for the part. If this parameter is ``None``, will generate a new id for the part.

    **Default**: ``None``

Returns
-------
:class:`IfContentPart`
    The newly-created part
        )doc"))

        .def_property("depth", &PyIfContentPart::depth, &PyIfContentPart::setDepth,
    py::doc(R"doc(:class:`int`: The depth this part is within the owning `IfTemplate`)doc"))

        .def("clone", &PyIfContentPart::clone, py::arg("newId") = false,
    py::doc(R"doc(
Creates a deep copy of this part, at the same depth

Parameters
----------
newId: :class:`bool`
    Whether to generate a new id for the cloned part :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning the clone keeps this part's own :attr:`id`

Returns
-------
:class:`IfContentPart`
    The cloned part
        )doc"))

        .def("__copy__", [](PyIfContentPart &self) { return self.clone(); },
    py::doc(R"doc(Creates a copy of this part (equivalent to :meth:`clone`); supports ``copy.copy()``)doc"))
        .def("__deepcopy__", [](PyIfContentPart &self, py::dict) { return self.clone(); }, py::arg("memo"),
    py::doc(R"doc(Creates a deep copy of this part (equivalent to :meth:`clone`); supports ``copy.deepcopy()``)doc"))

        // ---- insert family ----

        .def("addKVP", &PyIfContentPart::addKVP, py::arg("key"), py::arg("value"),
    py::doc(R"doc(Appends a KVP to the end)doc"))

        .def("addKVPToFront", &PyIfContentPart::addKVPToFront, py::arg("key"), py::arg("value"),
    py::doc(R"doc(Inserts a KVP at the beginning)doc"))

        .def("addKVPAt", &PyIfContentPart::addKVPAt, py::arg("index"), py::arg("key"), py::arg("value"),
    py::doc(R"doc(Inserts a KVP so it ends up at position 'index' (0-based); see :meth:`CppOrderedMultiMap.insertAt` for the full index semantics)doc"))

        .def("addKVPs", &PyIfContentPart::addKVPs, py::arg("kvps"),
    py::doc(R"doc(Appends a batch of KVPs to the end, in the order given)doc"))

        .def("addKVPsToFront", &PyIfContentPart::addKVPsToFront, py::arg("kvps"),
    py::doc(R"doc(Inserts a batch of KVPs at the beginning, in the order given)doc"))

        .def("addKVPsByInds", [](PyIfContentPart &self, const py::dict &kvps, bool sortIndices, const py::object &ranges) {
            return self.addKVPsByInds(parseIniInsertAllAtItems(kvps), sortIndices, parseRanges(ranges));
        }, py::arg("kvps"), py::arg("sortIndices") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(Bulk indexed insert of KVPs; see :meth:`CppOrderedMultiMap.insertAllAt` for the full semantics)doc"))

        // ---- removal ----

        .def("removeKVPAt", [](PyIfContentPart &self, size_t pos, const py::object &ranges) {
            return self.removeKVPAt(pos, parseRanges(ranges));
        }, py::arg("pos"), py::arg("ranges") = py::none(),
    py::doc(R"doc(Removes the KVP currently at position 'pos')doc"))

        .def("removeKey", [](PyIfContentPart &self, const std::string &key, const py::object &ranges,
                              const std::optional<PyIfContentPart::RemoveKeyCheck> &check) {
            return self.removeKey(key, parseRanges(ranges), check);
        }, py::arg("key"), py::arg("ranges") = py::none(), py::arg("check") = py::none(),
    py::doc(R"doc(
Removes every KVP with this key, subject to two independent, optional filters -- both must hold
(where provided) for a given occurrence to actually be removed. With neither filter provided,
this is unconditional removal of every KVP with this key.

Parameters
----------
key: Any
    The key whose KVPs to remove

ranges: Optional[:class:`Ranges`]
    If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
    must fall within 'ranges'

check: Optional[Callable[[int, Any], bool]]
    If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
    positional index and value

Returns
-------
:class:`int`
    How many KVPs were actually removed
        )doc"))

        .def("removeKeys", [](PyIfContentPart &self, const py::dict &keys, const py::object &ranges) {
            return self.removeKeys(parseRemoveKeys(keys), parseRanges(ranges));
        }, py::arg("keys"), py::arg("ranges") = py::none(),
    py::doc(R"doc(
Removes multiple, independently-specified keys -- a thin loop over :meth:`removeKey`, sharing
one 'ranges' filter across all of them.

Parameters
----------
keys: Dict[Any, Optional[Callable[[int, Any], bool]]]
    Each key to remove, mapped to its own optional check predicate -- ``check(index, value)``
    must return ``True`` for a given occurrence of that key to actually be removed, if provided;
    if omitted, every occurrence of that key is removed unconditionally (subject to 'ranges'
    below)

ranges: Optional[:class:`Ranges`]
    If provided, an occurrence's true positional index (same convention as :meth:`getByInd`)
    must fall within 'ranges' for it to be eligible for removal, for every key in 'keys'

Returns
-------
:class:`int`
    How many KVPs were actually removed, summed across every key in 'keys'
        )doc"))

        // ---- bulk edits ----

        .def("reorder", [](PyIfContentPart &self, const py::dict &orderMap, const py::object &ranges) {
            self.reorder(parseOrderMap(orderMap), parseRanges(ranges));
        }, py::arg("orderMap"), py::arg("ranges") = py::none(),
    py::doc(R"doc(Reorders existing KVPs in place; see :meth:`CppOrderedMultiMap.reorder` for the full semantics)doc"))

        .def("remapKeys", [](PyIfContentPart &self, const py::dict &keyRemap, const py::object &ranges) {
            self.remapKeys(parseIniKeyRemap(keyRemap), parseRanges(ranges));
        }, py::arg("keyRemap"), py::arg("ranges") = py::none(),
    py::doc(R"doc(Bulk-renames keys; see :meth:`CppOrderedMultiMap.remapKeys` for the full semantics)doc"))

        .def("replaceVals", [](PyIfContentPart &self, const py::dict &newVals, bool addNew, const py::object &ranges) {
            self.replaceVals(parseIniReplaceVals(newVals), addNew, parseRanges(ranges));
        }, py::arg("newVals"), py::arg("addNew") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(Bulk-updates values by key; see :meth:`CppOrderedMultiMap.replaceVals` for the full semantics)doc"))

        // ---- lookups ----

        .def("contains", &PyIfContentPart::contains, py::arg("key"), py::doc(R"doc(Checks whether a key exists)doc"))
        .def("containsKey", &PyIfContentPart::containsKey, py::arg("key"), py::doc(R"doc(Checks whether a key exists)doc"))
        .def("__contains__", &PyIfContentPart::contains, py::arg("key"), py::doc(R"doc(Determines whether 'key' exists)doc"))

        .def("count", &PyIfContentPart::count, py::arg("key"), py::doc(R"doc(Retrieves how many KVPs share a given key)doc"))

        .def("size", &PyIfContentPart::size, py::doc(R"doc(Retrieves the number of KVPs)doc"))
        .def("length", &PyIfContentPart::length, py::doc(R"doc(Retrieves the number of KVPs)doc"))
        .def("__len__", &PyIfContentPart::size, py::doc(R"doc(Retrieves the number of KVPs)doc"))

        .def("keySize", &PyIfContentPart::keySize, py::doc(R"doc(Retrieves the number of distinct keys)doc"))

        .def("empty", &PyIfContentPart::empty, py::doc(R"doc(Checks whether the part has no KVPs)doc"))

        .def("getVals", [](const PyIfContentPart &self, const std::string &key, bool ordered, const py::object &ranges) {
            return self.getVals(key, ordered, parseRanges(ranges));
        }, py::arg("key"), py::arg("ordered") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Retrieves all values currently stored under a key

Parameters
----------
key: Any
    The key to look up

ordered: :class:`bool`
    If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
    returned in whatever order they were added to this key

ranges: Optional[:class:`Ranges`]
    If provided, only occurrences whose true positional index (same convention as
    :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning every occurrence is included

Returns
-------
List[Any]
    The values for this key, in the requested order
        )doc"))

        .def("getValsWithInds", [](const PyIfContentPart &self, const std::string &key, bool ordered, const py::object &ranges) {
            return self.getValsWithInds(key, ordered, parseRanges(ranges));
        }, py::arg("key"), py::arg("ordered") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Retrieves all values currently stored under a key, each paired with its true positional index
(equivalent to :meth:`getVals`, except each value is paired with its true positional index)

Parameters
----------
key: Any
    The key to look up

ordered: :class:`bool`
    If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
    returned in whatever order they were added to this key

ranges: Optional[:class:`Ranges`]
    If provided, only occurrences whose true positional index (same convention as
    :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning every occurrence is included

Returns
-------
List[Tuple[:class:`int`, Any]]
    The ``(index, value)`` pairs for this key, in the requested order
        )doc"))

        .def("getKeys", &PyIfContentPart::getKeys,
    py::doc(R"doc(
Retrieves every distinct key currently in this part

Returns
-------
Set[Any]
    Every distinct key, as a set (unordered)
        )doc"))

        .def("getByInd", &PyIfContentPart::getByInd, py::arg("index"),
    py::doc(R"doc(Retrieves the ``(key, value)`` pair at a true positional index)doc"))

        .def("getByIndWithOccurrence", &PyIfContentPart::getByIndWithOccurrence, py::arg("index"),
    py::doc(R"doc(Retrieves the KVP at a true positional index, paired with its occurrence index)doc"))

        .def("setValByInd", &PyIfContentPart::setValByInd, py::arg("index"), py::arg("value"),
    py::doc(R"doc(Sets the value of the KVP at a true positional index, leaving its key untouched)doc"))

        .def("__getitem__", &PyIfContentPart::getByInd, py::arg("index"),
    py::doc(R"doc(Retrieves the ``(key, value)`` pair at a true positional index)doc"))

        // A str doesn't cast to 'long long' (pybind11's integer caster requires __index__,
        // which str doesn't implement) and 'long long' doesn't cast to std::string, so pybind11
        // dispatches correctly between this overload and the one just above based on the
        // argument's actual type -- no manual isinstance check needed.
        .def("__getitem__", [](PyIfContentPart &self, const std::string &key) {
            if (!self.contains(key)) {
                throw py::key_error(key);
            }
            return self.getVals(key, true);
        }, py::arg("key"),
    py::doc(R"doc(Retrieves all values currently stored under a key, in true positional order (equivalent to :meth:`getVals` with ``ordered=True``); raises :class:`KeyError` if the key doesn't exist)doc"))

        // Same int/str dispatch trick as __getitem__ above -- see that overload pair's comment.
        .def("get", [](PyIfContentPart &self, long long key, bool errorOnNotFound, const py::object &defaultVal,
                        bool ordered, bool withInds, const py::object &ranges) -> py::object {
            (void)ordered; (void)withInds; (void)ranges;  // only meaningful for the str overload below
            try {
                return py::cast(self.getByInd(key));
            } catch (const std::out_of_range&) {
                if (errorOnNotFound) throw;
                return defaultVal;
            }
        }, py::arg("key"), py::arg("errorOnNotFound") = false, py::arg("default") = py::none(),
           py::arg("ordered") = true, py::arg("withInds") = false, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Retrieves the ``(key, value)`` pair at a true positional index (if ``key`` is an :class:`int`) or
every value currently stored under a key (if ``key`` is a :class:`str`) -- like :meth:`__getitem__`,
except not finding anything is configurable instead of always raising.

Parameters
----------
key: Union[:class:`int`, :class:`str`]
    The true positional index or key to look up

errorOnNotFound: :class:`bool`
    If ``True`` and nothing is found, raises :class:`KeyError` (``key`` was a :class:`str`) or
    :class:`IndexError` (``key`` was an :class:`int`, out of range). If ``False`` (the default),
    returns ``default`` instead of raising.

default: Any
    The value returned when nothing is found and ``errorOnNotFound`` is ``False`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

ordered: :class:`bool`
    Only takes effect when ``key`` is a :class:`str` -- same purpose as ``ordered`` from
    :meth:`CppOrderedMultiMap.getAll` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

withInds: :class:`bool`
    Only takes effect when ``key`` is a :class:`str` -- if ``True``, each returned value is
    paired with its true positional index (equivalent to :meth:`getValsWithInds`); if ``False``
    (the default), values are returned bare (equivalent to :meth:`getVals`)

ranges: Optional[:class:`Ranges`]
    Only takes effect when ``key`` is a :class:`str` -- if provided, only occurrences whose true
    positional index (same convention as :meth:`getByInd`) falls within ``ranges`` are
    considered :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning every occurrence is considered

Returns
-------
Any
    The result described above, or ``default`` if nothing was found and ``errorOnNotFound`` is
    ``False``
        )doc"))

        .def("get", [](PyIfContentPart &self, const std::string &key, bool errorOnNotFound, const py::object &defaultVal,
                        bool ordered, bool withInds, const py::object &ranges) -> py::object {
            auto actualRanges = parseRanges(ranges);
            if (!self.contains(key)) {
                if (errorOnNotFound) throw py::key_error(key);
                return defaultVal;
            }
            if (withInds) {
                return py::cast(self.getValsWithInds(key, ordered, actualRanges));
            }
            return py::cast(self.getVals(key, ordered, actualRanges));
        }, py::arg("key"), py::arg("errorOnNotFound") = false, py::arg("default") = py::none(),
           py::arg("ordered") = true, py::arg("withInds") = false, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Same as the ``int``-keyed overload above, for a :class:`str` ``key`` -- see its docstring for
the full parameter descriptions
        )doc"))

        .def("entries", &PyIfContentPart::entries,
    py::doc(R"doc(Retrieves a copy of the full ordered sequence, as ``(key, value)`` pairs)doc"))

        .def("items", &itemsAsTuples,
    py::doc(R"doc(Retrieves a copy of the full ordered sequence, as ``(key, value, occurrenceIndex, orderIndex)`` tuples)doc"))

        .def("__iter__", [](const PyIfContentPart &self) {
            // py::iter(), not a bare list -- __iter__ must return an actual iterator.
            return py::iter(py::cast(itemsAsTuples(self)));
        }, py::doc(R"doc(Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples)doc"))

        .def("splitByInds", &PyIfContentPart::splitByInds,
             py::arg("inds"), py::arg("includeSplitKVP") = true, py::arg("includeEmptyParts") = false, py::arg("sortIndices") = true,
    py::doc(R"doc(
Splits this part into several smaller parts at the given indices, each at the same depth as
this part; see :meth:`CppOrderedMultiMap.splitByInds` for the full semantics

Returns
-------
List[:class:`IfContentPart`]
    The resulting parts, left to right
        )doc"))

        .def("toStr", [](const PyIfContentPart &self, const std::string &linePrefix) {
            std::string result;
            const auto entries = self.entries();
            const size_t n = entries.size();
            for (size_t i = 0; i < n; ++i) {
                result += linePrefix;
                result += py::str(entries[i].first).cast<std::string>();
                result += " = ";
                result += py::str(entries[i].second).cast<std::string>();
                if (i < n - 1) result += "\n";
            }
            return result;
        }, py::arg("linePrefix") = "",
    py::doc(R"doc(
Retrieves the part as a string, one ``key = value`` line per KVP in true positional order

Parameters
----------
linePrefix: :class:`str`
    The string that will prefix every line :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`str`
    The string representation of the part
        )doc"));
}
