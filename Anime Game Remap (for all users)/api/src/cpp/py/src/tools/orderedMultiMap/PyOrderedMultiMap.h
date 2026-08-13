#ifndef AGRemapPyBind_PyOrderedMultiMap_H
#define AGRemapPyBind_PyOrderedMultiMap_H

#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include "../PyTools.h"
#include "../PyRanges.h"
#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


// K and V are both bound as 'py::object' (see PyOrderedMultiMap.cpp's module-level
// comment for why), and BaseOrderedMultiMap's internal key -> bucket index
// (std::unordered_map<K, KeyBucket, KeyHash, KeyEqual>) needs a real Hash/KeyEqual
// for K -- PyObjectHash/PyObjectEqual (see ../PyTools.h) are this project's existing,
// established way of doing that for a 'py::object' key (see PyBiMap.h/PyDFA.h), so
// they're plugged in here via BaseOrderedMultiMap's KeyHash/KeyEqual template
// parameters rather than specializing std::hash<py::object>/std::equal_to<py::object>
// globally.
extern template class AGRC::OrderedMultiMap<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 The `pybind11`_ bound version of `RemappedKeyData`, using ``py::object`` for both the
 key and value type
 @endrst
 */
using PyRemappedKeyData = AGRC::RemappedKeyData<py::object, py::object>;

/**
 * @brief
 @rst
 The `pybind11`_ bound version of `KeyRemapData`, using ``py::object`` for both the key
 and value type
 @endrst
 */
using PyKeyRemapData = AGRC::KeyRemapData<py::object, py::object>;


/**
 * @brief
 @rst
 A Python-only marker (there's no equivalent C++ type) for a :meth:`PyOrderedMultiMap.replaceVals`
 spec: "update this key's entries positionally, from this list of values" :raw-html:`<br />` :raw-html:`<br />`

 A bare Python value can't be used for this directly, since ``V`` is itself ``py::object`` here --
 a bare list would then be genuinely ambiguous between "the single new value happens to be a
 list" and "here are several values, applied positionally", so this wrapper disambiguates the
 latter case explicitly.
 @endrst
 */
class PyReplaceList {
    public:
        explicit PyReplaceList(std::vector<py::object> values): values_(std::move(values)) {}

        const std::vector<py::object>& values() const { return values_; }

    private:
        std::vector<py::object> values_;
};


/**
 * @brief
 @rst
 A Python-only marker (there's no equivalent C++ type) for a :meth:`PyOrderedMultiMap.replaceVals`
 spec: "replace with this value, wherever this predicate returns True for the old value" :raw-html:`<br />` :raw-html:`<br />`

 Disambiguates against a bare replacement value for the same reason as :class:`PyReplaceList`.
 @endrst
 */
class PyReplaceIf {
    public:
        using Predicate = std::function<bool(const py::object&)>;

        explicit PyReplaceIf(py::object value, Predicate predicate):
            value_(std::move(value)), predicate_(std::move(predicate)) {}

        const py::object& value() const { return value_; }
        const Predicate& predicate() const { return predicate_; }

    private:
        py::object value_;
        Predicate predicate_;
};


/**
 * @brief
 @rst
 The `pybind11`_ bound version of `OrderedMultiMap`, using ``py::object`` for both the key
 and value type :raw-html:`<br />` :raw-html:`<br />`

 Unlike :cpp:class:`AGRC::BaseAhoCorasickDFA`/:cpp:class:`AGRC::BaseTrie` (which this class's
 own trampoline convention otherwise mirrors), :cpp:class:`AGRC::BaseOrderedMultiMap` is a
 `CRTP`_ base with no virtual methods at all -- there is nothing inherited to override.
 The methods declared ``virtual`` below are therefore new, purely Python-facing entry points
 introduced at this layer (not overrides of anything in the C++ core): specifically, the ones
 whose C++ signatures don't translate 1:1 into Python (they take a `tsl::ordered_map`_/`tsl::ordered_set`_,
 an ``std::variant``, or a container keyed by ``K`` that needs disambiguating marker types --
 see this file's ``.cpp`` for the reasoning behind each). Everything else (insert family,
 removeAt/removeKey, the simple getters, iteration) is bound directly to
 :cpp:class:`AGRC::OrderedMultiMap`'s own methods with no adapter needed, the same way
 :cpp:class:`PyRanges`'s non-virtual majority is bound directly to :cpp:class:`AGRC::Ranges`.
 @endrst
 */
class PyOrderedMultiMap: public AGRC::OrderedMultiMap<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::OrderedMultiMap<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Base::Base;

        /**
         * @brief Builds an instance from a key -> list of (index, value) mapping, preserving the dict's key order for cross-key tie-breaking
         *
         * @param indexed The key -> list of (index, value) pairs to build from
         *
         * @return The newly-built instance
         */
        static PyOrderedMultiMap fromIndexed(const tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> &indexed);

        /**
         * @copydoc AGRC::BaseOrderedMultiMap::insertAllAt(const tsl::ordered_map<long long, std::pair<K, V>>&, bool, const std::optional<Ranges<long long>>&)
         *
         @rst
         Python-facing adapter: ``items`` is a plain ``dict`` (``Dict[int, Tuple[Any, Any]]``),
         converted internally into the `tsl::ordered_map`_ overload so dict insertion order
         drives tie-breaking, matching this project's "one clean Python signature per operation"
         convention (see this file's ``.cpp``).
         @endrst
         */
        virtual size_t pyInsertAllAt(const py::dict &items, bool sortIndices, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc AGRC::BaseOrderedMultiMap::reorder(const tsl::ordered_map<long long, long long>&, const std::optional<Ranges<long long>>&)
         *
         @rst
         Python-facing adapter, same conversion as :meth:`pyInsertAllAt`.
         @endrst
         */
        virtual void pyReorder(const py::dict &orderMap, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc AGRC::BaseOrderedMultiMap::remapKeys
         *
         @rst
         Python-facing adapter: ``keyRemap`` is a plain ``dict`` whose values are either a
         :class:`CppKeyRemapData` instance, or a ``list`` mixing bare keys and
         :class:`CppRemappedKeyData` instances -- manually disambiguated by ``isinstance`` checks
         rather than relying on `pybind11`_'s automatic ``std::variant`` support, since a bare
         ``py::object`` alternative in a variant always matches first and would otherwise starve
         out the ``CppRemappedKeyData``/``CppKeyRemapData`` alternatives (see this file's ``.cpp``).
         @endrst
         */
        virtual void pyRemapKeys(const py::dict &keyRemap, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc AGRC::BaseOrderedMultiMap::replaceVals(const tsl::ordered_map<K, ReplaceSpec, KeyHash, KeyEqual>&, bool, const std::optional<Ranges<long long>>&)
         *
         @rst
         Python-facing adapter: ``newVals``'s values are either a bare replacement value, a
         :class:`ReplaceList`, or a :class:`ReplaceIf` -- see those classes' doc comments for
         why the wrapper markers are needed here specifically (unlike :meth:`pyRemapKeys`, where
         the existing :class:`CppRemappedKeyData`/:class:`CppKeyRemapData` classes already served as
         their own disambiguators).
         @endrst
         */
        virtual void pyReplaceVals(const py::dict &newVals, bool addNew, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc AGRC::BaseOrderedMultiMap::computeSplitGroups
         *
         @rst
         Python-facing adapter: ``inds`` is a plain ``list``, converted into the
         `tsl::ordered_set`_ overload so list order (post-dedup, first occurrence kept) is
         preserved for ``sortIndices=False`` callers.
         @endrst
         */
        virtual std::vector<PyOrderedMultiMap> pySplitByInds(const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices);

    private:
        // Lets fromIndexed() reach the protected BaseOrderedMultiMap::buildFromIndexed()
        // template from outside the class (buildFromIndexed() stays protected -- this is
        // the one narrow, deliberate crack in that, same idea as BaseOrderedMultiMap::iterKey()
        // existing solely to let Iterator reach a protected primitive).
        void buildFromIndexedDict(const tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> &indexed);

        // Rebuilds a PyOrderedMultiMap from a plain Base instance (e.g. one of splitByInds()'s
        // resulting parts) by copying its entries out -- Base has no knowledge of the derived
        // PyOrderedMultiMap type, so this can't happen on the Base side (same "Derived is
        // incomplete inside Base" reason OrderedMultiMap::groupsToParts() itself exists).
        static PyOrderedMultiMap fromBase(const Base &base);
};


/**
 * @brief
 @rst
 The `pybind11`_ trampoline class for `PyOrderedMultiMap`, allowing `PyOrderedMultiMap` to be
 subclassed from `Python`_ and its Python-facing adapter methods overridden
 @endrst
 */
class PyBindOrderedMultiMap: public PyOrderedMultiMap {
    public:
        using PyOrderedMultiMap::PyOrderedMultiMap;

        size_t pyInsertAllAt(const py::dict &items, bool sortIndices, const std::optional<PyRanges<long long>> &ranges) override;
        void pyReorder(const py::dict &orderMap, const std::optional<PyRanges<long long>> &ranges) override;
        void pyRemapKeys(const py::dict &keyRemap, const std::optional<PyRanges<long long>> &ranges) override;
        void pyReplaceVals(const py::dict &newVals, bool addNew, const std::optional<PyRanges<long long>> &ranges) override;
        std::vector<PyOrderedMultiMap> pySplitByInds(const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices) override;
};


/**
 * @brief
 @rst
 A small `pybind11`_-only forward iterator wrapper around
 :cpp:class:`AGRC::BaseOrderedMultiMap::Iterator`, since the latter dereferences to an ``Item``
 of references (``const K&``/``const V&``) that :func:`py::make_iterator` isn't set up to copy
 out cleanly on its own (see the core library's own notes on this). Bound with ``__iter__``/
 ``__next__`` instead, copying each entry's key/value/occurrenceIndex/orderIndex out into a
 plain Python tuple per step.
 @endrst
 */
class PyOrderedMultiMapIterator {
    public:
        using BaseIterator = PyOrderedMultiMap::Base::Iterator;

        PyOrderedMultiMapIterator(BaseIterator current, BaseIterator end): current_(current), end_(end) {}

        PyOrderedMultiMapIterator& iter();
        std::tuple<py::object, py::object, size_t, size_t> next();

    private:
        BaseIterator current_;
        BaseIterator end_;
};


void initCppOrderedMultiMap(pybind11::module_ &m);

#endif
