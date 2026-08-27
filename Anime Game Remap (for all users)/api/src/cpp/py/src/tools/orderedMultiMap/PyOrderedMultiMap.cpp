#include "PyOrderedMultiMap.h"

#include <unordered_map>

#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMapAdapter.h"


// Why K and V are both 'py::object' here (rather than e.g. binding a handful of concrete
// K/V type combinations the way other generic containers in this project might): there were
// no existing AGRC::OrderedMultiMap<...>/AGRC::OrderedMultiMapSqrt<...> instantiations
// anywhere else in the codebase to infer real usage from, and this was the combination
// explicitly requested -- a fully generic "ordered multimap with any hashable key, holding
// anything" surface, matching what a Python 'dict' user would expect. Two consequences that
// shape most of the adapter code below:
//
//   1. BaseOrderedMultiMap's internal key -> bucket index needs a real Hash/KeyEqual for K.
//      py::object has neither a std::hash nor a natural operator== a plain
//      std::unordered_map<K, ...> could pick up on its own, so BaseOrderedMultiMap /
//      OrderedMultiMap / OrderedMultiMapSqrt were given KeyHash/KeyEqual template
//      parameters (defaulting to std::hash<K>/std::equal_to<K>, so every other caller of
//      those class templates is unaffected), and PyObjectHash/PyObjectEqual (this project's
//      existing py::object hash/equality functors -- see ../PyTools.h, already used the same
//      way by PyBiMap/PyDFA) are plugged in here.
//
//   2. Several C++-side std::variant parameters (ReplaceSpec, RemapListItem) have 'V' or 'K'
//      (i.e. py::object) as one of their alternatives. pybind11's automatic std::variant
//      support tries each alternative's type_caster in order and uses the first one that
//      succeeds -- but py::object's caster accepts literally any Python object, so it would
//      always match first and starve out every other alternative. Every method that would
//      otherwise take one of these variants is therefore a manual adapter here (see
//      pyRemapKeys/pyReplaceVals below) rather than relying on <pybind11/stl.h>'s std::variant
//      caster.
namespace py = pybind11;
namespace AGRC = AGRemapCore;


template class AGRC::OrderedMultiMap<py::object, py::object, PyObjectHash, PyObjectEqual>;


// ------------------------------------------------------------------
// PyOrderedMultiMap
// ------------------------------------------------------------------

PyOrderedMultiMap PyOrderedMultiMap::fromBase(const Base &base) {
    PyOrderedMultiMap result;
    for (const auto &entry : base.entries()) {
        result.insert(entry.key, entry.value);
    }
    return result;
}

void PyOrderedMultiMap::buildFromIndexedDict(
    const tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> &indexed) {
    this->buildFromIndexed(indexed);
}

PyOrderedMultiMap PyOrderedMultiMap::fromIndexed(
    const tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> &indexed) {
    PyOrderedMultiMap result;
    result.buildFromIndexedDict(indexed);
    return result;
}

size_t PyOrderedMultiMap::pyInsertAllAt(const py::dict &items, bool sortIndices, const std::optional<PyRanges<long long>> &ranges) {
    tsl::ordered_map<long long, std::pair<py::object, py::object>> converted;
    for (auto item : items) {
        converted[item.first.cast<long long>()] = item.second.cast<std::pair<py::object, py::object>>();
    }
    return this->insertAllAt(converted, sortIndices, toCoreRanges(ranges));
}

void PyOrderedMultiMap::pyReorder(const py::dict &orderMap, const std::optional<PyRanges<long long>> &ranges) {
    tsl::ordered_map<long long, long long> converted;
    for (auto item : orderMap) {
        converted[item.first.cast<long long>()] = item.second.cast<long long>();
    }
    this->reorder(converted, toCoreRanges(ranges));
}

void PyOrderedMultiMap::pyRemapKeys(const py::dict &keyRemap, const std::optional<PyRanges<long long>> &ranges) {
    using KeyRemapList = Base::KeyRemapList;
    using KeyRemapValue = Base::KeyRemapValue;

    std::unordered_map<py::object, KeyRemapValue, PyObjectHash, PyObjectEqual> converted;
    for (auto item : keyRemap) {
        py::object key = py::reinterpret_borrow<py::object>(item.first);
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        if (py::isinstance<PyKeyRemapData>(value)) {
            converted.emplace(std::move(key), value.cast<PyKeyRemapData>());
            continue;
        }

        if (!py::isinstance<py::list>(value) && !py::isinstance<py::tuple>(value)) {
            throw py::type_error(
                "remapKeys(): each keyRemap value must be a KeyRemapData instance, or a "
                "list/tuple mixing bare keys and RemappedKeyData instances");
        }

        // Manual dispatch, not pybind11's automatic std::variant support -- see this file's
        // top-of-file comment for why a bare key (py::object) always wins that race.
        KeyRemapList list;
        for (py::handle elemH : value) {
            py::object elem = py::reinterpret_borrow<py::object>(elemH);
            if (py::isinstance<PyRemappedKeyData>(elem)) {
                list.emplace_back(elem.cast<PyRemappedKeyData>());
            } else {
                list.emplace_back(elem); // RemappedKeyData's ctor is 'explicit', so this
                                          // unambiguously selects the bare-key alternative
            }
        }
        converted.emplace(std::move(key), std::move(list));
    }

    this->remapKeys(converted, toCoreRanges(ranges));
}

void PyOrderedMultiMap::pyReplaceVals(const py::dict &newVals, bool addNew, const std::optional<PyRanges<long long>> &ranges) {
    using ReplaceSpec = Base::ReplaceSpec;

    tsl::ordered_map<py::object, ReplaceSpec, PyObjectHash, PyObjectEqual> converted;
    for (auto item : newVals) {
        py::object key = py::reinterpret_borrow<py::object>(item.first);
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        if (py::isinstance<PyReplaceList>(value)) {
            converted[key] = value.cast<PyReplaceList>().values();
        } else if (py::isinstance<PyReplaceIf>(value)) {
            PyReplaceIf spec = value.cast<PyReplaceIf>();
            converted[key] = std::make_pair(spec.value(), spec.predicate());
        } else {
            converted[key] = value;
        }
    }

    this->replaceVals(converted, addNew, toCoreRanges(ranges));
}

std::vector<PyOrderedMultiMap> PyOrderedMultiMap::pySplitByInds(
    const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices) {
    tsl::ordered_set<long long> converted(inds.begin(), inds.end());
    std::vector<Base> parts = this->splitByInds(converted, includeSplitKVP, includeEmptyParts, sortIndices);

    std::vector<PyOrderedMultiMap> result;
    result.reserve(parts.size());
    for (const Base &part : parts) {
        result.push_back(fromBase(part));
    }
    return result;
}


// ------------------------------------------------------------------
// PyBindOrderedMultiMap
// ------------------------------------------------------------------

size_t PyBindOrderedMultiMap::pyInsertAllAt(const py::dict &items, bool sortIndices, const std::optional<PyRanges<long long>> &ranges) {
    PYBIND11_OVERRIDE_NAME(
        size_t,
        PyOrderedMultiMap,
        "insertAllAt",
        pyInsertAllAt,
        items, sortIndices, ranges
    );
}

void PyBindOrderedMultiMap::pyReorder(const py::dict &orderMap, const std::optional<PyRanges<long long>> &ranges) {
    PYBIND11_OVERRIDE_NAME(
        void,
        PyOrderedMultiMap,
        "reorder",
        pyReorder,
        orderMap, ranges
    );
}

void PyBindOrderedMultiMap::pyRemapKeys(const py::dict &keyRemap, const std::optional<PyRanges<long long>> &ranges) {
    PYBIND11_OVERRIDE_NAME(
        void,
        PyOrderedMultiMap,
        "remapKeys",
        pyRemapKeys,
        keyRemap, ranges
    );
}

void PyBindOrderedMultiMap::pyReplaceVals(const py::dict &newVals, bool addNew, const std::optional<PyRanges<long long>> &ranges) {
    PYBIND11_OVERRIDE_NAME(
        void,
        PyOrderedMultiMap,
        "replaceVals",
        pyReplaceVals,
        newVals, addNew, ranges
    );
}

std::vector<PyOrderedMultiMap> PyBindOrderedMultiMap::pySplitByInds(
    const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices) {
    PYBIND11_OVERRIDE_NAME(
        std::vector<PyOrderedMultiMap>,
        PyOrderedMultiMap,
        "splitByInds",
        pySplitByInds,
        inds, includeSplitKVP, includeEmptyParts, sortIndices
    );
}


// ------------------------------------------------------------------
// PyOrderedMultiMapIterator
// ------------------------------------------------------------------

PyOrderedMultiMapIterator& PyOrderedMultiMapIterator::iter() { return *this; }

std::tuple<py::object, py::object, size_t, size_t> PyOrderedMultiMapIterator::next() {
    if (current_ == end_) {
        throw py::stop_iteration();
    }
    auto item = *current_; // BaseIterator::Item -- key/value are references, copied out below
    std::tuple<py::object, py::object, size_t, size_t> result(
        item.key, item.value, item.occurrenceIndex, item.orderIndex);
    ++current_;
    return result;
}


// ------------------------------------------------------------------
// module registration
// ------------------------------------------------------------------

void initCppOrderedMultiMap(pybind11::module_ &m) {
    py::class_<PyRemappedKeyData>(m, "RemappedKeyData", R"doc(
A single rule inside a :meth:`OrderedMultiMap.remapKeys` remap list, expressing a
*conditional* and/or *repositioned* rename (as opposed to a plain key, which always fires and
stays in place).

Parameters
----------
key: Any
    The new key to remap matching occurrences to

check: Optional[Callable[[Any, Any], :class:`bool`]]
    An optional predicate over ``(oldKey, oldValue)``. If omitted, this rule always fires. If
    present, it fires only when the predicate returns ``True`` for a given occurrence :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

toInd: Optional[:class:`int`]
    An optional target index. If present, every entry this rule produces (across all
    occurrences it fires for) is moved, as a group, to this index once the remap is complete --
    using the exact same index semantics, conflict resolution, and front/back
    overflow-clustering rules as :meth:`OrderedMultiMap.reorder`'s target index. If omitted,
    produced entries stay wherever the remap naturally places them (in place of the occurrence
    that produced them) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc")

        .def(py::init<py::object, std::optional<PyRemappedKeyData::CheckPredicate>, std::optional<long long>>(),
             py::arg("key"), py::arg("check") = py::none(), py::arg("toInd") = py::none())

        .def_property_readonly("key", &PyRemappedKeyData::key, py::doc(R"doc(Any: The new key to remap matching occurrences to)doc"))
        .def_property_readonly("check", &PyRemappedKeyData::check, py::doc(R"doc(Optional[Callable[[Any, Any], :class:`bool`]]: The optional firing predicate)doc"))
        .def_property_readonly("toInd", &PyRemappedKeyData::toInd, py::doc(R"doc(Optional[:class:`int`]: The optional repositioning target index)doc"));


    py::class_<PyKeyRemapData>(m, "KeyRemapData", R"doc(
A :meth:`OrderedMultiMap.remapKeys` ``keyRemap`` value (alongside a bare list of
keys/:class:`RemappedKeyData`) that additionally controls what happens to an occurrence when
none of its rules fire.

Parameters
----------
remappedKeys: List[Union[Any, :class:`RemappedKeyData`]]
    The remap rules for this key -- identical in meaning to passing a bare list directly

keepKeyWithoutRemap: :class:`bool`
    If ``True``, an occurrence for which zero rules fired (an empty list, or every rule's
    ``check`` ``False``) retains its original ``(key, value)`` pair instead of being removed.
    Evaluated per occurrence :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, matching a bare list's behavior: zero firings means removal
        )doc")

        .def(py::init([](const py::sequence &remappedKeys, bool keepKeyWithoutRemap) {
            PyOrderedMultiMap::Base::KeyRemapList list;
            for (py::handle elemH : remappedKeys) {
                py::object elem = py::reinterpret_borrow<py::object>(elemH);
                if (py::isinstance<PyRemappedKeyData>(elem)) {
                    list.emplace_back(elem.cast<PyRemappedKeyData>());
                } else {
                    list.emplace_back(elem);
                }
            }
            return PyKeyRemapData(std::move(list), keepKeyWithoutRemap);
        }), py::arg("remappedKeys"), py::arg("keepKeyWithoutRemap") = false)

        .def_property_readonly("keepKeyWithoutRemap", &PyKeyRemapData::keepKeyWithoutRemap,
    py::doc(R"doc(:class:`bool`: Whether a non-firing occurrence retains its original pair)doc"));


    py::class_<PyReplaceList>(m, "ReplaceList", R"doc(
A :meth:`OrderedMultiMap.replaceVals` spec: update this key's entries positionally from a
list of values -- the i-th existing entry (true left-to-right order) gets ``values[i]``. A list
shorter than the key's entry count leaves the remaining entries untouched; a longer list just
has its extra values unused.

Parameters
----------
values: List[Any]
    The values to assign positionally
        )doc")
        .def(py::init<std::vector<py::object>>(), py::arg("values"))
        .def_property_readonly("values", &PyReplaceList::values, py::doc(R"doc(List[Any]: The values to assign positionally)doc"));


    py::class_<PyReplaceIf>(m, "ReplaceIf", R"doc(
A :meth:`OrderedMultiMap.replaceVals` spec: replace this key's value with ``value``,
wherever ``predicate(oldValue)`` is ``True``.

.. note::
    ``predicate`` is stored as-is and each consumer decides what it passes to it. Every
    ``replaceVals`` (:meth:`OrderedMultiMap.replaceVals`, :meth:`OrderedMultiMapSqrt.replaceVals`,
    :meth:`IOrderedMultiMap.replaceVals`, :meth:`IfContentPart.replaceVals`) calls it with just the
    old value. The one deliberate exception is :class:`RegNewVals`, which calls it as
    ``predicate(oldValue, modType)`` -- see that class for why.

Parameters
----------
value: Any
    The replacement value

predicate: Callable[..., :class:`bool`]
    The predicate deciding whether a given old value should be replaced. Takes ``(oldValue)``
    for every ``replaceVals``, or ``(oldValue, modType)`` when handed to :class:`RegNewVals`
        )doc")
        .def(py::init<py::object, py::object>(), py::arg("value"), py::arg("predicate"))
        .def_property_readonly("value", &PyReplaceIf::value, py::doc(R"doc(Any: The replacement value)doc"))
        // Bound to predicateObj (the raw Python callable), not to the single-argument 'predicate'
        // adapter -- so 'someSpec.predicate is theCallableYouPassed' holds, and so a caller
        // reading it back gets something it can invoke with whatever arity that consumer defines.
        .def_property_readonly("predicate", &PyReplaceIf::predicateObj, py::doc(R"doc(Callable[..., :class:`bool`]: The replacement predicate)doc"));


    py::class_<PyOrderedMultiMapIterator>(m, "OrderedMultiMapIterator", R"doc(
A forward iterator over a :class:`OrderedMultiMap`, yielding ``(key, value,
occurrenceIndex, orderIndex)`` tuples in true positional order.
        )doc")
        .def("__iter__", &PyOrderedMultiMapIterator::iter, py::return_value_policy::reference_internal)
        .def("__next__", &PyOrderedMultiMapIterator::next);


    py::class_<PyOrderedMultiMap, PyBindOrderedMultiMap>(m, "OrderedMultiMap", R"doc(
An ordered multimap implemented in C++: preserves insertion/positional order, allows duplicate
keys, and gives both fast key-based access and fast positional access. Backed by a plain linked
list -- positional access walks from whichever end is closer to the requested index.

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: key in x

        Determines whether 'key' exists

    .. describe:: len(x)

        Retrieves the number of entries

    .. describe:: x[index]

        Retrieves the ``(key, value)`` pair at the given true positional index

    .. describe:: iter(x)

        Iterates every entry in true positional order, yielding ``(key, value,
        occurrenceIndex, orderIndex)`` tuples

.. note::
    Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
    deep copy (equivalent to ``x.copy()``)

Parameters
----------
items: Optional[List[Tuple[Any, Any]]]
    Key-value pairs to insert at the end, in order :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None`` (an empty instance)
        )doc")

        .def(py::init([](const std::optional<std::vector<std::pair<py::object, py::object>>> &items) {
            if (!items.has_value()) return PyOrderedMultiMap();
            return PyOrderedMultiMap(*items);
        }), py::arg("items") = py::none())

        .def_static("fromIndexed", [](const py::dict &indexed) {
            tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> converted;
            for (auto item : indexed) {
                py::object key = py::reinterpret_borrow<py::object>(item.first);
                converted[key] = item.second.cast<std::vector<std::pair<long long, py::object>>>();
            }
            return PyOrderedMultiMap::fromIndexed(converted);
        }, py::arg("indexed"),
    py::doc(R"doc(
Builds an instance from a fully-indexed description: for each key, a list of ``(index, value)``
pairs. The index is treated as a sort key, not a strict absolute position: every ``(index, key,
value)`` triple across every key is gathered, stable-sorted by index ascending, and inserted in
that order -- gaps and out-of-order values just determine relative order, and duplicate indices
land consecutively (tie-broken by encounter order: list order within a key, then 'indexed's own
dict order across different keys).

Parameters
----------
indexed: Dict[Any, List[Tuple[:class:`int`, Any]]]
    The key -> list of ``(index, value)`` pairs to build from

Returns
-------
:class:`OrderedMultiMap`
    The newly-built instance
        )doc"))

        .def("insert", &PyOrderedMultiMap::insert, py::arg("key"), py::arg("value"),
    py::doc(R"doc(Appends a key-value pair to the end)doc"))

        .def("insertStart", &PyOrderedMultiMap::insertStart, py::arg("key"), py::arg("value"),
    py::doc(R"doc(Inserts a key-value pair at the beginning)doc"))

        .def("insertAt", &PyOrderedMultiMap::insertAt, py::arg("index"), py::arg("key"), py::arg("value"),
    py::doc(R"doc(
Inserts a key-value pair so it ends up at position 'index' (0-based) :raw-html:`<br />` :raw-html:`<br />`

Supports Python-style negative indices. Out-of-range indices are clamped rather than rejected:
an index greater than ``len(self)`` is treated as ``len(self)`` (append); an index less than
``-(len(self) + 1)`` is treated as ``-(len(self) + 1)`` (front)

Parameters
----------
index: :class:`int`
    The target position

key: Any
    The key of the pair to insert

value: Any
    The value of the pair to insert
        )doc"))

        .def("insertAllEnd", &PyOrderedMultiMap::insertAllEnd, py::arg("items"),
    py::doc(R"doc(Appends a batch of key-value pairs to the end, in the order given)doc"))

        .def("insertAllStart", &PyOrderedMultiMap::insertAllStart, py::arg("items"),
    py::doc(R"doc(
Inserts a batch of key-value pairs at the beginning, in the order given -- ``items[0]`` ends up
first, ``items[1]`` right after it, and so on, all before whatever was originally at the front
        )doc"))

        .def("insertAllAt", &PyOrderedMultiMap::pyInsertAllAt, py::arg("items"), py::arg("sortIndices") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Bulk indexed insert: inserts many key-value pairs at their own target indices in a single pass.
Index semantics match :meth:`insertAt` (Python-style negative indices, clamping), but with
"original position" (numpy-style) semantics: each index refers to a position in the sequence as
it was *before* this call, not a position in the growing result.

Parameters
----------
items: Dict[:class:`int`, Tuple[Any, Any]]
    Maps an index to insert at -> the key-value pair to insert there

sortIndices: :class:`bool`
    If ``True`` (the default), 'items' is stable-sorted by normalized index first. If you
    already know 'items' iterates in ascending normalized-index order, pass ``False`` to skip
    that sort -- **this precondition is unchecked**, and violating it produces a silently wrong
    (not crashing) result

ranges: Optional[:class:`Ranges`]
    If provided, an entry is only inserted when its normalized target index falls within
    'ranges'; filtered entries are dropped before sorting/the insertion pass :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`int`
    How many entries were actually inserted
        )doc"))

        .def("reorder", &PyOrderedMultiMap::pyReorder, py::arg("orderMap"), py::arg("ranges") = py::none(),
    py::doc(R"doc(
Reorders existing entries in place. 'orderMap' maps an old index -> new index for a subset (or
all) of the current entries; every entry not mentioned keeps its relative order and fills
whatever slots are left over :raw-html:`<br />` :raw-html:`<br />`

**Old-index (key) semantics:** must be in ``[-len(self), len(self) - 1]`` -- anything outside
that raises :class:`IndexError`. :raw-html:`<br />` :raw-html:`<br />`

**New-index (value) semantics:** also Python-style, but out-of-range values are bucketed rather
than rejected: a value ``>= len(self)`` goes in a trailing cluster at the very end, a value
``< -len(self)`` goes in a leading cluster at the very front, and within a cluster a smaller raw
value sorts earlier. :raw-html:`<br />` :raw-html:`<br />`

**Conflicts:** if two distinct entries of 'orderMap' target the same physical old entry, or the
same effective new-index target, dict iteration order (Python 3.7+ insertion order) breaks the
tie.

Parameters
----------
orderMap: Dict[:class:`int`, :class:`int`]
    The old index -> new index mapping to apply

ranges: Optional[:class:`Ranges`]
    If provided, an 'orderMap' entry only takes effect when its old index falls within 'ranges';
    otherwise it's ignored entirely, and the old position it would have pinned floats instead :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc"))

        .def("removeAt", [](PyOrderedMultiMap &self, size_t pos, const std::optional<PyRanges<long long>> &ranges) {
            return self.removeAt(pos, toCoreRanges(ranges));
        }, py::arg("pos"), py::arg("ranges") = py::none(),
    py::doc(R"doc(
Removes the entry currently at position 'pos'

Parameters
----------
pos: :class:`int`
    The position of the entry to remove

ranges: Optional[:class:`Ranges`]
    If provided, removal only proceeds when 'pos' falls within 'ranges'; otherwise this call is
    a no-op, same as an out-of-bounds 'pos' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`bool`
    Whether an entry was actually removed
        )doc"))

        .def("removeKey", [](PyOrderedMultiMap &self, const py::object &key, const std::optional<PyRanges<long long>> &ranges,
                              const std::optional<PyOrderedMultiMap::Base::RemoveKeyCheck> &check) {
            return self.removeKey(key, toCoreRanges(ranges), check);
        }, py::arg("key"), py::arg("ranges") = py::none(), py::arg("check") = py::none(),
    py::doc(R"doc(
Removes every entry with this key, subject to two independent, optional filters -- both must
hold (where provided) for a given occurrence to actually be removed. With neither filter
provided, this is unconditional removal of every entry with this key.

Parameters
----------
key: Any
    The key whose entries to remove

ranges: Optional[:class:`Ranges`]
    If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
    must fall within 'ranges' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

check: Optional[Callable[[:class:`int`, Any], :class:`bool`]]
    If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
    positional index and value :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`int`
    How many entries were actually removed
        )doc"))

        .def("remapKeys", &PyOrderedMultiMap::pyRemapKeys, py::arg("keyRemap"), py::arg("ranges") = py::none(),
    py::doc(R"doc(
Bulk-renames keys. 'keyRemap' maps an old key -> either a bare list of keys/
:class:`RemappedKeyData`, or a :class:`KeyRemapData`. :raw-html:`<br />` :raw-html:`<br />`

For every existing entry, walked in true positional order: if its key is not a key in
'keyRemap', it's left completely unchanged. Otherwise, each rule in the mapped list is evaluated
independently against this occurrence -- a plain key always fires, a :class:`RemappedKeyData`
fires if it has no ``check``, or ``check(oldKey, oldValue)`` is ``True``. Every rule that fires
produces one new entry (that rule's key, this occurrence's original value); a
:class:`RemappedKeyData` with ``toInd`` set instead moves its entry (as part of a group with
every other entry sharing that same ``toInd`` across every occurrence) to that target index,
using :meth:`reorder`'s exact index semantics. :raw-html:`<br />` :raw-html:`<br />`

If zero rules fire for a given occurrence: with a bare list, or ``keepKeyWithoutRemap=False``,
that occurrence is removed entirely. With ``keepKeyWithoutRemap=True`` (via
:class:`KeyRemapData`), it retains its original ``(key, value)`` pair instead. :raw-html:`<br />` :raw-html:`<br />`

Old keys mentioned in 'keyRemap' that don't actually exist right now are simply never
triggered -- no error, nothing happens. This is a single pass over the original entries:
newly-created entries are never looked up in 'keyRemap' again, so there's no cascading/recursive
re-application.

Parameters
----------
keyRemap: Dict[Any, Union[List[Union[Any, :class:`RemappedKeyData`]], :class:`KeyRemapData`]]
    The old key -> remap rules mapping to apply

ranges: Optional[:class:`Ranges`]
    If provided, an occurrence outside 'ranges' is treated exactly as if its key were never
    mentioned in 'keyRemap' at all -- a pure pass-through :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc"))

        .def("replaceVals", &PyOrderedMultiMap::pyReplaceVals, py::arg("newVals"), py::arg("addNew") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Bulk-updates values by key. 'newVals' maps a key -> either a bare replacement value, a
:class:`ReplaceList` (positional, by existing true left-to-right order), or a :class:`ReplaceIf`
(conditional, by predicate).

Parameters
----------
newVals: Dict[Any, Union[Any, :class:`ReplaceList`, :class:`ReplaceIf`]]
    The key -> replace spec mapping to apply

addNew: :class:`bool`
    What to do when a key in 'newVals' doesn't currently exist. If ``True`` (the default), it's
    added, appended at the end (a bare value -> one entry; :class:`ReplaceList` -> one entry per
    value, in order; :class:`ReplaceIf` -> one entry with just the value, predicate ignored
    since there's nothing existing to test it against). If ``False``, the key is skipped
    entirely; no error.

ranges: Optional[:class:`Ranges`]
    If provided, gates whether an existing entry's value actually gets replaced, on top of
    whatever the spec itself already decides -- both must hold. Not consulted for 'addNew' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc"))

        .def("contains", &PyOrderedMultiMap::contains, py::arg("key"), py::doc(R"doc(Checks whether a key exists)doc"))
        .def("containsKey", &PyOrderedMultiMap::containsKey, py::arg("key"), py::doc(R"doc(Checks whether a key exists)doc"))
        .def("__contains__", &PyOrderedMultiMap::contains, py::arg("key"), py::doc(R"doc(Determines whether 'key' exists)doc"))

        .def("count", &PyOrderedMultiMap::count, py::arg("key"), py::doc(R"doc(Retrieves how many entries share a given key)doc"))

        .def("size", &PyOrderedMultiMap::size, py::doc(R"doc(Retrieves the number of entries)doc"))
        .def("length", &PyOrderedMultiMap::length, py::doc(R"doc(Retrieves the number of entries)doc"))
        .def("__len__", &PyOrderedMultiMap::size, py::doc(R"doc(Retrieves the number of entries)doc"))

        .def("keySize", &PyOrderedMultiMap::keySize, py::doc(R"doc(Retrieves the number of distinct keys)doc"))

        .def("empty", &PyOrderedMultiMap::empty, py::doc(R"doc(Checks whether the map is empty)doc"))

        .def("getAll", [](const PyOrderedMultiMap &self, const py::object &key, bool ordered, const std::optional<PyRanges<long long>> &ranges) {
            return self.getAll(key, ordered, toCoreRanges(ranges));
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

        .def("getAllWithInds", [](const PyOrderedMultiMap &self, const py::object &key, bool ordered, const std::optional<PyRanges<long long>> &ranges) {
            return self.getAll(key, ordered, true, toCoreRanges(ranges));
        }, py::arg("key"), py::arg("ordered") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Retrieves all values currently stored under a key, each paired with its true positional index
(equivalent to :meth:`getAll`, except each value is paired with its true positional index)

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
    The ``(order index, value)`` pairs for this key, in the requested order
        )doc"))

        .def("getKeys", &PyOrderedMultiMap::getKeys,
    py::doc(R"doc(
Retrieves every distinct key currently in the map

Returns
-------
Set[Any]
    Every distinct key, as a set (unordered)
        )doc"))

        .def("getByInd", [](const PyOrderedMultiMap &self, long long index) {
            auto result = self.getByInd(index);
            return std::make_tuple(result.first, result.second);
        }, py::arg("index"),
    py::doc(R"doc(
Retrieves the entry at a true positional index

Parameters
----------
index: :class:`int`
    The position to retrieve. Python-style negative indices are supported

Raises
------
:class:`IndexError`
    If 'index' is out of range

Returns
-------
Tuple[Any, Any]
    The ``(key, value)`` pair at that position
        )doc"))

        .def("getByIndWithOccurrence", [](const PyOrderedMultiMap &self, long long index) {
            return self.getByInd(index, true);
        }, py::arg("index"),
    py::doc(R"doc(
Retrieves the entry at a true positional index, paired with its occurrence index (how many
times this same key already appeared earlier in the sequence, 0-based) instead of its key
(equivalent to :meth:`getByInd`, except the entry's value is paired with its occurrence index)

Parameters
----------
index: :class:`int`
    The position to retrieve. Python-style negative indices are supported

Raises
------
:class:`IndexError`
    If 'index' is out of range

Returns
-------
Tuple[:class:`int`, Any]
    The ``(occurrence index, value)`` pair at that position
        )doc"))

        .def("setValByInd", &PyOrderedMultiMap::setValByInd, py::arg("index"), py::arg("value"),
    py::doc(R"doc(
Sets the value of the entry at a true positional index, leaving its key untouched

Parameters
----------
index: :class:`int`
    The position to update. Python-style negative indices are supported

value: Any
    The new value for that entry

Raises
------
:class:`IndexError`
    If 'index' is out of range
        )doc"))

        .def("__getitem__", [](const PyOrderedMultiMap &self, long long index) {
            auto result = self.getByInd(index);
            return std::make_tuple(result.first, result.second);
        }, py::arg("index"), py::doc(R"doc(Retrieves the ``(key, value)`` pair at a true positional index)doc"))

        .def("entries", [](const PyOrderedMultiMap &self) {
            std::vector<std::pair<py::object, py::object>> result;
            const auto &order = self.entries();
            result.reserve(order.size());
            for (const auto &entry : order) {
                result.emplace_back(entry.key, entry.value);
            }
            return result;
        }, py::doc(R"doc(
Retrieves read-only access to the full ordered sequence

Returns
-------
List[Tuple[Any, Any]]
    The full ordered sequence of ``(key, value)`` pairs
        )doc"))

        .def("splitByInds", &PyOrderedMultiMap::pySplitByInds,
             py::arg("inds"), py::arg("includeSplitKVP") = true, py::arg("includeEmptyParts") = false, py::arg("sortIndices") = true,
    py::doc(R"doc(
Splits this map into several smaller maps at the given indices, preserving relative order both
within each part and across parts. Each resulting part is a genuinely independent new instance --
mutating one part never affects the original or any sibling part.

'inds' uses the same index convention as :meth:`getByInd`. Each index becomes a boundary using
standard slice semantics: everything before it goes in the earlier part, everything from it
onward starts the next part.

Parameters
----------
inds: List[:class:`int`]
    The indices at which to split

includeSplitKVP: :class:`bool`
    What happens to the entry at each split point. If ``True`` (the default), it starts the
    later part. If ``False``, it's dropped entirely, belonging to neither part

includeEmptyParts: :class:`bool`
    Whether empty parts are included in the result or silently dropped :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

sortIndices: :class:`bool`
    If ``True`` (the default), 'inds' is normalized, deduplicated, and sorted ascending first.
    If you already know 'inds' iterates in that exact order, pass ``False`` to skip that pass --
    **this precondition is unchecked**, and violating it produces a silently wrong (not
    crashing) result

Raises
------
:class:`IndexError`
    If an index in 'inds' is out of range

Returns
-------
List[:class:`OrderedMultiMap`]
    The resulting parts, left to right
        )doc"))

        .def("__iter__", [](const PyOrderedMultiMap &self) {
            return PyOrderedMultiMapIterator(self.begin(), self.end());
        }, py::keep_alive<0, 1>(),
    py::doc(R"doc(Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples)doc"))

        .def("copy", [](const PyOrderedMultiMap &self) { return PyOrderedMultiMap(self); },
    py::doc(R"doc(
Creates a deep copy of this instance -- rebuilt entry-by-entry, so the copy shares no internal
state with the original

Returns
-------
:class:`OrderedMultiMap`
    The newly-created copy
        )doc"))

        .def("__copy__", [](const PyOrderedMultiMap &self) { return PyOrderedMultiMap(self); },
    py::doc(R"doc(Creates a copy of this instance (equivalent to :meth:`copy`); supports ``copy.copy()``)doc"))

        .def("__deepcopy__", [](const PyOrderedMultiMap &self, py::dict) { return PyOrderedMultiMap(self); }, py::arg("memo"),
    py::doc(R"doc(Creates a deep copy of this instance (equivalent to :meth:`copy`); supports ``copy.deepcopy()``)doc"))

        .def("asInterface", [](const PyOrderedMultiMap &self) -> std::unique_ptr<AGRC::IOrderedMultiMap<py::object, py::object>> {
            return std::make_unique<AGRC::OrderedMultiMapListAdapter<py::object, py::object, PyObjectHash, PyObjectEqual>>(
                static_cast<const PyOrderedMultiMap::Base&>(self));
        }, py::doc(R"doc(
Creates an independent snapshot of this instance, viewed through the generic
:class:`IOrderedMultiMap` interface -- like :meth:`copy`, this is a deep copy; mutating the
result does not affect this instance (or vice versa)

Returns
-------
:class:`IOrderedMultiMap`
    An independent :class:`IOrderedMultiMap`-typed snapshot of this instance
        )doc"));
}
