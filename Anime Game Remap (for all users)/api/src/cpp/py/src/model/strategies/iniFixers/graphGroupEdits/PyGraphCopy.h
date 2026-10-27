#ifndef PY_GRAPH_COPY_H
#define PY_GRAPH_COPY_H

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
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCopy.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphCopy`\\<std::string, std::string\\>. It holds the
 exact `Python`_ objects passed for the two ids and the predicate, and re-derives the core members
 from them at the start of every ``edit`` (see #refresh)
 @endrst
 */
class PyGraphCopy: public AGRC::GraphCopy<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphCopy<std::string, std::string>;

        /**
         * @brief The exact `Python`_ id of the graph to copy
         */
        py::object srcIdObj;

        /**
         * @brief The exact `Python`_ id the copy is stored under
         */
        py::object dstIdObj;

        /**
         * @brief The exact `Python`_ predicate, or ``None``
         */
        py::object predicateObj;

        /**
         * @brief Constructs a new copying edit
         *
         * @param srcIdObj The id of the graph to copy
         * @param dstIdObj The id the copy is stored under
         * @param predicateObj The Python predicate, or None
         * @param minimal Whether the copy is a minimal one
         * @param newPartIds Whether the copy's parts get fresh ids
         */
        PyGraphCopy(py::object srcIdObj, py::object dstIdObj, py::object predicateObj, bool minimal, bool newPartIds);

        /**
         * @brief Re-derives the core members from #srcIdObj, #dstIdObj and #predicateObj
         */
        void refresh();
};


void initCppGraphCopy(pybind11::module_ &m);

#endif
