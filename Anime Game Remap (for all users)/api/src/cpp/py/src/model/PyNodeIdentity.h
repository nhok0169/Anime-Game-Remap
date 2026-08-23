#ifndef AGRemapPyBind_PyNodeIdentity_H
#define AGRemapPyBind_PyNodeIdentity_H

#include <cstdint>

#include <pybind11/pybind11.h>

#include "iftemplate/PyIfContentPart.h"


namespace py = pybind11;


/**
 * @brief
 @rst
 Computes the same integer `Python`_'s own builtin ``id(x)`` would report for the given
 :class:`IfContentPart` pointer's `pybind11`_ wrapper object -- shared by every binding
 (`CallGraph`, `IniSectionGraph`) that needs to hand `Python`_ callers a node identity matching
 what they'd get from calling ``id()`` on that same part themselves (see :cpp:class:`AGRC::CallGraph`'s
 own top-level note for why this has to be exact, not just "a" unique integer). ``py::cast`` with
 ``reference`` policy reuses `pybind11`_'s existing instance-registry wrapper for 'part' (creating
 one, registered under that same pointer, if none exists yet) -- the same wrapper object any other
 `Python`_-side reference to this exact C++ pointer already resolves to.
 @endrst
 *
 * @param part The part to compute the id of -- must not be ``nullptr``
 * @return The id, matching ``id(thatPartsPythonWrapper)``
 */
inline std::uintptr_t pyIdOfPart(PyIfContentPart *part) {
    py::object wrapper = py::cast(part, py::return_value_policy::reference);
    return reinterpret_cast<std::uintptr_t>(wrapper.ptr());
}

#endif
