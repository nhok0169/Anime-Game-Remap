#ifndef AGRemapPyBind_PyBaseIniGraphEdit_H
#define AGRemapPyBind_PyBaseIniGraphEdit_H

#include <pybind11/pybind11.h>

#include "../../../PyIniSectionGraph.h"  // reuses PyIniSectionGraph (the exact IniSectionGraph
                                          // instantiation every graph edit below edits) and, through
                                          // it, PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::BaseIniGraphEdit`\\<py::object, py::object\\>. Registered
 under the bare ``BaseIniGraphEdit`` name; the pure-Python original this replaced has been removed
 :raw-html:`<br />` :raw-html:`<br />`

 A plain alias, not a subclass -- nothing on the C++ side ever holds a graph edit through this base
 and calls ``edit`` itself (``GraphGroupEdit``, its one consumer, dispatches through `Python`_
 attribute lookup instead -- see `AGRC::GraphGroupEdit::PartEdit`), so no trampoline is needed: a
 `Python`_ subclass overriding ``edit`` is resolved by ordinary `Python`_ method lookup before
 anything reaches C++
 @endrst
 */
using PyBaseIniGraphEdit = AGRC::BaseIniGraphEdit<std::string, std::string>;


/**
 * @brief
 @rst
 Resolves the `Python`_ object a graph edit was handed for ``graph`` into the concrete
 `PyIniSectionGraph` every core ``edit`` takes :raw-html:`<br />` :raw-html:`<br />`

 A thin wrapper over ``py::cast``, kept as a named helper so every graph edit's binding raises the
 same message for a non-graph argument rather than `pybind11`_'s generic caster error
 @endrst
 *
 * @param graph The Python value to resolve
 *
 * @throw pybind11::type_error If 'graph' isn't an :class:`IniSectionGraph`
 *
 * @return The resolved graph -- borrowed, still owned by its Python wrapper
 */
PyIniSectionGraph &parseGraphArg(const py::object &graph);


/**
 * @brief Registers the Python-facing ``BaseIniGraphEdit``
 *
 * @param m The module to register into
 */
void initCppBaseIniGraphEdit(pybind11::module_ &m);

#endif
