#ifndef PY_GRAPH_FILTER_H
#define PY_GRAPH_FILTER_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphFilter.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 Wraps a `Python`_ callable ``(section, part, index) -> bool`` as a
 :cpp:type:`AGRemapCore::GraphFilter::Predicate`. ``None`` gives an empty predicate, which keeps
 everything. The section and part are passed by reference, so the callable sees the live ``IfTemplate`` / ``IfContentPart``
 @endrst
 */
AGRC::GraphFilter<std::string, std::string>::Predicate toGraphPredicate(const py::object &predicate);

/**
 * @brief The callable a predicate made by :cpp:func:`toGraphPredicate` was made from, the C++ predicate cast to Python
 *        for any other, or ``None`` for an empty one
 */
py::object graphPredicateToPy(const AGRC::GraphFilter<std::string, std::string>::Predicate &predicate);


/**
 * @brief
 @rst
 The `Python`_ ``GraphFilter`` is :cpp:class:`AGRemapCore::GraphFilter` itself: its arguments are parsed into the C++ members
 when it is built or they are set, and read back from them -- so one built in `Python`_ runs in a C++ fixer's list, and
 one built in C++ reads in `Python`_ as a ``GraphFilter``
 @endrst
 */
using PyGraphFilter = AGRC::GraphFilter<std::string, std::string>;


void initCppGraphFilter(pybind11::module_ &m);

#endif
