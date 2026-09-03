#ifndef AGRemapPyBind_PyCallGraph_H
#define AGRemapPyBind_PyCallGraph_H

#include <pybind11/pybind11.h>

#include "iftemplate/PyIfContentPart.h"  // reuses PyIfContentPart/PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/CallGraph.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::CallGraph`\\<py::object, py::object\\>. A plain alias, not
 a subclass.

 :raw-html:`<br />`

 .. note::
    See `AGRC::CallGraph`'s own top-level note: this binding is where a #AGRC::CallGraph::Node
    (a real ``(part pointer, isExit)`` pair) turns into whatever shape `Python`_ callers expect --
    an integer equal to `Python`_'s own builtin ``id(part)`` for a plain node, or an
    ``("exit", thatSameInteger)`` tuple for an exit node. This is load-bearing, not cosmetic:
    real callers (``RegSurroundedAdd.py``) correlate a part they already hold via
    ``id(part)`` directly against this graph's ``forwardEdges``/``partsById`` keys, so the integer
    produced here must be bit-identical to what `Python`_'s own ``id()`` computes for that same
    part's existing wrapper object -- achieved via ``reinterpret_cast<std::uintptr_t>`` on the
    `pybind11`_-registered wrapper `pybind11`_'s own instance registry hands back for a given C++
    pointer (the same wrapper `Python`_ code holding that part would see, since `pybind11`_
    guarantees one wrapper per live C++ pointer under the default holder).
 @endrst
 */
using PyCallGraph = AGRC::CallGraph<std::string, std::string>;


void initCppCallGraph(pybind11::module_ &m);

#endif
