#include "PyIOrderedMultiMap.h"


// See PyOrderedMultiMap.cpp's top-of-file comment for why K/V are 'py::object' and why the
// std::variant-typed parameters (KeyRemapValue, ReplaceSpec) need manual dispatch rather than
// pybind11's automatic std::variant support -- everything there applies identically here.
// PyIOrderedMultiMap.h's own doc comment covers the one thing specific to this file: why
// 'ranges' is IOrderedMultiMap::RangeSpec (a plain std::vector), not Ranges<long long> itself.
namespace py = pybind11;
namespace AGRC = AGRemapCore;


// PYBIND11_OVERRIDE_PURE splits its first (return-type) argument on literal commas, so any
// return type with an unparenthesized template-argument comma needs a type alias first --
// same convention already used in PyAhoCorasickDFA.cpp.
using GetByIndReturn = std::pair<py::object, py::object>;
using GetByIndWithOccurrenceReturn = std::pair<long long, py::object>;
using EntriesReturn = std::vector<std::pair<py::object, py::object>>;
using GetAllWithIndsReturn = std::vector<std::pair<long long, py::object>>;
using SplitByIndsReturn = std::vector<std::unique_ptr<PyIOrderedMultiMap>>;


// Accepts either a bound Ranges instance (the common case, matching OrderedMultiMap's own
// convention) or a raw list of (start, end) bounds (RangeSpec's own native shape, needed since
// that's what a Python subclass's OWN override of a ranges-taking method receives -- see
// PyIOrderedMultiMap.h's doc comment) -- and None. Declared in PyIOrderedMultiMap.h so other
// binding files can reuse these instead of duplicating them.
std::optional<PyIOrderedMultiMap::RangeSpec> parseRanges(const py::object &ranges) {
    if (ranges.is_none()) {
        return std::nullopt;
    }
    if (py::isinstance<PyRanges<long long>>(ranges)) {
        return ranges.cast<PyRanges<long long>>().ranges;
    }
    return ranges.cast<PyIOrderedMultiMap::RangeSpec>();
}

// Manual dispatch, not pybind11's automatic std::variant support -- see PyOrderedMultiMap.cpp's
// top-of-file comment for why a bare key (py::object) always wins that race.
PyIOrderedMultiMap::KeyRemapList parseKeyRemapList(const py::object &value) {
    PyIOrderedMultiMap::KeyRemapList list;
    for (py::handle elemH : value) {
        py::object elem = py::reinterpret_borrow<py::object>(elemH);
        if (py::isinstance<PyRemappedKeyData>(elem)) {
            list.emplace_back(elem.cast<PyRemappedKeyData>());
        } else {
            list.emplace_back(elem);
        }
    }
    return list;
}

std::vector<std::pair<py::object, PyIOrderedMultiMap::KeyRemapValue>> parseKeyRemap(const py::dict &keyRemap) {
    std::vector<std::pair<py::object, PyIOrderedMultiMap::KeyRemapValue>> result;
    result.reserve(keyRemap.size());
    for (auto item : keyRemap) {
        py::object key = py::reinterpret_borrow<py::object>(item.first);
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        if (py::isinstance<PyKeyRemapData>(value)) {
            result.emplace_back(std::move(key), value.cast<PyKeyRemapData>());
            continue;
        }

        if (!py::isinstance<py::list>(value) && !py::isinstance<py::tuple>(value)) {
            throw py::type_error(
                "remapKeys(): each keyRemap value must be a KeyRemapData instance, or a "
                "list/tuple mixing bare keys and RemappedKeyData instances");
        }
        result.emplace_back(std::move(key), parseKeyRemapList(value));
    }
    return result;
}

std::vector<std::pair<py::object, PyIOrderedMultiMap::ReplaceSpec>> parseReplaceVals(const py::dict &newVals) {
    std::vector<std::pair<py::object, PyIOrderedMultiMap::ReplaceSpec>> result;
    result.reserve(newVals.size());
    for (auto item : newVals) {
        py::object key = py::reinterpret_borrow<py::object>(item.first);
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        if (py::isinstance<PyReplaceList>(value)) {
            result.emplace_back(std::move(key), value.cast<PyReplaceList>().values());
        } else if (py::isinstance<PyReplaceIf>(value)) {
            PyReplaceIf spec = value.cast<PyReplaceIf>();
            result.emplace_back(std::move(key), std::make_pair(spec.value(), spec.predicate()));
        } else {
            result.emplace_back(std::move(key), value);
        }
    }
    return result;
}

std::vector<std::pair<long long, std::pair<py::object, py::object>>> parseInsertAllAtItems(const py::dict &items) {
    std::vector<std::pair<long long, std::pair<py::object, py::object>>> result;
    result.reserve(items.size());
    for (auto item : items) {
        long long index = item.first.cast<long long>();
        auto kv = item.second.cast<std::pair<py::object, py::object>>();
        result.emplace_back(index, std::move(kv));
    }
    return result;
}

std::vector<std::pair<long long, long long>> parseOrderMap(const py::dict &orderMap) {
    std::vector<std::pair<long long, long long>> result;
    result.reserve(orderMap.size());
    for (auto item : orderMap) {
        result.emplace_back(item.first.cast<long long>(), item.second.cast<long long>());
    }
    return result;
}


// ------------------------------------------------------------------
// PyBindIOrderedMultiMap
// ------------------------------------------------------------------

void PyBindIOrderedMultiMap::insert(const py::object &key, const py::object &value) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, insert, key, value);
}

void PyBindIOrderedMultiMap::insertStart(const py::object &key, const py::object &value) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, insertStart, key, value);
}

void PyBindIOrderedMultiMap::insertAt(long long index, const py::object &key, const py::object &value) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, insertAt, index, key, value);
}

void PyBindIOrderedMultiMap::insertAllEnd(const std::vector<std::pair<py::object, py::object>> &items) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, insertAllEnd, items);
}

void PyBindIOrderedMultiMap::insertAllStart(const std::vector<std::pair<py::object, py::object>> &items) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, insertAllStart, items);
}

size_t PyBindIOrderedMultiMap::insertAllAt(const std::vector<std::pair<long long, std::pair<py::object, py::object>>> &items,
                                            bool sortIndices, const std::optional<RangeSpec> &ranges) {
    PYBIND11_OVERRIDE_PURE(size_t, PyIOrderedMultiMap, insertAllAt, items, sortIndices, ranges);
}

void PyBindIOrderedMultiMap::reorder(const std::vector<std::pair<long long, long long>> &orderMap,
                                      const std::optional<RangeSpec> &ranges) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, reorder, orderMap, ranges);
}

bool PyBindIOrderedMultiMap::removeAt(size_t pos, const std::optional<RangeSpec> &ranges) {
    PYBIND11_OVERRIDE_PURE(bool, PyIOrderedMultiMap, removeAt, pos, ranges);
}

size_t PyBindIOrderedMultiMap::removeKey(const py::object &key, const std::optional<RangeSpec> &ranges,
                                          const std::optional<RemoveKeyCheck> &check) {
    PYBIND11_OVERRIDE_PURE(size_t, PyIOrderedMultiMap, removeKey, key, ranges, check);
}

void PyBindIOrderedMultiMap::remapKeys(const std::vector<std::pair<py::object, KeyRemapValue>> &keyRemap,
                                        const std::optional<RangeSpec> &ranges) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, remapKeys, keyRemap, ranges);
}

void PyBindIOrderedMultiMap::replaceVals(const std::vector<std::pair<py::object, ReplaceSpec>> &newVals, bool addNew,
                                          const std::optional<RangeSpec> &ranges) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, replaceVals, newVals, addNew, ranges);
}

bool PyBindIOrderedMultiMap::contains(const py::object &key) const {
    PYBIND11_OVERRIDE_PURE(bool, PyIOrderedMultiMap, contains, key);
}

bool PyBindIOrderedMultiMap::containsKey(const py::object &key) const {
    PYBIND11_OVERRIDE_PURE(bool, PyIOrderedMultiMap, containsKey, key);
}

size_t PyBindIOrderedMultiMap::count(const py::object &key) const {
    PYBIND11_OVERRIDE_PURE(size_t, PyIOrderedMultiMap, count, key);
}

size_t PyBindIOrderedMultiMap::size() const {
    PYBIND11_OVERRIDE_PURE(size_t, PyIOrderedMultiMap, size);
}

size_t PyBindIOrderedMultiMap::length() const {
    PYBIND11_OVERRIDE_PURE(size_t, PyIOrderedMultiMap, length);
}

size_t PyBindIOrderedMultiMap::keySize() const {
    PYBIND11_OVERRIDE_PURE(size_t, PyIOrderedMultiMap, keySize);
}

bool PyBindIOrderedMultiMap::empty() const {
    PYBIND11_OVERRIDE_PURE(bool, PyIOrderedMultiMap, empty);
}

std::vector<py::object> PyBindIOrderedMultiMap::getAll(const py::object &key, bool ordered, const std::optional<RangeSpec> &ranges) const {
    PYBIND11_OVERRIDE_PURE(std::vector<py::object>, PyIOrderedMultiMap, getAll, key, ordered, ranges);
}

std::vector<std::pair<long long, py::object>> PyBindIOrderedMultiMap::getAllWithInds(const py::object &key, bool ordered, const std::optional<RangeSpec> &ranges) const {
    PYBIND11_OVERRIDE_PURE(GetAllWithIndsReturn, PyIOrderedMultiMap, getAllWithInds, key, ordered, ranges);
}

std::vector<py::object> PyBindIOrderedMultiMap::getKeys() const {
    PYBIND11_OVERRIDE_PURE(std::vector<py::object>, PyIOrderedMultiMap, getKeys);
}

std::pair<py::object, py::object> PyBindIOrderedMultiMap::getByInd(long long index) const {
    PYBIND11_OVERRIDE_PURE(GetByIndReturn, PyIOrderedMultiMap, getByInd, index);
}

std::pair<long long, py::object> PyBindIOrderedMultiMap::getByIndWithOccurrence(long long index) const {
    PYBIND11_OVERRIDE_PURE(GetByIndWithOccurrenceReturn, PyIOrderedMultiMap, getByIndWithOccurrence, index);
}

void PyBindIOrderedMultiMap::setValByInd(long long index, const py::object &value) {
    PYBIND11_OVERRIDE_PURE(void, PyIOrderedMultiMap, setValByInd, index, value);
}

std::vector<std::pair<py::object, py::object>> PyBindIOrderedMultiMap::entries() const {
    PYBIND11_OVERRIDE_PURE(EntriesReturn, PyIOrderedMultiMap, entries);
}

std::vector<PyIOrderedMultiMap::Item> PyBindIOrderedMultiMap::items() const {
    PYBIND11_OVERRIDE_PURE(std::vector<Item>, PyIOrderedMultiMap, items);
}

std::vector<std::unique_ptr<PyIOrderedMultiMap>> PyBindIOrderedMultiMap::splitByInds(
    const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices) const {
    PYBIND11_OVERRIDE_PURE(SplitByIndsReturn, PyIOrderedMultiMap, splitByInds,
                            inds, includeSplitKVP, includeEmptyParts, sortIndices);
}

std::unique_ptr<PyIOrderedMultiMap> PyBindIOrderedMultiMap::clone() const {
    PYBIND11_OVERRIDE_PURE(std::unique_ptr<PyIOrderedMultiMap>, PyIOrderedMultiMap, clone);
}


// ------------------------------------------------------------------
// module registration
// ------------------------------------------------------------------

void initCppIOrderedMultiMap(pybind11::module_ &m) {
    // py::smart_holder (not the default std::unique_ptr holder) is required here specifically
    // so ownership of a bound IOrderedMultiMap instance can be *transferred* from Python into a
    // C++ std::unique_ptr<PyIOrderedMultiMap> parameter (e.g. IfContentPart's constructor) --
    // the default holder only supports handing a fresh unique_ptr *out* to Python (return
    // values), not consuming one *in* from an existing Python-owned object.
    py::class_<PyIOrderedMultiMap, PyBindIOrderedMultiMap, py::smart_holder>(m, "IOrderedMultiMap", R"doc(
An abstract ordered-multimap interface: implement every method below (in a Python subclass) to
plug an entirely custom backing structure into any C++ code that accepts this interface --
:class:`OrderedMultiMap` and :class:`OrderedMultiMapSqrt` are two such implementations,
each exposed as an :class:`IOrderedMultiMap` via their own ``asInterface()`` method.

.. note::
    Unlike :class:`OrderedMultiMap`, the ``ranges`` parameter accepted throughout this
    interface takes either a bound :class:`Ranges` instance or a plain
    ``List[Tuple[Optional[int], Optional[int]]]`` of ``(start, end)`` bounds -- a Python
    subclass's own override of a ``ranges``-taking method always receives the latter, plain
    shape.
        )doc")

        .def(py::init<>())

        .def("insert", &PyIOrderedMultiMap::insert, py::arg("key"), py::arg("value"),
    py::doc(R"doc(Appends a key-value pair to the end)doc"))

        .def("insertStart", &PyIOrderedMultiMap::insertStart, py::arg("key"), py::arg("value"),
    py::doc(R"doc(Inserts a key-value pair at the beginning)doc"))

        .def("insertAt", &PyIOrderedMultiMap::insertAt, py::arg("index"), py::arg("key"), py::arg("value"),
    py::doc(R"doc(Inserts a key-value pair so it ends up at position 'index' (0-based); see :meth:`OrderedMultiMap.insertAt` for the full index semantics)doc"))

        .def("insertAllEnd", &PyIOrderedMultiMap::insertAllEnd, py::arg("items"),
    py::doc(R"doc(Appends a batch of key-value pairs to the end, in the order given)doc"))

        .def("insertAllStart", &PyIOrderedMultiMap::insertAllStart, py::arg("items"),
    py::doc(R"doc(Inserts a batch of key-value pairs at the beginning, in the order given)doc"))

        .def("insertAllAt", [](PyIOrderedMultiMap &self, const py::dict &items, bool sortIndices, const py::object &ranges) {
            return self.insertAllAt(parseInsertAllAtItems(items), sortIndices, parseRanges(ranges));
        }, py::arg("items"), py::arg("sortIndices") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(Bulk indexed insert; see :meth:`OrderedMultiMap.insertAllAt` for the full semantics)doc"))

        .def("reorder", [](PyIOrderedMultiMap &self, const py::dict &orderMap, const py::object &ranges) {
            self.reorder(parseOrderMap(orderMap), parseRanges(ranges));
        }, py::arg("orderMap"), py::arg("ranges") = py::none(),
    py::doc(R"doc(Reorders existing entries in place; see :meth:`OrderedMultiMap.reorder` for the full semantics)doc"))

        .def("removeAt", [](PyIOrderedMultiMap &self, size_t pos, const py::object &ranges) {
            return self.removeAt(pos, parseRanges(ranges));
        }, py::arg("pos"), py::arg("ranges") = py::none(),
    py::doc(R"doc(Removes the entry currently at position 'pos')doc"))

        .def("removeKey", [](PyIOrderedMultiMap &self, const py::object &key, const py::object &ranges,
                              const std::optional<PyIOrderedMultiMap::RemoveKeyCheck> &check) {
            return self.removeKey(key, parseRanges(ranges), check);
        }, py::arg("key"), py::arg("ranges") = py::none(), py::arg("check") = py::none(),
    py::doc(R"doc(
Removes every entry with this key, subject to optional 'ranges'/'check' filters; see
:meth:`OrderedMultiMap.removeKey` for the full semantics -- 'check', if provided, is
``check(index, value)``
        )doc"))

        .def("remapKeys", [](PyIOrderedMultiMap &self, const py::dict &keyRemap, const py::object &ranges) {
            self.remapKeys(parseKeyRemap(keyRemap), parseRanges(ranges));
        }, py::arg("keyRemap"), py::arg("ranges") = py::none(),
    py::doc(R"doc(Bulk-renames keys; see :meth:`OrderedMultiMap.remapKeys` for the full semantics)doc"))

        .def("replaceVals", [](PyIOrderedMultiMap &self, const py::dict &newVals, bool addNew, const py::object &ranges) {
            self.replaceVals(parseReplaceVals(newVals), addNew, parseRanges(ranges));
        }, py::arg("newVals"), py::arg("addNew") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(Bulk-updates values by key; see :meth:`OrderedMultiMap.replaceVals` for the full semantics)doc"))

        .def("contains", &PyIOrderedMultiMap::contains, py::arg("key"), py::doc(R"doc(Checks whether a key exists)doc"))
        .def("containsKey", &PyIOrderedMultiMap::containsKey, py::arg("key"), py::doc(R"doc(Checks whether a key exists)doc"))
        .def("__contains__", &PyIOrderedMultiMap::contains, py::arg("key"), py::doc(R"doc(Determines whether 'key' exists)doc"))

        .def("count", &PyIOrderedMultiMap::count, py::arg("key"), py::doc(R"doc(Retrieves how many entries share a given key)doc"))

        .def("size", &PyIOrderedMultiMap::size, py::doc(R"doc(Retrieves the number of entries)doc"))
        .def("length", &PyIOrderedMultiMap::length, py::doc(R"doc(Retrieves the number of entries)doc"))
        .def("__len__", &PyIOrderedMultiMap::size, py::doc(R"doc(Retrieves the number of entries)doc"))

        .def("keySize", &PyIOrderedMultiMap::keySize, py::doc(R"doc(Retrieves the number of distinct keys)doc"))

        .def("empty", &PyIOrderedMultiMap::empty, py::doc(R"doc(Checks whether the map is empty)doc"))

        .def("getAll", [](const PyIOrderedMultiMap &self, const py::object &key, bool ordered, const py::object &ranges) {
            return self.getAll(key, ordered, parseRanges(ranges));
        }, py::arg("key"), py::arg("ordered") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Retrieves all values currently stored under a key; see :meth:`CppOrderedMultiMap.getAll` for the
full semantics
        )doc"))

        .def("getAllWithInds", [](const PyIOrderedMultiMap &self, const py::object &key, bool ordered, const py::object &ranges) {
            return self.getAllWithInds(key, ordered, parseRanges(ranges));
        }, py::arg("key"), py::arg("ordered") = true, py::arg("ranges") = py::none(),
    py::doc(R"doc(
Retrieves all values currently stored under a key, each paired with its true positional index;
see :meth:`CppOrderedMultiMap.getAllWithInds` for the full semantics
        )doc"))

        .def("getKeys", &PyIOrderedMultiMap::getKeys,
    py::doc(R"doc(
Retrieves every distinct key currently in the map, as a :class:`list` rather than a real
``set`` -- unlike :meth:`CppOrderedMultiMap.getKeys`, this interface has no way to guarantee an
arbitrary key type is hashable
        )doc"))

        .def("getByInd", &PyIOrderedMultiMap::getByInd, py::arg("index"),
    py::doc(R"doc(Retrieves the ``(key, value)`` pair at a true positional index)doc"))

        .def("getByIndWithOccurrence", &PyIOrderedMultiMap::getByIndWithOccurrence, py::arg("index"),
    py::doc(R"doc(Retrieves the entry at a true positional index, paired with its occurrence index)doc"))

        .def("setValByInd", &PyIOrderedMultiMap::setValByInd, py::arg("index"), py::arg("value"),
    py::doc(R"doc(Sets the value of the entry at a true positional index, leaving its key untouched)doc"))

        .def("__getitem__", &PyIOrderedMultiMap::getByInd, py::arg("index"),
    py::doc(R"doc(Retrieves the ``(key, value)`` pair at a true positional index)doc"))

        .def("entries", &PyIOrderedMultiMap::entries,
    py::doc(R"doc(Retrieves a copy of the full ordered sequence, as ``(key, value)`` pairs)doc"))

        .def("items", [](const PyIOrderedMultiMap &self) {
            // Item isn't itself pybind-castable (a plain aggregate, not tuple-shaped) --
            // copied out into plain tuples here, same as OrderedMultiMap's __iter__.
            std::vector<std::tuple<py::object, py::object, size_t, size_t>> result;
            for (const auto &item : self.items()) {
                result.emplace_back(item.key, item.value, item.occurrenceIndex, item.orderIndex);
            }
            return result;
        }, py::doc(R"doc(Retrieves a copy of the full ordered sequence, as ``(key, value, occurrenceIndex, orderIndex)`` tuples)doc"))

        .def("__iter__", [](const PyIOrderedMultiMap &self) {
            std::vector<std::tuple<py::object, py::object, size_t, size_t>> result;
            for (const auto &item : self.items()) {
                result.emplace_back(item.key, item.value, item.occurrenceIndex, item.orderIndex);
            }
            // py::iter(), not a bare list -- __iter__ must return an actual iterator (something
            // with __next__), and a plain Python list isn't its own iterator.
            return py::iter(py::cast(result));
        }, py::doc(R"doc(Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples)doc"))

        .def("splitByInds", &PyIOrderedMultiMap::splitByInds,
             py::arg("inds"), py::arg("includeSplitKVP") = true, py::arg("includeEmptyParts") = false, py::arg("sortIndices") = true,
    py::doc(R"doc(Splits this map into several smaller maps at the given indices; see :meth:`OrderedMultiMap.splitByInds` for the full semantics)doc"))

        .def("clone", &PyIOrderedMultiMap::clone,
    py::doc(R"doc(Creates a deep copy of this instance)doc"))

        .def("__copy__", &PyIOrderedMultiMap::clone, py::doc(R"doc(Creates a copy of this instance (equivalent to :meth:`clone`); supports ``copy.copy()``)doc"))
        .def("__deepcopy__", [](const PyIOrderedMultiMap &self, py::dict) { return self.clone(); }, py::arg("memo"),
    py::doc(R"doc(Creates a deep copy of this instance (equivalent to :meth:`clone`); supports ``copy.deepcopy()``)doc"));


    m.def("appendAllToOrderedMultiMap", [](PyIOrderedMultiMap &target, const std::vector<std::pair<py::object, py::object>> &items) {
        AGRC::appendAll<py::object, py::object>(target, items);
    }, py::arg("target"), py::arg("items"),
    py::doc(R"doc(
Appends every ``(key, value)`` pair to any :class:`IOrderedMultiMap` implementation, in order --
a small example of code written once against the interface, working identically whether
'target' is :class:`OrderedMultiMap`/:class:`OrderedMultiMapSqrt` (via their
``asInterface()``) or a user's own Python subclass of :class:`IOrderedMultiMap`.

Parameters
----------
target: :class:`IOrderedMultiMap`
    The map to append to

items: List[Tuple[Any, Any]]
    The key-value pairs to append, in order
        )doc"));
}
