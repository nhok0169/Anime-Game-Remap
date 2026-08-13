#ifndef AGRemapPyBind_PyIfContentPartColour_H
#define AGRemapPyBind_PyIfContentPartColour_H

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyIfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IfContentPartColourChange`\\<py::object\\>. A plain
 alias, not a subclass -- same reasoning as `PyIfContentPart` (see its own doc comment): there's
 no virtual method here to adapt, so nothing needs a C++-level wrapper.
 @endrst
 */
using PyIfContentPartColourChange = AGRC::IfContentPartColourChange<py::object>;

/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IfContentPartColouring`\\<py::object, py::object,
 PyObjectHash, PyObjectEqual, PyObjectHash, PyObjectEqual\\>. Both the key hashing/equality
 *and* the value hashing/equality (used by :cpp:func:`AGRC::IfContentPartColouring::getUniqueVals`)
 plug in `PyObjectHash`/`PyObjectEqual` (see `PyIfContentPart.h`), matching how `PyIfContentPart`
 itself plugs them in for `KeyHash`/`KeyEqual`.
 @endrst
 */
using PyIfContentPartColouring = AGRC::IfContentPartColouring<py::object, py::object, PyObjectHash, PyObjectEqual, PyObjectHash, PyObjectEqual>;


void initCppIfContentPartColour(pybind11::module_ &m);

#endif
