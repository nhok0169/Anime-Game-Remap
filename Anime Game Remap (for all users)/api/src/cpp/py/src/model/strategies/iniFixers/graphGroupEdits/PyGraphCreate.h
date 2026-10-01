#ifndef PY_GRAPH_CREATE_H
#define PY_GRAPH_CREATE_H

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
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphCreate`\\<std::string, std::string\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Holds the exact `Python`_ objects given for ``graphId`` and ``graph``, as `PyGraphRemove` holds its
 ``graphIds``: ``someEdit.graph is theGraphYouPassed`` holds, and reassigning either afterwards
 changes what the edit does. The C++ members are re-derived from them at the start of every ``edit``
 (see #refresh) :raw-html:`<br />` :raw-html:`<br />`

 Holding the graph as a `Python`_ object is also what keeps it ALIVE for the duration of the edit:
 the core takes a borrowed ``Graph*`` and deep-copies it in, so the object it points at has to
 outlive the call
 @endrst
 */
class PyGraphCreate: public AGRC::GraphCreate<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphCreate<std::string, std::string>;

        /**
         * @brief The exact `Python`_ object given for ``graphId`` -- an ``(iniIndex, componentName, objectName)`` tuple
         */
        py::object graphIdObj;

        /**
         * @brief The exact `Python`_ object given for ``graph`` -- an :class:`IniSectionGraph`, or ``None``
         */
        py::object graphObj;

        /**
         * @brief Constructs a new graph-adding edit
         *
         * @param graphIdObj The Python ``(iniIndex, componentName, objectName)`` tuple
         * @param graphObj The Python :class:`IniSectionGraph` to add, or ``None``
         * @param minimal Whether the copy taken of the graph is a minimal one
         * @param newPartIds Whether the copy's parts get fresh ids
         */
        PyGraphCreate(py::object graphIdObj, py::object graphObj, bool minimal, bool newPartIds);

        /**
         * @brief Re-derives the core's \ref graphId and \ref graph from #graphIdObj and #graphObj
         */
        void refresh();
};


/**
 * @brief Registers the Python-facing ``GraphCreate``
 *
 * @param m The module to register into
 */
void initCppGraphCreate(pybind11::module_ &m);

#endif
