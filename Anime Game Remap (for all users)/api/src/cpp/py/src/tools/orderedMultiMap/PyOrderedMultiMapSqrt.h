#ifndef AGRemapPyBind_PyOrderedMultiMapSqrt_H
#define AGRemapPyBind_PyOrderedMultiMapSqrt_H

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
#include "PyOrderedMultiMap.h"  // reuses PyRemappedKeyData/PyKeyRemapData/PyReplaceList/PyReplaceIf --
                                // these are all keyed purely on py::object K/V, not on which
                                // backing structure remapKeys()/replaceVals() is called on, so
                                // OrderedMultiMap and OrderedMultiMapSqrt share one Python-facing
                                // RemappedKeyData/KeyRemapData/ReplaceList/ReplaceIf rather than
                                // each getting their own redundant copy
#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMapSqrt.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


extern template class AGRC::OrderedMultiMapSqrt<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 The `pybind11`_ bound version of `OrderedMultiMapSqrt`, using ``py::object`` for both the key
 and value type -- see :cpp:class:`PyOrderedMultiMap`'s doc comment for the overall binding
 convention this follows (only :meth:`insertAllAt`, :meth:`reorder`, :meth:`remapKeys`,
 :meth:`replaceVals`, and :meth:`splitByInds` need adapter logic; everything else binds
 directly). The two classes intentionally do **not** share a common Python-facing base: their
 shared C++ base, :cpp:class:`AGRC::BaseOrderedMultiMap`, is a CRTP base with no
 `pybind11`_-visible surface of its own (see `PyOrderedMultiMap.h`'s doc comment), so there's
 nothing to factor out on the Python side either -- only the small marker/helper types
 (`CppRemappedKeyData`, `CppKeyRemapData`, `ReplaceList`, `ReplaceIf`) are actually shared,
 and those are reused directly (see the include of `PyOrderedMultiMap.h` above) rather than
 duplicated.
 @endrst
 */
class PyOrderedMultiMapSqrt: public AGRC::OrderedMultiMapSqrt<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::OrderedMultiMapSqrt<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Base::Base;

        /**
         * @copydoc PyOrderedMultiMap::fromIndexed
         */
        static PyOrderedMultiMapSqrt fromIndexed(const tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> &indexed);

        /**
         * @copydoc PyOrderedMultiMap::pyInsertAllAt
         */
        virtual size_t pyInsertAllAt(const py::dict &items, bool sortIndices, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc PyOrderedMultiMap::pyReorder
         */
        virtual void pyReorder(const py::dict &orderMap, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc PyOrderedMultiMap::pyRemapKeys
         */
        virtual void pyRemapKeys(const py::dict &keyRemap, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc PyOrderedMultiMap::pyReplaceVals
         */
        virtual void pyReplaceVals(const py::dict &newVals, bool addNew, const std::optional<PyRanges<long long>> &ranges);

        /**
         * @copydoc PyOrderedMultiMap::pySplitByInds
         */
        virtual std::vector<PyOrderedMultiMapSqrt> pySplitByInds(const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices);

    private:
        // See PyOrderedMultiMap::buildFromIndexedDict()'s comment.
        void buildFromIndexedDict(const tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> &indexed);

        // See PyOrderedMultiMap::fromBase()'s comment.
        static PyOrderedMultiMapSqrt fromBase(const Base &base);
};


/**
 * @brief
 @rst
 The `pybind11`_ trampoline class for `PyOrderedMultiMapSqrt`, allowing `PyOrderedMultiMapSqrt`
 to be subclassed from `Python`_ and its Python-facing adapter methods overridden
 @endrst
 */
class PyBindOrderedMultiMapSqrt: public PyOrderedMultiMapSqrt {
    public:
        using PyOrderedMultiMapSqrt::PyOrderedMultiMapSqrt;

        size_t pyInsertAllAt(const py::dict &items, bool sortIndices, const std::optional<PyRanges<long long>> &ranges) override;
        void pyReorder(const py::dict &orderMap, const std::optional<PyRanges<long long>> &ranges) override;
        void pyRemapKeys(const py::dict &keyRemap, const std::optional<PyRanges<long long>> &ranges) override;
        void pyReplaceVals(const py::dict &newVals, bool addNew, const std::optional<PyRanges<long long>> &ranges) override;
        std::vector<PyOrderedMultiMapSqrt> pySplitByInds(const std::vector<long long> &inds, bool includeSplitKVP, bool includeEmptyParts, bool sortIndices) override;
};


/**
 * @copydoc PyOrderedMultiMapIterator
 */
class PyOrderedMultiMapSqrtIterator {
    public:
        using BaseIterator = PyOrderedMultiMapSqrt::Base::Iterator;

        PyOrderedMultiMapSqrtIterator(BaseIterator current, BaseIterator end): current_(current), end_(end) {}

        PyOrderedMultiMapSqrtIterator& iter();
        std::tuple<py::object, py::object, size_t, size_t> next();

    private:
        BaseIterator current_;
        BaseIterator end_;
};


void initCppOrderedMultiMapSqrt(pybind11::module_ &m);

#endif
