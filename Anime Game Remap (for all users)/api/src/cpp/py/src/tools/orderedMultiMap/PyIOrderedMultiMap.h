#ifndef AGRemapPyBind_PyIOrderedMultiMap_H
#define AGRemapPyBind_PyIOrderedMultiMap_H

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyOrderedMultiMap.h"  // reuses PyRemappedKeyData/PyKeyRemapData/PyReplaceList/PyReplaceIf
#include "AGRemapCore/tools/orderedMultiMap/IOrderedMultiMap.h"
#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMapAdapter.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IOrderedMultiMap`\\<py::object, py::object\\>. Unlike
 `PyOrderedMultiMap`/`PyOrderedMultiMapSqrt`, this is a plain alias rather than a subclass --
 :cpp:class:`AGRC::IOrderedMultiMap` is already fully generic and every one of its parameter
 types (``std::vector``-based, no `tsl::ordered_map`_/``std::unordered_map``, and a plain
 :cpp:type:`AGRC::IOrderedMultiMap::RangeSpec` instead of the polymorphic :cpp:class:`Ranges`
 itself -- see that type's doc comment) is already directly usable from `pybind11`_'s
 ``<pybind11/stl.h>``, so there's nothing to adapt at the C++ level. The Python-facing
 conveniences below (accepting a plain ``dict`` for ``keyRemap``/``newVals``/etc., matching
 `OrderedMultiMap`'s own convention, and accepting either a bound `Ranges` or a raw list of
 bounds for ``ranges``) are handled entirely as plain (non-virtual) lambda bindings in this
 file's ``.cpp``, the same way `PyRanges`'s non-virtual majority is bound directly.
 @endrst
 */
using PyIOrderedMultiMap = AGRC::IOrderedMultiMap<py::object, py::object>;


/**
 * @brief
 @rst
 The `pybind11`_ trampoline class for `PyIOrderedMultiMap`, letting a `Python`_ class implement
 the ordered-multimap interface directly -- every pure virtual method here, when called from
 C++ (e.g. by :cpp:func:`AGRC::appendAll`, or eventually by ``IfContentPart``) on an instance
 backed by a `Python`_ subclass, calls back into that subclass's own method.
 @endrst
 */
class PyBindIOrderedMultiMap: public PyIOrderedMultiMap, public py::trampoline_self_life_support {
    public:
        using PyIOrderedMultiMap::PyIOrderedMultiMap;

        void insert(const py::object &key, const py::object &value) override;
        void insertStart(const py::object &key, const py::object &value) override;
        void insertAt(long long index, const py::object &key, const py::object &value) override;
        void insertAllEnd(const std::vector<std::pair<py::object, py::object>> &items) override;
        void insertAllStart(const std::vector<std::pair<py::object, py::object>> &items) override;
        size_t insertAllAt(const std::vector<std::pair<long long, std::pair<py::object, py::object>>> &items,
                            bool sortIndices, const std::optional<RangeSpec> &ranges) override;

        void reorder(const std::vector<std::pair<long long, long long>> &orderMap,
                     const std::optional<RangeSpec> &ranges) override;

        bool removeAt(size_t pos, const std::optional<RangeSpec> &ranges) override;
        size_t removeKey(const py::object &key, const std::optional<RangeSpec> &ranges,
                          const std::optional<RemoveKeyCheck> &check) override;

        void remapKeys(const std::vector<std::pair<py::object, KeyRemapValue>> &keyRemap,
                        const std::optional<RangeSpec> &ranges) override;

        void replaceVals(const std::vector<std::pair<py::object, ReplaceSpec>> &newVals, bool addNew,
                          const std::optional<RangeSpec> &ranges) override;

        bool contains(const py::object &key) const override;
        bool containsKey(const py::object &key) const override;
        size_t count(const py::object &key) const override;
        size_t size() const override;
        size_t length() const override;
        size_t keySize() const override;
        bool empty() const override;

        std::vector<py::object> getAll(const py::object &key, bool ordered, const std::optional<RangeSpec> &ranges) const override;
        std::vector<std::pair<long long, py::object>> getAllWithInds(const py::object &key, bool ordered, const std::optional<RangeSpec> &ranges) const override;
        std::vector<py::object> getKeys() const override;

        std::pair<py::object, py::object> getByInd(long long index) const override;
        std::pair<long long, py::object> getByIndWithOccurrence(long long index) const override;
        void setValByInd(long long index, const py::object &value) override;

        std::vector<std::pair<py::object, py::object>> entries() const override;
        std::vector<Item> items() const override;

        std::vector<std::unique_ptr<PyIOrderedMultiMap>> splitByInds(
            const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices) const override;

        std::unique_ptr<PyIOrderedMultiMap> clone() const override;
};


// The following free functions convert plain Python dict/list arguments into
// PyIOrderedMultiMap's own vector-based parameter shapes -- shared here (rather than kept as
// file-local helpers) so other binding files (e.g. PyIfContentPart.cpp, which delegates to an
// IOrderedMultiMap and wants the exact same dict-based Python ergonomics) can reuse them without
// duplicating this parsing logic.

/**
 * @brief Accepts either a bound Ranges instance, a raw list of (start, end) bounds, or None
 *
 * @param ranges The Python value to convert
 *
 * @return The equivalent RangeSpec, or std::nullopt if 'ranges' was None
 */
std::optional<PyIOrderedMultiMap::RangeSpec> parseRanges(const py::object &ranges);

/**
 * @brief Parses one keyRemap dict value into a KeyRemapList (manual dispatch, not pybind11's automatic std::variant support -- see PyOrderedMultiMap.cpp's top-of-file comment)
 *
 * @param value The Python list/tuple mixing bare keys and RemappedKeyData instances
 *
 * @return The equivalent KeyRemapList
 */
PyIOrderedMultiMap::KeyRemapList parseKeyRemapList(const py::object &value);

/**
 * @brief Converts a Python 'keyRemap' dict into remapKeys()'s vector-based parameter shape
 *
 * @param keyRemap The Python dict to convert
 *
 * @return The equivalent vector of (old key, remap rules) pairs, in dict iteration order
 */
std::vector<std::pair<py::object, PyIOrderedMultiMap::KeyRemapValue>> parseKeyRemap(const py::dict &keyRemap);

/**
 * @brief Converts a Python 'newVals' dict into replaceVals()'s vector-based parameter shape
 *
 * @param newVals The Python dict to convert
 *
 * @return The equivalent vector of (key, replace spec) pairs, in dict iteration order
 */
std::vector<std::pair<py::object, PyIOrderedMultiMap::ReplaceSpec>> parseReplaceVals(const py::dict &newVals);

/**
 * @brief Converts a Python 'items' dict into insertAllAt()'s vector-based parameter shape
 *
 * @param items The Python dict to convert
 *
 * @return The equivalent vector of (index, (key, value)) entries, in dict iteration order
 */
std::vector<std::pair<long long, std::pair<py::object, py::object>>> parseInsertAllAtItems(const py::dict &items);

/**
 * @brief Converts a Python 'orderMap' dict into reorder()'s vector-based parameter shape
 *
 * @param orderMap The Python dict to convert
 *
 * @return The equivalent vector of (old index, new index) pairs, in dict iteration order
 */
std::vector<std::pair<long long, long long>> parseOrderMap(const py::dict &orderMap);


/**
 * @brief
 @rst
 The ``.ini``-domain interface a :cpp:class:`AGRC::IfContentPart` is built over :raw-html:`<br />`
 :raw-html:`<br />`

 A ``.ini`` file's `KVPs`_ are ``std::string`` -> ``std::string``, so every ``.ini`` class is
 instantiated that way, while the general-purpose ``CppOrderedMultiMap`` stays ``py::object``-keyed.
 This alias names the interface of the former. It is deliberately **not** bound to `Python`_ -- see
 ``PyIfContentPart``'s dropped ``content`` property
 @endrst
 */
using PyIniIOrderedMultiMap = AGRC::IOrderedMultiMap<std::string, std::string>;


/**
 * @brief
 @rst
 The ``.ini``-domain counterparts of :cpp:func:`parseKeyRemap`/:cpp:func:`parseReplaceVals`/
 :cpp:func:`parseInsertAllAtItems` :raw-html:`<br />` :raw-html:`<br />`

 These three take the *same* `Python`_ values as their generic siblings -- including bound
 ``CppKeyRemapData``/``CppRemappedKeyData``/``ReplaceList``/``ReplaceIf`` instances, which stay
 ``py::object``-typed and singly-bound -- and build the ``std::string``-typed structures the
 ``.ini`` classes need. Converting here is what keeps those four value types from each needing a
 second bound copy

 :raw-html:`<br />`

 .. note::
    The remaining helpers need no counterpart. ``parseRanges``/``parseOrderMap`` are already
    key/value-agnostic (``RangeSpec`` is a ``std::vector<Ranges<long long>::Range>``);
    ``parseKeyRemapList`` is generic-only; and ``parseRemoveKeys`` is ``.ini``-only, so it was
    simply narrowed in place
 @endrst
 */
std::vector<std::pair<std::string, PyIniIOrderedMultiMap::KeyRemapValue>> parseIniKeyRemap(const py::dict &keyRemap);

/**
 * @copydoc parseIniKeyRemap
 */
std::vector<std::pair<std::string, PyIniIOrderedMultiMap::ReplaceSpec>> parseIniReplaceVals(const py::dict &newVals);

/**
 * @copydoc parseIniKeyRemap
 */
std::vector<std::pair<long long, std::pair<std::string, std::string>>> parseIniInsertAllAtItems(const py::dict &items);


void initCppIOrderedMultiMap(pybind11::module_ &m);

#endif
