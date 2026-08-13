#ifndef AGRemapPyBind_PyIfContentPart_H
#define AGRemapPyBind_PyIfContentPart_H

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "../../tools/orderedMultiMap/PyIOrderedMultiMap.h"  // reuses PyIOrderedMultiMap/PyBindIOrderedMultiMap
                                                                // and the parseRanges/parseKeyRemap/
                                                                // parseReplaceVals/parseInsertAllAtItems/
                                                                // parseOrderMap dict-parsing helpers
#include "AGRemapCore/model/iftemplate/IfContentPart.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IfContentPart`\\<py::object, py::object\\>. A plain
 alias, not a subclass -- :cpp:class:`AGRC::IfContentPart` has no virtual methods of its own
 (every operation is a thin, renamed delegation to its held :cpp:class:`AGRC::IOrderedMultiMap`,
 which is where the actual polymorphism/`Python`_-subclassing story already lives -- see
 `PyIOrderedMultiMap.h`), so there's nothing to adapt at the C++ level. The ``dict``-based
 `Python`_ conveniences below (``addKVPsByInds``/``remapKeys``/``replaceVals``/``removeKeys``/
 ``ranges``) reuse the exact parsing helpers `PyIOrderedMultiMap`'s own binding declares, so a
 `CppOrderedMultiMap`/`CppOrderedMultiMapSqrt`-style ``dict`` works identically here. :raw-html:`<br />` :raw-html:`<br />`

 .. warning::
    The constructor takes ``content`` by ownership (matching the C++ side's
    ``std::unique_ptr<IOrderedMultiMap<K, V>>``) -- once a bound :class:`IOrderedMultiMap`
    instance (e.g. the result of ``someMap.asInterface()``) is passed into
    :class:`IfContentPart`'s constructor, that original Python object no longer owns any
    underlying data and should not be used again. This mirrors real move-only ownership
    transfer, not a Python-specific quirk.
 @endrst
 */
using PyIfContentPart = AGRC::IfContentPart<py::object, py::object, PyObjectHash, PyObjectEqual>;


void initCppIfContentPart(pybind11::module_ &m);

#endif
