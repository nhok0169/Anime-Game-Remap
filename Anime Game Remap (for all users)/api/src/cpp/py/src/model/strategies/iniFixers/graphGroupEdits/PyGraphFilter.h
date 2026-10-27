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
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphFilter`\\<std::string, std::string\\>. It holds the
 exact `Python`_ objects passed for ``graphIds`` and ``predicate``, and re-derives the core members
 from them at the start of every ``edit`` (see #refresh), as `PyGraphRemove` does
 @endrst
 */
class PyGraphFilter: public AGRC::GraphFilter<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphFilter<std::string, std::string>;

        /**
         * @brief The exact `Python`_ list of graph id tuples
         */
        py::object graphIdsObj;

        /**
         * @brief The exact `Python`_ predicate, or ``None``
         */
        py::object predicateObj;

        /**
         * @brief Constructs a new filtering edit
         *
         * @param graphIdsObj The Python list of graph id tuples to filter
         * @param predicateObj The Python predicate, or None
         */
        PyGraphFilter(py::object graphIdsObj, py::object predicateObj);

        /**
         * @brief Re-derives the core members from #graphIdsObj and #predicateObj
         */
        void refresh();
};


void initCppGraphFilter(pybind11::module_ &m);

#endif
