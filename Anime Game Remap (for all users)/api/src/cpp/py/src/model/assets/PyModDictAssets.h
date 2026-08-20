#ifndef AGRemapPyBind_PyModDictAssets_H
#define AGRemapPyBind_PyModDictAssets_H

#include <optional>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "../../tools/PyTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


// K and T are both bound as 'py::object' -- see PyOrderedMultiMap.h's comment (this file's
// established precedent) for why PyObjectHash/PyObjectEqual are plugged in as the KeyHash/
// KeyEqual template args rather than specializing std::hash<py::object> globally.
extern template class AGRC::ModDictAssets<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 The `pybind11`_ bound version of ``ModDictAssets``, using ``py::object`` for both the index
 value and leaf content type -- adds nothing over :cpp:class:`AGRC::ModDictAssets` itself, this
 exists purely so `pybind11`_ has a concrete (non-template) name to register, the same pattern
 as :class:`CppBiMap` :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Bound directly as the bare ``ModDictAssets`` -- a full-replacement port (this project's own
    "two outcomes for porting a class" convention, outcome 2), not a wrapper. The old pure-Python
    implementation lives on as ``ModDictAssetsOld`` (``model/assets/ModDictAssetsOld.py``), kept
    importable but no longer the bare name. ``Hashes``/``Indices`` (``model/assets/Hashes.py``/
    ``Indices.py``) subclass :cpp:class:`PyModMappedAssets` directly from pure Python (not this
    class -- they hold their actual asset data in a :cpp:class:`ModDictAssets` *repo*, one level
    down) -- no trampoline needed either way, since neither overrides any method this class's own
    C++ code calls back into
 @endrst
 */
class PyModDictAssets: public AGRC::ModDictAssets<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::ModDictAssets<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Base::Base;

        /**
         * @brief Converting constructor from the base type -- lets a reference to the plain
         *      base (e.g. what :cpp:class:`PyModMappedAssets`'s own base stores its repo as)
         *      be copied into a real, pybind-registered :cpp:class:`PyModDictAssets` (see
         *      ``PyModMappedAssets.cpp``'s ``repo`` property getter for where this is needed --
         *      a base-typed reference can't be returned/bound as this derived type directly)
         */
        explicit PyModDictAssets(const Base &base): Base(base) {}
};


/**
 * @brief
 @rst
 Converts a plain list of ``(indexVals, value)`` tuples (rows crossing the pybind boundary in
 this shape -- see :cpp:class:`PyModDictAssets`'s class-level note) into
 :cpp:class:`AGRC::Row`\ s :raw-html:`<br />` :raw-html:`<br />`

 Shared by :cpp:class:`PyModDictAssets`'s own constructor/``addRows`` and
 :cpp:class:`PyModMappedAssets`'s ``addRepoRows``/``addMap``
 @endrst
 */
std::vector<AGRC::Row<py::object, py::object>> convertRows(const std::vector<std::pair<std::vector<py::object>, py::object>> &rows);

/**
 * @brief
 @rst
 Flattens a nested dict (``{indexVal0: {indexVal1: {... : leafValue}}}``, 'totalIndices' levels
 deep -- the shape ``HashData``/``IndexData`` are literally written as) into
 :cpp:class:`AGRC::Row`\ s, the same flat shape :cpp:func:`convertRows` produces from an explicit
 row list :raw-html:`<br />` :raw-html:`<br />`

 This is the C++-side replacement for what the pure-Python ``ModDictAssets.makeIndices``/
 ``DictTools.nestedDictToDataFrame`` used to do -- kept in the pybind layer (not `core/`, which
 stays Python-free) and not in Python, so the flattening this project's asset data needs never
 has to live on the Python side at all
 @endrst
 *
 * @throws py::value_error If 'repo' is not nested exactly 'totalIndices' levels deep (a level
 *      that isn't a ``dict`` where one is expected, or a ``dict`` where a leaf value is expected)
 */
std::vector<AGRC::Row<py::object, py::object>> flattenNestedDict(const py::dict &repo, std::size_t totalIndices);

/**
 * @brief
 @rst
 Accepts rows in *either* shape a real caller uses -- a flat list of ``(indexVals, value)``
 tuples (see :cpp:func:`convertRows`), or a real nested dict (see :cpp:func:`flattenNestedDict`)
 -- and converts whichever was given into :cpp:class:`AGRC::Row`\ s :raw-html:`<br />` :raw-html:`<br />`

 Confirmed necessary, not just convenient: ``ModType.py``'s own real ``addMap``/``addRepoRows``
 call sites pass a nested dict (mirroring the pure-Python original's ``addMap(assetMap, assets=...)``
 shape), not a flat row list -- caught via a real test failure during development, not anticipated
 up front
 @endrst
 */
std::vector<AGRC::Row<py::object, py::object>> convertRowsOrNestedDict(const py::object &rowsOrNestedDict, std::size_t totalIndices);

/**
 * @brief
 @rst
 Normalizes a flexible non-version-values argument into a plain positional
 ``List[Optional[py::object]]``, with ``std::nullopt`` filling any position 'raw' doesn't specify
 a value for -- the C++-side port of the pure-Python ``BaseModAssets.toWildcardList``
 (``model/assets/BaseModAssets.py``), used by :cpp:class:`PyModMappedAssets`'s
 ``nonVersionIndexNames``-aware methods (``hasFrom``/``getKey``/``replace``/``replaceAll``/
 ``_convertNonVersionVals``) to accept the same flexible bare/list/dict-shaped argument the
 pure-Python original did, without the caller having to pre-convert :raw-html:`<br />` :raw-html:`<br />`

 'raw' being ``None`` *or* ``UnHashableNone`` (``FixRaidenBoss2.tools.DictTools.UnHashableNone``,
 the whole argument, not an element within it -- resolved lazily via a Python import, see this
 function's own implementation) means "no values given at all" (every position wildcarded) --
 still the documented default for real callers like ``GIMIParser.py``'s
 ``hashNonVersionVals``/``indexNonVersionVals`` and ``ModType.py``'s ``getHashRanges``. Otherwise:

 * A ``list`` is taken positionally, element-for-element, with ``std::nullopt`` for any position
   past its length (deliberately checks for an actual Python ``list`` only, matching the
   pure-Python original's own ``isinstance(indexVals, list)`` -- a ``tuple`` falls through to the
   "bare value" case below, exactly as it does there)
 * A ``dict`` is looked up by name, per position in 'indexNames', ``std::nullopt`` for any name
   not present
 * Anything else is treated as a single bare value for position 0, with every other position
   ``std::nullopt`` (matching the pure-Python original's own "does not iterate a bare Python
   string char-by-char" behavior, since a plain ``str``/``int``/etc. isn't a ``list`` or ``dict``)

 An explicit ``None`` found at a given position (whether padded in or given directly, in a list or
 dict) is indistinguishable from "no value given" here -- both mean wildcard, matching the
 pure-Python original exactly
 @endrst
 */
std::vector<std::optional<py::object>> toWildcardList(const py::object &raw, const std::vector<std::string> &indexNames);

void initCppModDictAssets(pybind11::module_ &m);

#endif
